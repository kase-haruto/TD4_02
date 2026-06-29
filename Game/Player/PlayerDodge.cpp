#include "PlayerDodge.h"

#include "Base/PlayerBase.h"

#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Physics/Character/CharacterMovementComponent.h>

void PlayerDodge::Update(PlayerBase* player, const PlayerInputState& input, float dt) {
	// タイマー減衰
	if (invincibleTimer_ > 0.0f) invincibleTimer_ -= dt;
	if (cooldownTimer_ > 0.0f) cooldownTimer_ -= dt;

	// --- 回避中 ---
	if (isDodging_) {
		dodgeTimer_ += dt;

		// 開始時に決めた向きへ進み続ける
		player->GetCharacterMovement().AddMovementInput(dodgeDir_);

		if (dodgeTimer_ >= kDodgeDuration) {
			isDodging_ = false;
			cooldownTimer_ = kCooldown;
		}
		return;
	}

	// --- 回避開始判定 ---
	if (input.dodgePressed && cooldownTimer_ <= 0.0f) {
		StartDodge(player);
	}
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
	invincibleTimer_ = kInvincibleTime;
	player->PlayAnimation(PlayerAnimationID::Dodge);
}
