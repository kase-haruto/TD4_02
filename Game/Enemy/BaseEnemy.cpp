#include "BaseEnemy.h"

#include <Game/Collision/CollisionLayerUtil.h>

#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Foundation/Input/Input.h>


BaseEnemy::BaseEnemy(const std::string& modelName, const std::string& objectName, EnemyStats& stats)
	: Actor(modelName, objectName), stats_(stats) {}


/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void BaseEnemy::Initialize() {
	Actor::Initialize();

	currentHp_ = stats_.maxHp;
	//hit_.Load("EnemyHit");
}

/////////////////////////////////////////////////////////////////////////////////////////
//			更新
/////////////////////////////////////////////////////////////////////////////////////////
void BaseEnemy::Update(float dt) {
	const float stopSq = stats_.knockbackStopSpeed * stats_.knockbackStopSpeed;
	if (knockbackVelocity_.LengthSquared() > stopSq) {
		// 吹き飛び中は追尾せず、ノックバックで動かす
		UpdateKnockback(dt);
	} else if (movement_ && target_ && AllowMovement()) {
		movement_->Update(*this, target_->GetWorldPosition(), dt);
	}

	// 攻撃（ノックバックで吹き飛んでいる間は攻撃しない）
	if (knockbackVelocity_.LengthSquared() <= stopSq && attack_ && target_ && AllowAttack()) {
		attack_->Update(*this, target_, dt);
	}

	if (CalyxFoundation::Input::TriggerKey(DIK_P)|| CalyxFoundation::Input::TriggerGamepadButton(CalyxFoundation::PadButton::X)) {
		CalyxEngine::Vector3 dir = CalyxEngine::Quaternion::RotateVector(
			CalyxEngine::Vector3::Forward(), target_->GetWorldTransform().rotation);
		dir.y = 0.0f;
		if (dir.LengthSquared() <= 0.0001f) {
			return;
		}
		//EffectAPI::Play(hit_, worldTransform_.GetWorldPosition());

		knockbackVelocity_ = dir.Normalize() * stats_.knockbackInitialSpeed;
	}

	Actor::Update(dt);

	if (IsDead()) {
		if (auto* context = SceneContext::Current()) {
			context->RemoveObject(
				std::static_pointer_cast<SceneObject>(shared_from_this()));
		}
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//			衝突
/////////////////////////////////////////////////////////////////////////////////////////
void BaseEnemy::OnCollisionEnter(Collider* other) {
	if (!other) {
		return;
	}

	const auto playerAttackLayer = GameCollision::FindLayerId("PlayerAttack");
	if (playerAttackLayer && other->GetLayerId() == *playerAttackLayer) {
		OnHitByPlayerAttack(other);
		//EffectAPI::Play(hit_, worldTransform_.GetWorldPosition());
	}


	//// プレイヤーの攻撃に当たったときだけ
	//if ((other->GetType() & ColliderType::Type_PlayerAttack) != ColliderType::Type_None) {
	//	OnHitByPlayerAttack(other);
	//	//EffectAPI::Play(hit_, worldTransform_.GetWorldPosition());
	//}
}

void BaseEnemy::DerivativeGui() {
	stats_.ShowGui();
	if (attack_) {
		attack_->ShowGui();
	}
}

void BaseEnemy::OnHitByPlayerAttack(Collider* attacker) {
	ApplyKnockbackFrom(attacker);
	TakeDamage(1);
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

	knockbackVelocity_ = dir.Normalize() * stats_.knockbackInitialSpeed;
}

void BaseEnemy::UpdateKnockback(float dt) {
	GetWorldTransform().translation =
		GetWorldTransform().translation + knockbackVelocity_ * dt;

	// 減衰
	knockbackVelocity_ = knockbackVelocity_ * (1.0f - stats_.knockbackDamping * dt);

	const float stopSq = stats_.knockbackStopSpeed * stats_.knockbackStopSpeed;
	if (knockbackVelocity_.LengthSquared() <= stopSq) {
		knockbackVelocity_ = {};
	}
}

void BaseEnemy::SetMovement(std::unique_ptr<IEnemyMovement> movement) {
	movement_ = std::move(movement);
}

void BaseEnemy::SetAttack(std::unique_ptr<IEnemyAttack> attack) {
	attack_ = std::move(attack);
	if (attack_) {
		SerializableParamObjectsMutable().push_back(&attack_->SerializableParam());
	}
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

