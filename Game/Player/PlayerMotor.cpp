#include "PlayerMotor.h"

#include "Base/PlayerBase.h"

#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <Engine/Physics/Character/CharacterMovementComponent.h>
#include <Engine/Foundation/Math/MathUtil.h>


void PlayerMotor::Initialize(PlayerBase* player) {
	param_.LoadParams();
	player->GetCharacterMovement().SetMaxWalkSpeed(param_.moveSpeed);
	player->GetCharacterMovement().SetJumpVelocity(param_.jumpForce);
}

void PlayerMotor::Update(PlayerBase* player, const PlayerInputState& input, float /*dt*/) {
	// --- 移動 ---
	CalyxEngine::Vector3 worldDirection = BuildWorldMoveDirection(input.move);
	if (worldDirection.LengthSquared() > 0.0f) {
		lastMoveDir_ = worldDirection;
		if (player->GetCurrentAnimationId() != PlayerAnimationID::Walk) {
			player->PlayAnimation(PlayerAnimationID::Walk);
		}
		player->GetCharacterMovement().AddMovementInput(worldDirection);
	} else {
		if (player->GetCurrentAnimationId() != PlayerAnimationID::Idle) {
			player->PlayAnimation(PlayerAnimationID::Idle);
		}
	}

	// --- 向き ---
	if (input.aimWithMouse) {
		// マウスカーソルの方を向く
		// 向きの基準位置：クローンは親Playerの位置を使う（未設定なら自分）
		const PlayerBase* originObj = aimOrigin_ ? aimOrigin_ : player;
		CalyxEngine::Vector2 originScreen = CalyxEngine::WorldToScreen(originObj->GetWorldPosition());
		CalyxEngine::Vector2 delta = input.aimScreen - originScreen;

		constexpr float kCursorDeadZoneSq = 4.0f;
		if (delta.LengthSquared() > kCursorDeadZoneSq) {
			CalyxEngine::Vector2 aim2D = { delta.x, -delta.y };
			FaceMoveDirection(player, BuildWorldMoveDirection(aim2D));
		}
	} else {
		// パッド右スティックの方を向く（中立ならFaceMoveDirection側で何もしない）
		constexpr float kAimThresholdSq = 0.04f;
		if (input.look.LengthSquared() > kAimThresholdSq) {
			FaceMoveDirection(player, BuildWorldMoveDirection(input.look));
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
