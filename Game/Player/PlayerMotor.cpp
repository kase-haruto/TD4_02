#include "PlayerMotor.h"

#include "Base/PlayerBase.h"

#include <Game/Battle/LockOn/ILockOnStateReader.h>

#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <Engine/Physics/Character/CharacterMovementComponent.h>
#include <Engine/Foundation/Math/MathUtil.h>

#include <cmath>


void PlayerMotor::Initialize(PlayerBase* player) {
	param_.LoadParams();
	player->GetCharacterMovement().SetMaxWalkSpeed(param_.moveSpeed);
	player->GetCharacterMovement().SetJumpVelocity(param_.jumpForce);
}

namespace {
	float Clamp01(float value) {
		if (value < 0.0f) return 0.0f;
		if (value > 1.0f) return 1.0f;
		return value;
	}
}

void PlayerMotor::Update(PlayerBase* player, const PlayerInputState& input, float dt, bool ignoreLockOnFacing) {
	player->GetCharacterMovement().SetMaxWalkSpeed(input.dashHeld ? param_.dashSpeed : param_.moveSpeed);

	// --- 移動 ---
	CalyxEngine::Vector3 worldDirection = BuildWorldMoveDirection(input.move);
	if (worldDirection.LengthSquared() > 0.01f) {
		lastMoveDir_ = worldDirection;

		CalyxEngine::Vector3 forward = CalyxEngine::Quaternion::RotateVector(
			CalyxEngine::Vector3::Forward(), player->GetWorldTransform().rotation);
		forward.y = 0.0f;
		forward = forward.LengthSquared() > 0.0001f ? forward.Normalize() : CalyxEngine::Vector3::Forward();
		const CalyxEngine::Vector3 right{ forward.z, 0.0f, -forward.x };
		const CalyxEngine::Vector3 move = worldDirection.Normalize();
		const float forwardAmount = CalyxEngine::Vector3::Dot(move, forward);
		const float rightAmount = CalyxEngine::Vector3::Dot(move, right);
		if (std::abs(forwardAmount) >= std::abs(rightAmount)) {
			player->RequestAnimation(forwardAmount >= 0.0f ? PlayerAnimationID::MoveFront : PlayerAnimationID::MoveBack);
		} else {
			player->RequestAnimation(rightAmount >= 0.0f ? PlayerAnimationID::MoveRight : PlayerAnimationID::MoveLeft);
		}
		if (player->AppliesMovement()) {
			player->GetCharacterMovement().AddMovementInput(worldDirection);
		}
	} else {
		player->RequestAnimation(PlayerAnimationID::Idle);
	}

	// --- 向き ---
	// ignoreLockOnFacing のときはロックオン中でも狙いを手動で決めさせる
	if (!ignoreLockOnFacing && lockOnState_ && lockOnState_->IsLockingOn()) {
		FaceLockOnTarget(player, dt);
	} else if (input.aimWithMouse) {
		// マウスカーソルの方を向く
		// 向きの基準位置：クローンは親Playerの位置を使う（未設定なら自分）
		// カーソルからカメラのレイを作り、プレイヤーの高さの水平面と交差させて狙点を求める
		const PlayerBase* originObj = aimOrigin_ ? aimOrigin_ : player;
		CalyxEngine::Vector3 originPos = originObj->GetWorldPosition();

		// 同じスクリーン座標を 2つの深度でワールドに戻し、視線レイの向きを作る
		// （near/far の2点差分にすることで depthZ の規約が [0,1] でも何でも向きは正しく出る）
		CalyxEngine::Vector3 nearP = CalyxEngine::ScreenToWorld(input.aimScreen, 0.0f);
		CalyxEngine::Vector3 farP = CalyxEngine::ScreenToWorld(input.aimScreen, 1.0f);
		CalyxEngine::Vector3 rayDir = farP - nearP;

		// レイ (nearP + t*rayDir) と 水平面 y = originPos.y の交点
		constexpr float kEps = 1e-6f;
		if (rayDir.y < -kEps || rayDir.y > kEps) {
			float t = (originPos.y - nearP.y) / rayDir.y;
			if (t > 0.0f) {  // カメラより奥（前方）で交わるときだけ
				CalyxEngine::Vector3 hit = nearP + rayDir * t;
				CalyxEngine::Vector3 toCursor = { hit.x - originPos.x, 0.0f, hit.z - originPos.z };

				constexpr float kAimDeadZoneSq = 0.01f; // ≈0.1m 以内なら向きを変えない
				if (toCursor.LengthSquared() > kAimDeadZoneSq) {
					FaceMoveDirection(player, toCursor);
				}
			}
		}
	} else {
		// パッド操作時の向き。
		// 右スティックを倒している間はそちらを優先し、中立の間は移動方向を向く。
		constexpr float kAimThresholdSq = 0.04f;
		if (input.look.LengthSquared() > kAimThresholdSq) {
			FaceMoveDirection(player, BuildWorldMoveDirection(input.look));
		} else if (worldDirection.LengthSquared() > 0.01f) {
			// 移動入力がある時だけ向きを更新する。
			// 停止中に向きが初期方向へ戻らないよう、入力ゼロでは何もしない。
			FaceMoveDirection(player, worldDirection);
		}
	}

	// --- ジャンプ ---
	/*if (input.jumpPressed) {
		player->GetCharacterMovement().Jump();
	}*/
}

void PlayerMotor::ShowGui(){
	param_.ShowGui();
}

CalyxEngine::Vector3 PlayerMotor::BuildWorldMoveDirection(const CalyxEngine::Vector2& move) const {
	CalyxEngine::Vector3 right = CalyxEngine::Vector3::Right();
	CalyxEngine::Vector3 forward = CalyxEngine::Vector3::Forward();

	if (auto* camera = CameraManager::GetMain3d()) {
		const auto& m = camera->GetWorldTransform().matrix.world;

		right = { m.m[0][0], 0.0f, m.m[0][2] };
		forward = { m.m[2][0], 0.0f, m.m[2][2] };

		right = (right.LengthSquared() > 0.0001f) ? right.Normalize() : CalyxEngine::Vector3::Right();
		forward = (forward.LengthSquared() > 0.0001f) ? forward.Normalize() : CalyxEngine::Vector3::Forward();
	}

	CalyxEngine::Vector3 direction = right * move.x + forward * move.y;

	// 斜め移動が速くならないよう、正規化
	if (direction.LengthSquared() > 1.0f) {
		direction = direction.Normalize();
	}
	return direction;
}

void PlayerMotor::FaceMoveDirection(PlayerBase* player, const CalyxEngine::Vector3& worldDirection) const {
	if (worldDirection.LengthSquared() <= 0.0f) {
		return;
	}

	CalyxEngine::Vector3 to = worldDirection.Normalize();

	// モデルの基準前方(Forward)を、移動方向(to)へ回す回転を作る
	player->GetWorldTransform().rotation =
		CalyxEngine::Quaternion::FromToQuaternion(CalyxEngine::Vector3::Forward(), to);
}

void PlayerMotor::FaceLockOnTarget(PlayerBase* player, float dt) const {
	if (!lockOnState_) {
		return;
	}

	const WorldTransform* target = lockOnState_->ResolveCurrentTarget();
	if (!target) {
		return;
	}

	CalyxEngine::Vector3 toTarget = target->GetWorldPosition() - player->GetWorldPosition();
	toTarget.y = 0.0f;
	if (toTarget.LengthSquared() <= 0.0001f) {
		return;
	}

	const CalyxEngine::Quaternion targetRotation =
		CalyxEngine::Quaternion::FromToQuaternion(CalyxEngine::Vector3::Forward(), toTarget.Normalize());
	const float t = Clamp01(lockOnState_->GetPlayerTurnSpeed() * dt);
	player->GetWorldTransform().rotation =
		CalyxEngine::Quaternion::Slerp(player->GetWorldTransform().rotation, targetRotation, t);
}
