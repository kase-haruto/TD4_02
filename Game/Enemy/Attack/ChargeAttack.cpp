#include "ChargeAttack.h"

#include <Enemy/BaseEnemy.h>
#include <Player/Sword/Sword.h>

#include <Engine/Objects/3D/Actor/Actor.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Scene/Context/SceneContext.h>
#include <Engine/Scene/Utility/SceneUtility.h>
#include <optional>
#include <string>

ChargeAttack::ChargeAttack() {
	param_.LoadParams();
}

void ChargeAttack::Update(BaseEnemy& self, const Actor* target, float dt) {
	// 攻撃中は状態ごとの処理へ
	switch (phase_) {
	case Phase::Windup:  UpdateWindup(self, dt);  return;
	case Phase::Dash:    UpdateDash(self, dt);    return;
	case Phase::Recover: UpdateRecover(self, dt); return;
	default: break;
	}

	// クールダウンと射程判定
	if (cooldownTimer_ > 0.0f) {
		cooldownTimer_ -= dt;
	}
	if (!target || cooldownTimer_ > 0.0f) {
		return;
	}

	const float range = self.GetStats().attackRange;
	if (PlanarDistanceSq(self, target) <= range * range) {
		StartCharge(self, target);
	}
}

void ChargeAttack::ShowGui() {
	param_.ShowGui();
}

CalyxEngine::SerializableObject& ChargeAttack::SerializableParam() {
	return param_;
}

void ChargeAttack::StartCharge(BaseEnemy& self, const Actor* target) {
	// 突進方向をこの瞬間に固定する
	CalyxEngine::Vector3 toTarget = target->GetWorldPosition() - self.GetWorldPosition();
	toTarget.y = 0.0f;
	if (toTarget.LengthSquared() <= 1.0e-6f) {
		return;
	}
	chargeDir_ = toTarget.Normalize();

	// overshootDistanceぶん通り過ぎた点を目標にする
	CalyxEngine::Vector3 playerPos = target->GetWorldPosition();
	playerPos.y = self.GetWorldPosition().y;
	chargeTargetPos_ = playerPos + chargeDir_ * param_.overshootDistance;

	// 突進方向を向く
	self.GetWorldTransform().rotation =
		CalyxEngine::Quaternion::FromToQuaternion(CalyxEngine::Vector3::Forward(), chargeDir_);

	phase_ = Phase::Windup;
	phaseTimer_ = 0.0f;
	self.PlayAnimation(EnemyAnimationID::Aim);
}

void ChargeAttack::UpdateWindup(BaseEnemy& self, float dt) {
	phaseTimer_ += dt;
	if (phaseTimer_ >= param_.windupTime) {
		phase_ = Phase::Dash;
		phaseTimer_ = 0.0f;
		self.PlayAnimation(EnemyAnimationID::Attack);
		CreateHitbox(self);   // 突進の瞬間に当たり判定を出す
	}
}

void ChargeAttack::UpdateDash(BaseEnemy& self, float dt) {
	phaseTimer_ += dt;

	// 突進
	self.GetWorldTransform().translation =
		self.GetWorldTransform().translation + chargeDir_ * param_.chargeSpeed * dt;

	UpdateHitbox(self);

	// 目標地点を通り過ぎた or 時間切れで終了
	CalyxEngine::Vector3 toGoal = chargeTargetPos_ - self.GetWorldPosition();
	toGoal.y = 0.0f;
	const bool passed = CalyxEngine::Vector3::Dot(toGoal, chargeDir_) <= 0.0f;

	if (passed || phaseTimer_ >= param_.maxChargeTime) {
		RemoveHitbox();
		phase_ = Phase::Recover;
		phaseTimer_ = 0.0f;
	}
}

void ChargeAttack::UpdateRecover(BaseEnemy& self, float dt) {
	phaseTimer_ += dt;
	if (phaseTimer_ >= param_.recoverTime) {
		EndCharge(self);
	}
}

void ChargeAttack::EndCharge(BaseEnemy& self) {
	phase_ = Phase::Idle;
	phaseTimer_ = 0.0f;
	self.PlayAnimation(EnemyAnimationID::Idle);
	RemoveHitbox();
	cooldownTimer_ = self.GetStats().attackInterval;   // 次の突進まで間隔をあける
}

void ChargeAttack::CreateHitbox(BaseEnemy& self) {
	if (hitbox_) {
		return;
	}
	hitbox_ = SceneAPI::Instantiate<Sword>(
		std::string("PlayerSword.obj"),
		std::optional<std::string>("BatChargeHitbox"));
	if (hitbox_) {
		hitbox_->ConfigureAsAttackHitbox(param_.hitboxSize, param_.drawHitbox, Sword::HitboxOwner::Enemy);
		hitbox_->SetDamage(self.GetStats().attackDamage);
		UpdateHitbox(self);
	}
}

void ChargeAttack::UpdateHitbox(BaseEnemy& self) {
	if (!hitbox_) {
		return;
	}

	const CalyxEngine::Vector3 position =
		self.GetWorldTransform().translation +
		CalyxEngine::Vector3(0.0f, param_.hitboxHeightOffset, 0.0f);

	hitbox_->SetTranslate(position);
	hitbox_->SetRotate(self.GetWorldTransform().rotation);
}

void ChargeAttack::RemoveHitbox() {
	if (!hitbox_) {
		return;
	}
	if (auto* context = SceneContext::Current()) {
		context->RemoveObject(std::static_pointer_cast<SceneObject>(hitbox_));
	}
	hitbox_.reset();
}

float ChargeAttack::PlanarDistanceSq(const BaseEnemy& self, const Actor* target) {
	CalyxEngine::Vector3 toTarget = target->GetWorldPosition() - self.GetWorldPosition();
	toTarget.y = 0.0f;
	return toTarget.LengthSquared();
}
