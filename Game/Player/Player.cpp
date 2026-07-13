#include "Player.h"

#include <Game/Collision/CollisionLayerUtil.h>
#include <Game/World/RespawnState.h>
#include <Game/World/EnemyState.h>

#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <Engine/Physics/Character/CharacterMovementComponent.h>
#include <Engine/Scene/Utility/SceneUtility.h>
#include <Engine/Scene/Context/SceneContext.h>
#include <filesystem>


/////////////////////////////////////////////////////////////////////////////////////////
//			ctor / dtor
/////////////////////////////////////////////////////////////////////////////////////////
Player::Player() {
	stats_.LoadParams();
}


void Player::Initialize() {
	PlayerBase::Initialize();
	currentHp_ = stats_.maxHp;
	knockbackVelocity_ = {};
	lastCloneAnchor_ = worldTransform_.translation;
	respawnPoint_ = worldTransform_.translation;

	if (collider_) {
		collider_->SetOwner(this);
	}

	if (RespawnState::Get().ConsumePendingApply()) {
		const CalyxEngine::Vector3 p = RespawnState::Get().Position();
		SetPosition(p);
		respawnPoint_ = p;
		lastCloneAnchor_ = p;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//			更新
/////////////////////////////////////////////////////////////////////////////////////////
void Player::Update(float dt) {
	damageAnimationTimer_ = damageAnimationTimer_ > dt ? damageAnimationTimer_ - dt : 0.0f;
	if (currentHp_ <= 0) {
		Respawn();
		Actor::Update(dt);
		return;
	}

	if (UpdateKnockback(dt)) {   // ← base の実装。ノックバック中は技も入力も止める
		Actor::Update(dt);
		lastCloneAnchor_ = GetWorldPosition();
		return;
	}

	ability_.MoveClones(GetWorldPosition() - lastCloneAnchor_);
	lastCloneAnchor_ = GetWorldPosition();

	input_.Update();

	const PlayerInputState& in = input_.GetState();
	ability_.Update(*this, &in, dt);

	// 回避を先に処理,回避中は移動/向き/ジャンプを受け付けない
	dodge_.Update(this, in, dt);

	// 回避中は攻撃しない
	if (!dodge_.IsDodging()) {
		attack_.Update(*this, in, dt);
	}

	// 回避中・攻撃中は通常移動しない
	if (!dodge_.IsDodging() && !attack_.BlocksMovement()) {
		motor_.Update(this, in, dt);
	}
	if (in.cloneAbilityHeld && !dodge_.IsDodging() && !attack_.BlocksMovement()) {
		PlayAnimation(PlayerAnimationID::Spirit);
	}
	if (damageAnimationTimer_ > 0.0f) {
		PlayAnimation(PlayerAnimationID::Damage);
	}
	Actor::Update(dt);
}

void Player::OnCollisionEnter(Collider* other) {
	if (!other) {
		return;
	}

	const auto enemyAttackLayer = GameCollision::FindLayerId("EnemyAttack");
	if (enemyAttackLayer && other->GetLayerId() == *enemyAttackLayer) {
		OnHitByEnemyAttack(other);
	}
}

void Player::TakeDamage(int amount) {
	if (amount <= 0) {
		return;
	}
	currentHp_ -= amount;
	if (currentHp_ < 0) {
		currentHp_ = 0;
	}
	damageAnimationTimer_ = 0.25f;
	PlayAnimation(PlayerAnimationID::Damage);
}

void Player::DerivativeGui(){
	PlayerBase::DerivativeGui();
	ability_.ShowGui();
}

bool Player::IsDodgeButtonTriggered() const {
	return input_.IsTriggerAction(InputAction::Dash)
		|| input_.IsTriggerGamepadAction(InputAction::Dash)
		|| CalyxFoundation::Input::TriggerMouseButton(CalyxFoundation::MouseButton::Right);
}

void Player::OnHitByEnemyAttack(Collider* attacker) {
	if (dodge_.IsInvincible()) {
		return;
	}

	// 攻撃側ヒットボックスから攻撃情報を読む
	auto* hitbox = attacker ? dynamic_cast<Sword*>(attacker->GetOwner()) : nullptr;
	const int   damage = hitbox ? hitbox->GetDamage() : 1;
	const float power = hitbox ? hitbox->GetKnockbackPower() : 0.0f;

	TakeDamage(damage);

	if (power <= 0.0f || !attacker || !attacker->GetOwner()) {
		return;
	}
	CalyxEngine::Vector3 dir =
		GetWorldPosition() - attacker->GetOwner()->GetWorldTransform().translation;
	dir.y = 0.0f;
	if (dir.LengthSquared() <= 0.0001f) {
		return;
	}
	CalyxEngine::Vector3 vel = dir.Normalize() * power;

	ApplyKnockback(vel, stats_.knockbackFriction);
	ability_.ApplyKnockbackToClones(vel, stats_.knockbackFriction);
}

void Player::Respawn() {
	currentHp_ = stats_.maxHp;   // 体力を元に
	knockbackVelocity_ = {};
	SetVelocity({});
	dodge_.Reset();
	attack_.Reset();
	ability_.ClearClones();

	EnemyState::Get().Clear();

	auto& rs = RespawnState::Get();
	const std::string dst = rs.Has() ? rs.ScenePath() : (SceneContext::Current() ? SceneContext::Current()->GetScenePath() : "");
	if (dst.empty()) return;

	if (rs.Has()) {
		rs.MarkPendingApply();
	}
	SceneAPI::RequestSceneChange(std::filesystem::path(dst));
}
