#include "HomingMove.h"

#include <Enemy/BaseEnemy.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Physics/Character/CharacterMovementComponent.h>

void HomingMove::Update(BaseEnemy& self, const CalyxEngine::Vector3& targetPos, float /*dt*/) {
	const EnemyStats& stats = self.GetStats();

	CalyxEngine::Vector3 toTarget = targetPos - self.GetWorldPosition();
	toTarget.y = 0.0f; // とりあえず高さ無視

	const float distanceSq = toTarget.LengthSquared();
	// 探索範囲外なら何もしない
	if (distanceSq > stats.detectionRange * stats.detectionRange) {
		return;
	}
	// 攻撃範囲まで近づいたら何もしない
	if (distanceSq <= stats.attackRange * stats.attackRange) {
		return;
	}

	CalyxEngine::Vector3 dir = toTarget.Normalize();
	self.GetCharacterMovement().AddMovementInput(dir);

	self.GetWorldTransform().rotation =
		CalyxEngine::Quaternion::FromToQuaternion(CalyxEngine::Vector3::Forward(), dir);
}
