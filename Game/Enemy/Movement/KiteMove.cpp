#include "KiteMove.h"

#include <Enemy/BaseEnemy.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Physics/Character/CharacterMovementComponent.h>

namespace {
	// 近すぎしきい値より内側なら後退
	constexpr float kKeepDistanceRatio = 0.6f;
	// 回り込む向きを切り替える間隔
	constexpr float kStrafeSwitchTime = 1.2f;
}

void KiteMove::Update(BaseEnemy& self, const CalyxEngine::Vector3& targetPos, float dt) {
	const EnemyStats& stats = self.GetStats();

	CalyxEngine::Vector3 toTarget = targetPos - self.GetWorldPosition();
	toTarget.y = 0.0f;

	const float distanceSq = toTarget.LengthSquared();

	// 索敵範囲外なら何もしない
	if (distanceSq > stats.detectionRange * stats.detectionRange) { return; }
	if (distanceSq <= 1.0e-6f) { return; }

	CalyxEngine::Vector3 dir = toTarget.Normalize();

	// 常にプレイヤーの方を向く
	self.GetWorldTransform().rotation =
		CalyxEngine::Quaternion::FromToQuaternion(CalyxEngine::Vector3::Forward(), dir);

	const float attackRange = stats.attackRange;
	const float keepDistance = attackRange * kKeepDistanceRatio;

	CalyxEngine::Vector3 input{};

	if (distanceSq > attackRange * attackRange) {
		// 遠い → 近づく
		input = dir;
	} else if (distanceSq < keepDistance * keepDistance) {
		// 近すぎ → 後退
		input = dir * -1.0f;
	} else {
		// 左右へ回り込む
		strafeTimer_ += dt;
		if (strafeTimer_ >= kStrafeSwitchTime) {
			strafeTimer_ = 0.0f;
			strafeSide_ = -strafeSide_;
		}
		CalyxEngine::Vector3 perp{ -dir.z, 0.0f, dir.x };
		input = perp * static_cast<float>(strafeSide_);
	}

	self.GetCharacterMovement().AddMovementInput(input);
}