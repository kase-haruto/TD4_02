#include "PlayerDodge.h"

#include "Base/PlayerBase.h"

#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Physics/Character/CharacterMovementComponent.h>

void PlayerDodge::Initialize([[maybe_unused]] PlayerBase* player) {
	param_.LoadParams();
}

void PlayerDodge::Update(PlayerBase* player, const PlayerInputState& input, float dt) {
	// タイマー減衰
	if (invincibleTimer_ > 0.0f) invincibleTimer_ -= dt;
	if (cooldownTimer_ > 0.0f) cooldownTimer_ -= dt;

	// --- 回避中 ---
	if (isDodging_) {
		dodgeTimer_ += dt;

		// 開始時に決めた向きへ進み続ける
		if (player->AppliesMovement()) {
			player->GetCharacterMovement().AddMovementInput(dodgeDir_);
		}

		if (dodgeTimer_ >= param_.dodgeDuration) {
			isDodging_ = false;
			cooldownTimer_ = param_.cooldown;
		}
		return;
	}

	// --- 回避開始判定 ---
	if (enabled_ && input.dodgePressed && cooldownTimer_ <= 0.0f) {
		StartDodge(player);
	}
}

void PlayerDodge::ShowGui() {
	param_.ShowGui();
}

void PlayerDodge::Reset() {
	isDodging_ = false;
	enabled_ = true;
	dodgeTimer_ = 0.0f;
	invincibleTimer_ = 0.0f;
	cooldownTimer_ = 0.0f;
	dodgeDir_ = {};
}

void PlayerDodge::StartDodge(PlayerBase* player) {
	CalyxEngine::Vector3 forward =
		player->GetMoveDir();
	forward.y = 0.0f;

	if (forward.LengthSquared() <= 1.0e-6f) {
		forward = CalyxEngine::Vector3::Forward();
	}
	dodgeDir_ = forward.Normalize();

	isDodging_ = true;
	dodgeTimer_ = 0.0f;
	invincibleTimer_ = param_.invincibleTime;
	player->PlayAnimation(PlayerAnimationID::Dodge);
	player->GetCharacterMovement().SetMaxWalkSpeed(param_.dodgeSpeed);

	dodgeDir_ = forward.Normalize();
	// 回避方向を向く（マウスの狙い向きより優先）
	player->GetWorldTransform().rotation =
		CalyxEngine::Quaternion::FromToQuaternion(CalyxEngine::Vector3::Forward(), dodgeDir_);
	isDodging_ = true;
}
