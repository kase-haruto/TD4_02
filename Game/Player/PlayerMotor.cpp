#include "PlayerMotor.h"

#include "Player.h"

#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <Engine/Physics/Character/CharacterMovementComponent.h>

void PlayerMotor::Update(Player& player, const PlayerInputState& input, float /*dt*/) {
	CalyxEngine::Vector3 worldDirection = BuildWorldMoveDirection(input.move);

	if (worldDirection.LengthSquared() > 0.0f) {
		player.GetCharacterMovement().AddMovementInput(worldDirection);

		FaceMoveDirection(player, worldDirection);
	}

	if (input.jumpPressed) {
		player.GetCharacterMovement().Jump();
	}
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

void PlayerMotor::FaceMoveDirection(Player& player, const CalyxEngine::Vector3& worldDirection) const {
	if (worldDirection.LengthSquared() <= 0.0f) {
		return;
	}

	CalyxEngine::Vector3 to = worldDirection.Normalize();

	// モデルの基準前方(Forward)を、移動方向(to)へ回す回転を作る
	player.GetWorldTransform().rotation =
		CalyxEngine::Quaternion::FromToQuaternion(CalyxEngine::Vector3::Forward(), to);
}