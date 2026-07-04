#include "RangedAttack.h"

#include <Enemy/BaseEnemy.h>
#include <Enemy/Projectile/Arrow.h>

#include <Engine/Objects/3D/Actor/Actor.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Scene/Utility/SceneUtility.h>
#include <optional>
#include <string>

RangedAttack::RangedAttack() {
	param_.LoadParams();
}

void RangedAttack::Update(BaseEnemy& self, const Actor* target, float dt) {
	if (cooldownTimer_ > 0.0f) {
		cooldownTimer_ -= dt;
	}
	if (!target || cooldownTimer_ > 0.0f) {
		return;
	}

	const float range = self.GetStats().attackRange;
	if (PlanarDistanceSq(self, target) <= range * range) {
		Fire(self, target);
		cooldownTimer_ = self.GetStats().attackInterval;
	}
}

void RangedAttack::ShowGui() {
	param_.ShowGui();
}

CalyxEngine::SerializableObject& RangedAttack::SerializableParam() {
	return param_;
}

void RangedAttack::Fire(BaseEnemy& self, const Actor* target) {
	// プレイヤーへの水平方向
	CalyxEngine::Vector3 dir = target->GetWorldPosition() - self.GetWorldPosition();
	dir.y = 0.0f;
	if (dir.LengthSquared() <= 1.0e-6f) {
		return;
	}
	dir = dir.Normalize();

	// 発射向きに敵を向ける
	const CalyxEngine::Quaternion aim =
		CalyxEngine::Quaternion::FromToQuaternion(CalyxEngine::Vector3::Forward(), dir);
	self.GetWorldTransform().rotation = aim;

	// 発射位置
	const CalyxEngine::Vector3 spawnPos =
		self.GetWorldTransform().translation +
		dir * param_.spawnForwardOffset +
		CalyxEngine::Vector3(0.0f, param_.spawnHeight, 0.0f);

	auto arrow = SceneAPI::Instantiate<Arrow>(
		std::string("debugCube.obj"),
		std::optional<std::string>("EnemyArrow"));
	if (!arrow) {
		return;
	}

	arrow->ConfigureAsAttackHitbox(param_.arrowSize, param_.drawHitbox, Sword::HitboxOwner::Enemy);
	arrow->SetDamage(self.GetStats().attackDamage);
	arrow->SetDrawEnable(true);
	arrow->SetScale(param_.arrowSize);   // サイズ

	arrow->SetTranslate(spawnPos);
	arrow->SetRotate(aim);

	arrow->Launch(dir * param_.arrowSpeed, param_.arrowLifeTime);
}

float RangedAttack::PlanarDistanceSq(const BaseEnemy& self, const Actor* target) {
	CalyxEngine::Vector3 toTarget = target->GetWorldPosition() - self.GetWorldPosition();
	toTarget.y = 0.0f;
	return toTarget.LengthSquared();
}