#include "BaseEnemy.h"

#include <Game/Collision/CollisionLayerUtil.h>
#include <Game/World/EnemyState.h>
#include <Game/World/KillPlane.h>
#include <Game/Player/Sword/Sword.h>

#include <Engine/Objects/Collider/Collider.h>
#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Foundation/Input/Input.h>
#include <Engine/Foundation/Utility/Random/Random.h>

namespace {
	constexpr float kDeathShakeTime = 0.75f; // 震えている時間(秒)
	constexpr float kDeathShakeAmplitude = 0.1f; // 震え幅(m)
	constexpr float kDeathBurstTime = 0.4f;  // エフェクト時間
}

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
	death_.Load("EnemyExp");

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

	if (damageRimTime_ > 0.0f) {
		ApplyDamageRim();
		damageRimTime_ = damageRimTime_ > dt ? damageRimTime_ - dt : 0.0f;
		if (damageRimTime_ <= 0.0f) {
			ClearRimLight();
		}
	}

	// 死亡演出中は移動・攻撃・アニメを止めて、演出だけを進める
	if (deathPhase_ != EnemyDeathPhase::None) {
		if (UpdateDeathSequence(dt)) {
			if (auto* context = SceneContext::Current()) {
				context->RemoveObject(std::static_pointer_cast<SceneObject>(shared_from_this()));
			}
		}
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

	Actor::Update(dt);

	const CalyxEngine::Vector3 moved = GetWorldPosition() - frameStartPosition;
	const bool isMoving = moved.LengthSquared() > 1.0e-6f;

	if (damageAnimationTimer_ <= 0.0f && !(attack_ && attack_->IsAttacking())) {
		PlayAnimation(isMoving ? EnemyAnimationID::Move : EnemyAnimationID::Idle);
	}

	UpdateDustEffect(isMoving);

	if (IsDead()) {
		BeginDeathSequence();
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

	// 死亡演出中は被弾を受け付けない
	if (deathPhase_ != EnemyDeathPhase::None) {
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
	// 攻撃中に消される場合、攻撃判定だけがシーンに残るのを防ぐ
	if (attack_) {
		attack_->Cancel();
	}
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
	if (!attacker) {
		return;
	}

	// 攻撃してきたヒットボックスは振った本人(プレイヤー or クローン)の向きを持っている。
	// そちらを優先し、取れないときだけプレイヤーの向きを使う。
	auto fallbackTarget = stats_.target.Resolve();   // 参照を保持したまま使う
	const BaseGameObject* source = attacker->GetOwner();
	if (!source) {
		source = fallbackTarget.get();
	}
	if (!source) {
		return;
	}

	// 攻撃してきた側が向いている方向へ吹き飛ばす
	CalyxEngine::Vector3 dir = CalyxEngine::Quaternion::RotateVector(
		CalyxEngine::Vector3::Forward(), source->GetRenderWorldTransform().rotation);
	dir.y = 0.0f;
	if (dir.LengthSquared() <= 0.0001f) {
		return;
	}

	knockbackVelocity_ = dir.Normalize() * stats_.knockbackInitialSpeed;
}

void BaseEnemy::ApplyDamageRim() {
	SetRimLight({ 1.0f, 0.1f, 0.1f, 1.0f }, 12.0f, 1.0f);
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

void BaseEnemy::UpdateDustEffect(bool isMoving) {
	if (walk_.GetData().emitters.empty()) {
		return;
	}

	if (isMoving) {
		if (!isDust_) {
			dustHandle_ = EffectAPI::Play(walk_, GetWorldPosition());
			isDust_ = true;
		} else {
			// 再生中はエミッターを敵に追従させる
			EffectAPI::Player()->SetTransform(
				dustHandle_,
				GetWorldPosition(),
				CalyxEngine::Quaternion::MakeIdentity(),
				{ 1.0f, 1.0f, 1.0f });
		}
	} else if (isDust_) {
		EffectAPI::Stop(dustHandle_);
		dustHandle_ = {};
		isDust_ = false;
	}
}

void BaseEnemy::BeginDeathSequence() {
	if (deathPhase_ != EnemyDeathPhase::None) {
		return;
	}

	// 撃破記録とロックオン解除は演出の開始時点で
	EnemyState::Get().MarkDefeated(GetGuid());
	lockOnTarget_.SetOwnerAlive(false);
	lockOnTarget_.Unregister();

	// 土煙止める
	EffectAPI::Stop(dustHandle_);
	dustHandle_ = {};
	isDust_ = false;

	// 震えている間に動かないよう、ノックバックは打ち切る
	knockbackVelocity_ = {};

	deathPhase_ = EnemyDeathPhase::Shake;
	deathTimer_ = 0.0f;
	deathBasePosition_ = GetWorldTransform().translation;

	// 攻撃を中断し
	if (attack_) {
		attack_->Cancel();
	}

	// 自分の当たり判定を切る。
	/*if (auto* collider = GetCollider()) {
		
	}*/
}

bool BaseEnemy::UpdateDeathSequence(float dt) {
	deathTimer_ += dt;

	if (deathPhase_ == EnemyDeathPhase::Shake) {
		// 終わりに近づくほど大きく震わせて、弾ける直前の溜めを作る
		const float shakeRate = deathTimer_ / kDeathShakeTime;
		CalyxEngine::Vector3 jitter =
			Random::GenerateVector3(-kDeathShakeAmplitude, kDeathShakeAmplitude) * (0.3f + 0.7f * shakeRate);
		jitter.y *= 0.3f;
		GetWorldTransform().translation = deathBasePosition_ + jitter;

		if (deathTimer_ >= kDeathShakeTime) {
			// 震え終了
			GetWorldTransform().translation = deathBasePosition_;
			SetDrawEnable(false);
			ClearRimLight();

			deathPhase_ = EnemyDeathPhase::Burst;
			deathTimer_ = 0.0f;
			deathEffectPosition_ = deathBasePosition_ + CalyxEngine::Vector3{ 0.0f, 0.5f, 0.0f };

			if (!death_.GetData().emitters.empty()) {
				EffectAPI::Play(death_, deathEffectPosition_);
			}
		}
		return false;
	}

	if (deathTimer_ >= kDeathBurstTime) {
		return true; // 演出完了
	}
	return false;
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
	damageRimTime_ = 0.3f;
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

