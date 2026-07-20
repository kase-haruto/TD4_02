#pragma once

#include "Base/PlayerBase.h"

// game
#include <Demo/Input/PlayerInput.h>
#include "PlayerMotor.h"
#include "PlayerDodge.h"
#include "PlayerStats.h"
#include "PlayerUI.h"
#include "Sword/Sword.h"
#include "Ability/PlayerAbility.h"
#include <Game/Battle/LockOn/PlayerLockOnController.h>

#include <Engine/Application/Effects/EffectAsset.h>
#include <Engine/Application/Effects/EffectPlayer.h>

#include <cstddef>
#include <vector>


/*-----------------------------------------------------------------------------------------
 * Player
 * - メインキャラ
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Player", Icon = "UI/Tool/cube.dds")
class Player
	:public PlayerBase{
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	Player();
	~Player() override = default;

	void Initialize() override;
	void Update(float dt) override;
	void OnCollisionEnter([[maybe_unused]] Collider* other)override;
	void TakeDamage(int amount);

	/**
	 * \brief 派生パラメータGUI
	 */
	void DerivativeGui() override;

	void SetRespawnPoint(const CalyxEngine::Vector3& p) { respawnPoint_ = p; }

	void SetDodgeEnabled(bool e) { dodge_.SetEnabled(e); }

	bool IsDodgeButtonTriggered() const;
	const ILockOnStateReader& GetLockOnStateReader() const { return lockOn_; }
	std::vector<CalyxEngine::TransformRef> QueryVisibleLockOnTargets(size_t maxCount) const;

private:
	void OnHitByEnemyAttack(Collider* attacker);
	void Respawn();

	void UpdateInvincible(float dt);
	void StartInvincible(float duration);

	void UpdateWalkEffect(bool isWalking);

	//===================================================================*/
	//						private variables
	//===================================================================*/
	PlayerAbility ability_;
	PlayerLockOnController lockOn_;
	PlayerStats stats_;
	PlayerUI    ui_;
	float damageAnimationTimer_ = 0.0f;
	float spiritAnimTimer_ = 0.0f;

	int        currentHp_ = 0;  // 現在HP
	bool       isRespawning_ = false; // リスポーン要求済み(シーン遷移待ち)
	float      invincibleTimer_ = 0.0f;
	CalyxEngine::Vector3 lastCloneAnchor_{};

	CalyxEngine::Vector3 respawnPoint_{};

	bool isWalk_ = false;
	CalyxEngine::EffectAsset walk_;
	CalyxEngine::EffectHandle walkHandle_{};

};
