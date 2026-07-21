#include "BaseEnemy.h"

#include <Game/Collision/CollisionLayerUtil.h>
#include <Game/World/EnemyState.h>
#include <Game/World/KillPlane.h>
#include <Game/Player/Sword/Sword.h>

#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Foundation/Input/Input.h>


BaseEnemy::BaseEnemy(EnemyAnimationSet animations, const std::string& objectName, EnemyStats& stats)
	: Actor(animations.idle, objectName), stats_(stats), animations_(std::move(animations)) {
	SerializableParamObjectsMutable().push_back(&lockOnTarget_);
}


/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void BaseEnemy::Initialize() {
	Actor::Initialize();

	worldTransform_.scale = { 0.5f,0.5f,0.5f };
	currentHp_ = stats_.maxHp;
	lockOnTarget_.Initialize(*this);
	PlayAnimation(EnemyAnimationID::Idle);
	hit_.Load("EnemyHit");
	hitLight_.Load("hitLight");

	if (EnemyState::Get().IsDefeated(GetGuid())) {
		pendingRemove_ = true;
		lockOnTarget_.SetOwnerAlive(false);
		SetDrawEnable(false);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//			更新
/////////////////////////////////////////////////////////////////////////////////////////
void BaseEnemy::Update(float dt) {
	if (pendingRemove_) {
		lockOnTarget_.Unregister();
		if (auto* ctx = SceneContext::Current())
			ctx->RemoveObject(std::static_pointer_cast<SceneObject>(shared_from_this()));
		return;
	}

	// 落下死。ステージ外に落ちたら倒された扱いにして、末尾の IsDead() で消す
	if (!IsDead() && KillPlane::IsFallenOut(GetWorldPosition())) {
		currentHp_ = 0;
	}

	const CalyxEngine::Vector3 frameStartPosition = GetWorldPosition();
	damageAnimationTimer_ = damageAnimationTimer_ > dt ? damageAnimationTimer_ - dt : 0.0f;
	auto targetPlayer = stats_.target.Resolve().get();
	const float stopSq = stats_.knockbackStopSpeed * stats_.knockbackStopSpeed;
	if (knockbackVelocity_.LengthSquared() > stopSq) {
		// 吹き飛び中は追尾せず、ノックバックで動かす
		UpdateKnockback(dt);
	} else if (movement_ && targetPlayer && AllowMovement()) {
		movement_->Update(*this, targetPlayer->GetWorldPosition(), dt);
	}

	// 攻撃（ノックバックで吹き飛んでいる間は攻撃しない）
	if (knockbackVelocity_.LengthSquared() <= stopSq && attack_ && targetPlayer && AllowAttack()) {
		attack_->Update(*this, targetPlayer, dt);
	}

	if (CalyxFoundation::Input::TriggerKey(DIK_P)|| CalyxFoundation::Input::TriggerGamepadButton(CalyxFoundation::PadButton::X)) {
		CalyxEngine::Vector3 dir = CalyxEngine::Quaternion::RotateVector(
			CalyxEngine::Vector3::Forward(), targetPlayer->GetRenderWorldTransform().rotation);
		dir.y = 0.0f;
		if (dir.LengthSquared() <= 0.0001f) {
			return;
		}
		EffectAPI::Play(hit_, worldTransform_.GetWorldPosition());
		EffectAPI::Play(hitLight_, worldTransform_.GetWorldPosition());

		knockbackVelocity_ = dir.Normalize() * stats_.knockbackInitialSpeed;
	}

	Actor::Update(dt);

	if (damageAnimationTimer_ <= 0.0f && !(attack_ && attack_->IsAttacking())) {
		const CalyxEngine::Vector3 moved = GetWorldPosition() - frameStartPosition;
		PlayAnimation(moved.LengthSquared() > 1.0e-6f ? EnemyAnimationID::Move : EnemyAnimationID::Idle);
	}

	if (IsDead()) {
		EnemyState::Get().MarkDefeated(GetGuid());     // 倒したら記録
		lockOnTarget_.SetOwnerAlive(false);
		lockOnTarget_.Unregister();
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
		EffectAPI::Play(hit_, worldTransform_.GetWorldPosition());
		EffectAPI::Play(hitLight_, worldTransform_.GetWorldPosition());

	}


	//// プレイヤーの攻撃に当たったときだけ
	//if ((other->GetType() & ColliderType::Type_PlayerAttack) != ColliderType::Type_None) {
	//	OnHitByPlayerAttack(other);
	//	//EffectAPI::Play(hit_, worldTransform_.GetWorldPosition());
	//}
}

void BaseEnemy::DerivativeGui() {
	GuiCmd::SceneObjectReferenceField("PlayerTrans", stats_.target);
	stats_.ShowGui();
	lockOnTarget_.ShowGui();
	if (attack_) {
		attack_->ShowGui();
	}
}

void BaseEnemy::Destroy() {
	lockOnTarget_.Unregister();
	Actor::Destroy();
}

void BaseEnemy::OnHitByPlayerAttack(Collider* attacker) {
	ApplyKnockbackFrom(attacker);

	auto* hitbox = attacker ? dynamic_cast<Sword*>(attacker->GetOwner()) : nullptr;
	const int   damage = hitbox ? hitbox->GetDamage() : 1;
	TakeDamage(damage);
}

void BaseEnemy::ApplyKnockbackFrom(Collider* attacker) {
	auto targetPlayer = stats_.target.Resolve().get();
	if (!attacker || !targetPlayer) {
		return;
	}

	// プレイヤーが向いている方向へ吹き飛ばす
	CalyxEngine::Vector3 dir = CalyxEngine::Quaternion::RotateVector(
		CalyxEngine::Vector3::Forward(), targetPlayer->GetRenderWorldTransform().rotation);
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
}

void BaseEnemy::TakeDamage(int amount) {
	if (amount <= 0) {
		return;
	}
	currentHp_ -= amount;
	if (currentHp_ < 0) {
		currentHp_ = 0;
	}
	if (!animations_.damage.empty()) {
		damageAnimationTimer_ = 0.25f;
		PlayAnimation(EnemyAnimationID::Damage);
	}
	// 死亡時の処理は後で IsDead() を見て呼び出し側で行う
}

void BaseEnemy::PlayAnimation(EnemyAnimationID animationId) {
	const std::string* requested = &animations_.idle;
	switch (animationId) {
	case EnemyAnimationID::Move:    requested = &animations_.move; break;
	case EnemyAnimationID::Attack:  requested = &animations_.attack; break;
	case EnemyAnimationID::Attack2: requested = &animations_.attack2; break;
	case EnemyAnimationID::Damage:  requested = &animations_.damage; break;
	case EnemyAnimationID::Defence: requested = &animations_.defence; break;
	case EnemyAnimationID::Aim:     requested = &animations_.aim; break;
	default: break;
	}

	const std::string& modelName = requested->empty() ? animations_.idle : *requested;
	if (modelName.empty() || currentAnimationModel_ == modelName) {
		return;
	}
	currentAnimationModel_ = modelName;
	SetModelFileNameForEditor(modelName);
}

void BaseEnemy::PlayNextAttackAnimation() {
	if (animations_.attack2.empty()) {
		PlayAnimation(EnemyAnimationID::Attack);
		return;
	}
	PlayAnimation(useSecondAttackAnimation_ ? EnemyAnimationID::Attack2 : EnemyAnimationID::Attack);
	useSecondAttackAnimation_ = !useSecondAttackAnimation_;
}

