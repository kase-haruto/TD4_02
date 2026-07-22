#include "Player.h"

#include <Game/Collision/CollisionLayerUtil.h>
#include <Game/World/KillPlane.h>
#include <Game/World/RespawnState.h>
#include <Game/World/EnemyState.h>
#include <Game/World/WorldState.h>

#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <Engine/Physics/Character/CharacterMovementComponent.h>
#include <Engine/Scene/Utility/SceneUtility.h>
#include <Engine/Scene/Context/SceneContext.h>
#include <Engine/Foundation/Clock/ClockManager.h>
#include <algorithm>
#include <cmath>
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
	lockOn_.Initialize();
	motor_.SetLockOnStateReader(&lockOn_);

	if (collider_) {
		collider_->SetOwner(this);
	}

	if (RespawnState::Get().ConsumePendingApply()) {
		const CalyxEngine::Vector3 p = RespawnState::Get().Position();
		SetPosition(p);
		respawnPoint_ = p;
		lastCloneAnchor_ = p;
	}
	// 死亡リスポーンなら満タン、エリア移動なら前のエリアのHPを引き継ぐ
	if (RespawnState::Get().ConsumeJustRespawned()) {
		StartInvincible(stats_.respawnInvincibleTime);
	} else if (WorldState::Get().IsPlayerHpStored()) {
		currentHp_ = std::clamp(WorldState::Get().PlayerHp(), 1, stats_.maxHp);
	}
	WorldState::Get().SetPlayerHp(currentHp_);

	walk_.Load("playerWalk");
	dodgeEffect_.Load("playerDodge");

	ui_.Initialize(ability_.MaxCloneCount());
}

/////////////////////////////////////////////////////////////////////////////////////////
//			更新
/////////////////////////////////////////////////////////////////////////////////////////
void Player::Update(float dt) {
	WorldState::Get().SetPlayerHp(currentHp_);   // エリア移動で引き継ぐ用(途中returnがあるので先頭で保存)
	damageAnimationTimer_ = damageAnimationTimer_ > dt ? damageAnimationTimer_ - dt : 0.0f;
	UpdateInvincible(dt);
	UpdateLowHpRim(dt);

	// 落下死。床をすり抜けたら通常の死亡と同じ扱いにしてリスポーンさせる
	if (currentHp_ > 0 && KillPlane::IsFallenOut(GetWorldPosition())) {
		currentHp_ = 0;
	}

	if (currentHp_ <= 0) {
		// 遷移が終わるまでHP0のまま見せる(UIも0で更新しておく)
		ui_.Update(currentHp_, stats_.maxHp, ability_.BuildSlotViews());
		UpdateWalkEffect(false);
		if (!isRespawning_) {
			isRespawning_ = true;
			Respawn();
		}
		FlushAnimation(dt);
		Actor::Update(dt);
		return;
	}
	ability_.CooldownUpdate(dt);
	ui_.Update(currentHp_, stats_.maxHp, ability_.BuildSlotViews());

	if (UpdateKnockback(dt)) {   // ノックバック中は技も入力も止める
		UpdateWalkEffect(false);
		FlushAnimation(dt);
		Actor::Update(dt);
		lastCloneAnchor_ = GetWorldPosition();
		return;
	}

	ability_.MoveClones(GetWorldPosition() - lastCloneAnchor_);
	lastCloneAnchor_ = GetWorldPosition();

	input_.Update();

	const PlayerInputState& in = input_.GetState();
	lockOn_.HandleInput(
		GetWorldPosition(),
		in.lockOnPressed,
		in.unlockPressed,
		in.switchLeftPressed,
		in.switchRightPressed);
	lockOn_.Update(GetWorldPosition(), dt);

	// 回避を先に処理,回避中は移動/向き/ジャンプを受け付けない
	dodge_.Update(this, in, dt);
	if (dodge_.IsStartDodge()) {
		EffectAPI::Play(dodgeEffect_, GetWorldPosition());
	}
	// 回避中はアビリティの入力を受け付けない（チャージは中断）
	ability_.Update(*this, dodge_.IsDodging() ? nullptr : &in, dt);

	// 回避中は攻撃しない
	if (!dodge_.IsDodging()) {
		attack_.Update(*this, in, dt);
	}

	// 回避中・攻撃中は通常移動しない
	const bool canMove = !dodge_.IsDodging() && !attack_.BlocksMovement();
	if (canMove) {
		motor_.Update(this, in, dt);
	}
	UpdateWalkEffect(canMove && in.move.LengthSquared() > 0.0f && GetCharacterMovement().IsMovingOnGround());

	spiritAnimTimer_ = spiritAnimTimer_ > dt ? spiritAnimTimer_ - dt : 0.0f;
	if (ability_.ConsumeJustSpawned()) {
		spiritAnimTimer_ = 0.3f;
	}
	if (ability_.IsCharging() || spiritAnimTimer_ > 0.0f) {
		RequestAnimation(PlayerAnimationID::Spirit);
	}
	if (damageAnimationTimer_ > 0.0f) {
		RequestAnimation(PlayerAnimationID::Damage);
	}
	FlushAnimation(dt);
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
	StartInvincible(stats_.damageInvincibleTime);
	PlayAnimation(PlayerAnimationID::Damage);
}

void Player::DerivativeGui(){
	PlayerBase::DerivativeGui();
	ability_.ShowGui();
	lockOn_.ShowGui();
	stats_.ShowGui();
}

bool Player::IsDodgeButtonTriggered() const {
	return input_.IsTriggerAction(InputAction::Dash)
		|| input_.IsTriggerGamepadAction(InputAction::Dash)
		|| CalyxFoundation::Input::TriggerMouseButton(CalyxFoundation::MouseButton::Right);
}

std::vector<CalyxEngine::TransformRef> Player::QueryVisibleLockOnTargets(size_t maxCount) const {
	return lockOn_.QueryVisibleTargets(GetWorldPosition(), maxCount);
}

void Player::OnHitByEnemyAttack(Collider* attacker) {
	if (dodge_.IsInvincible() || invincibleTimer_ > 0.0f) {
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
	// HPは遷移先で Initialize() が満タンに戻すので、ここでは戻さない
	knockbackVelocity_ = {};
	SetVelocity({});
	dodge_.Reset();
	attack_.Reset();
	ability_.ClearClones();
	invincibleTimer_ = 0.0f;
	SetDrawEnable(true);
	ClearRimLight();
	isLowHpRim_ = false;

	isWalk_ = false;

	EnemyState::Get().Clear();

	ClockManager::GetInstance()->SetTimeScale(1.0f);

	auto& rs = RespawnState::Get();
	const std::string dst = rs.Has() ? rs.ScenePath() : (SceneContext::Current() ? SceneContext::Current()->GetScenePath() : "");
	if (dst.empty()) {
		// 遷移先が無い＝Initialize()が走らないので、その場で復帰させる
		currentHp_ = stats_.maxHp;
		isRespawning_ = false;
		return;
	}

	rs.MarkJustRespawned();
	if (rs.Has()) {
		rs.MarkPendingApply();
	}
	SceneAPI::RequestSceneChange(std::filesystem::path(dst));
}

void Player::UpdateInvincible(float dt) {
	if (invincibleTimer_ <= 0.0f) {
		return;
	}

	invincibleTimer_ = invincibleTimer_ > dt ? invincibleTimer_ - dt : 0.0f;

	if (invincibleTimer_ <= 0.0f) {
		SetDrawEnable(true);
		ClearRimLight();
		return;
	}

	// 残り時間を間隔で割った回数の偶奇で表示/非表示を切り替える
	const float interval = (std::max)(stats_.damageFlashInterval, 0.001f);
	const int   step = static_cast<int>(invincibleTimer_ / interval);
	SetDrawEnable((step % 2) == 0);
}

void Player::UpdateLowHpRim(float dt) {
	// 被弾直後は赤いダメージリムを優先する
	if (invincibleTimer_ > 0.0f) {
		isLowHpRim_ = false;
		return;
	}

	const float hpRate = stats_.maxHp > 0
		? static_cast<float>(currentHp_) / static_cast<float>(stats_.maxHp)
		: 0.0f;

	// 死亡中と、まだHPに余裕があるときは点滅させない
	if (currentHp_ <= 0 || hpRate > stats_.lowHpRatio) {
		if (isLowHpRim_) {          // 点滅していたときだけ消す
			isLowHpRim_ = false;
			lowHpRimPhase_ = 0.0f;
			ClearRimLight();
		}
		return;
	}

	// HPが低いほど速くする(0=しきい値ちょうど, 1=瀕死)
	const float danger = stats_.lowHpRatio > 0.0f
		? std::clamp(1.0f - hpRate / stats_.lowHpRatio, 0.0f, 1.0f)
		: 1.0f;

	constexpr float kTwoPi = 6.28318530718f;
	lowHpRimPhase_ += dt * stats_.lowHpRimSpeed * (1.0f + danger);
	if (lowHpRimPhase_ > kTwoPi) {
		lowHpRimPhase_ -= kTwoPi;
	}

	// 0→1→0 で滑らかに脈動させる
	const float t = 0.5f * (1.0f - std::cos(lowHpRimPhase_));
	const float intensity = stats_.lowHpRimIntensityMin
		+ (stats_.lowHpRimIntensityMax - stats_.lowHpRimIntensityMin) * t;

	// モデル差し替えで消えても復帰できるよう毎フレーム貼り直す
	isLowHpRim_ = true;
	SetRimLight({ 1.0f, 0.15f, 0.15f, 1.0f }, intensity, 1.0f);
}

void Player::StartInvincible(float duration) {
	invincibleTimer_ = duration;
	SetRimLight({ 1.0f, 0.1f, 0.1f, 1.0f }, 8.0f, 1.0f);
}

void Player::UpdateWalkEffect(bool isWalking) {
	if (isWalking) {
		if (!isWalk_) {
			walkHandle_ = EffectAPI::Play(walk_, GetWorldPosition());
			isWalk_ = true;
		} else {
			// 再生中はエミッターをプレイヤーに追従させる
			EffectAPI::Player()->SetTransform(
				walkHandle_,
				GetWorldPosition(),
				CalyxEngine::Quaternion::MakeIdentity(),
				{ 1.0f, 1.0f, 1.0f });
		}
	} else if (isWalk_) {
		EffectAPI::Stop(walkHandle_);
		walkHandle_ = {};
		isWalk_ = false;
	}
}
