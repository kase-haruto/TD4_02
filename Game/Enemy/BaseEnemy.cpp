#include "BaseEnemy.h"

#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Foundation/Input/Input.h>


namespace {
	constexpr float kKnockbackInitialSpeed = 16.0f;  // 吹き飛び初速 (m/s)
	constexpr float kKnockbackDamping      = 8.0f;  // 減衰の強さ。大きいほど早く止まる
	constexpr float kKnockbackStopSq       = 0.04f; // 速度がこれ以下(≈0.2m/s)で停止
}

BaseEnemy::BaseEnemy(const std::string& modelName, const std::string& objectName)
	: Actor(modelName, objectName) {}


/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void BaseEnemy::Initialize() {
	Actor::Initialize();

	currentHp_ = stats_.maxHp;
	hit_.Load("EnemyHit");
}

/////////////////////////////////////////////////////////////////////////////////////////
//			更新
/////////////////////////////////////////////////////////////////////////////////////////
void BaseEnemy::Update(float dt) {
	if (knockbackVelocity_.LengthSquared() > kKnockbackStopSq) {
		// 吹き飛び中は追尾せず、ノックバックで動かす
		UpdateKnockback(dt);
	} else if (movement_ && target_) {
		movement_->Update(*this, target_->GetWorldPosition(), dt);
	}

	if (CalyxFoundation::Input::TriggerKey(DIK_P)) {
		CalyxEngine::Vector3 dir = CalyxEngine::Quaternion::RotateVector(
			CalyxEngine::Vector3::Forward(), target_->GetWorldTransform().rotation);
		dir.y = 0.0f;
		if (dir.LengthSquared() <= 0.0001f) {
			return;
		}
		EffectAPI::Play(hit_, worldTransform_.GetWorldPosition());

		knockbackVelocity_ = dir.Normalize() * kKnockbackInitialSpeed;
	}

	Actor::Update(dt);
}

/////////////////////////////////////////////////////////////////////////////////////////
//			衝突
/////////////////////////////////////////////////////////////////////////////////////////
void BaseEnemy::OnCollisionEnter(Collider* other) {
	if (!other) {
		return;
	}
	// プレイヤーの攻撃に当たったときだけ
	if ((other->GetType() & ColliderType::Type_PlayerAttack) != ColliderType::Type_None) {
		OnHitByPlayerAttack(other);
		EffectAPI::Play(hit_, worldTransform_.GetWorldPosition());
	}
}

void BaseEnemy::OnHitByPlayerAttack(Collider* attacker) {
	ApplyKnockbackFrom(attacker);
}

void BaseEnemy::ApplyKnockbackFrom(Collider* attacker) {
	if (!attacker || !target_) {
		return;
	}

	// プレイヤーが向いている方向へ吹き飛ばす
	CalyxEngine::Vector3 dir = CalyxEngine::Quaternion::RotateVector(
		CalyxEngine::Vector3::Forward(), target_->GetWorldTransform().rotation);
	dir.y = 0.0f;
	if (dir.LengthSquared() <= 0.0001f) {
		return;
	}

	knockbackVelocity_ = dir.Normalize() * kKnockbackInitialSpeed;
}

void BaseEnemy::UpdateKnockback(float dt) {
	GetWorldTransform().translation =
		GetWorldTransform().translation + knockbackVelocity_ * dt;

	// 減衰
	knockbackVelocity_ = knockbackVelocity_ * (1.0f - kKnockbackDamping * dt);

	if (knockbackVelocity_.LengthSquared() <= kKnockbackStopSq) {
		knockbackVelocity_ = {};
	}
}

void BaseEnemy::SetMovement(std::unique_ptr<IEnemyMovement> movement) {
	movement_ = std::move(movement);
}

void BaseEnemy::TakeDamage(int amount) {
	if (amount <= 0) {
		return;
	}
	currentHp_ -= amount;
	if (currentHp_ < 0) {
		currentHp_ = 0;
	}
	// 死亡時の処理は後で IsDead() を見て呼び出し側で行う
}

