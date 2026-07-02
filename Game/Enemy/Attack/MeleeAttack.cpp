#include "MeleeAttack.h"

#include <Enemy/BaseEnemy.h>
#include <Player/Sword/Sword.h>

#include <Engine/Objects/3D/Actor/Actor.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Scene/Context/SceneContext.h>
#include <Engine/Scene/Utility/SceneUtility.h>
#include <optional>
#include <string>

MeleeAttack::MeleeAttack() {
	param_.LoadParams();
}

void MeleeAttack::Update(BaseEnemy& self, const Actor* target, float dt) {
	if (isAttacking_) {
		UpdateAttack(self, dt);
		return;
	}

	if (cooldownTimer_ > 0.0f) {
		cooldownTimer_ -= dt;
	}

	if (!target || cooldownTimer_ > 0.0f) {
		return;
	}

	// 攻撃範囲に入っていたら攻撃開始
	const float range = self.GetStats().attackRange;
	if (PlanarDistanceSq(self, target) <= range * range) {
		StartAttack(self, target);
	}
}

void MeleeAttack::ShowGui() {
	param_.ShowGui();
}

CalyxEngine::SerializableObject& MeleeAttack::SerializableParam() {
	return param_;
}

void MeleeAttack::StartAttack(BaseEnemy& self, const Actor* target) {
	isAttacking_ = true;
	attackTimer_ = 0.0f;

	CalyxEngine::Vector3 toTarget = target->GetWorldPosition() - self.GetWorldPosition();
	toTarget.y = 0.0f;
	if (toTarget.LengthSquared() > 1.0e-6f) {
		self.GetWorldTransform().rotation =
			CalyxEngine::Quaternion::FromToQuaternion(
				CalyxEngine::Vector3::Forward(), toTarget.Normalize());
	}
}

void MeleeAttack::UpdateAttack(BaseEnemy& self, float dt) {
	attackTimer_ += dt;

	// 判定を出す時間だけ Hitbox を生成・追従
	if (IsHitboxActive()) {
		CreateAttackHitbox(self);
		UpdateAttackHitbox(self);
	} else {
		RemoveAttackHitbox();
	}

	if (attackTimer_ >= param_.attackDuration) {
		EndAttack(self);
	}
}

void MeleeAttack::EndAttack(BaseEnemy& self) {
	isAttacking_ = false;
	attackTimer_ = 0.0f;
	RemoveAttackHitbox();

	// 次の攻撃まで間隔をあける
	cooldownTimer_ = self.GetStats().attackInterval;
}

bool MeleeAttack::IsHitboxActive() const {
	return isAttacking_ &&
		attackTimer_ >= param_.hitboxActiveStart &&
		attackTimer_ <= param_.hitboxActiveEnd;
}

void MeleeAttack::CreateAttackHitbox(BaseEnemy& self) {
	if (attackHitbox_) {
		return;
	}

	attackHitbox_ = SceneAPI::Instantiate<Sword>(
		std::string("PlayerSword.obj"),
		std::optional<std::string>("EnemyAttackHitbox"));
	if (attackHitbox_) {
		attackHitbox_->ConfigureAsAttackHitbox(param_.hitboxSize, param_.drawHitbox, Sword::HitboxOwner::Enemy);
		UpdateAttackHitbox(self);
	}
}

void MeleeAttack::UpdateAttackHitbox(BaseEnemy& self) {
	if (!attackHitbox_) {
		return;
	}

	CalyxEngine::Vector3 forward =
		CalyxEngine::Quaternion::RotateVector(
			CalyxEngine::Vector3::Forward(),
			self.GetWorldTransform().rotation);
	forward.y = 0.0f;
	forward = forward.LengthSquared() > 1.0e-6f ? forward.Normalize() : CalyxEngine::Vector3::Forward();

	const CalyxEngine::Vector3 position =
		self.GetWorldTransform().translation +
		forward * param_.hitboxForwardOffset +
		CalyxEngine::Vector3(0.0f, param_.hitboxHeightOffset, 0.0f);

	attackHitbox_->SetTranslate(position);
	attackHitbox_->SetRotate(self.GetWorldTransform().rotation);
}

void MeleeAttack::RemoveAttackHitbox() {
	if (!attackHitbox_) {
		return;
	}

	if (auto* context = SceneContext::Current()) {
		context->RemoveObject(std::static_pointer_cast<SceneObject>(attackHitbox_));
	}
	attackHitbox_.reset();
}

float MeleeAttack::PlanarDistanceSq(const BaseEnemy& self, const Actor* target) {
	CalyxEngine::Vector3 toTarget = target->GetWorldPosition() - self.GetWorldPosition();
	toTarget.y = 0.0f;
	return toTarget.LengthSquared();
}
