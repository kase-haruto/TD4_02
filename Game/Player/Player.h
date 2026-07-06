#pragma once

#include "Base/PlayerBase.h"

// game
#include <Demo/Input/PlayerInput.h>
#include "PlayerMotor.h"
#include "PlayerDodge.h"
#include "PlayerStats.h"
#include "Sword/Sword.h"
#include "Ability/PlayerAbility.h"



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

private:
	void OnHitByEnemyAttack(Collider* attacker);
	void Respawn();

	//===================================================================*/
	//						private variables
	//===================================================================*/
	PlayerAbility ability_;
	PlayerStats stats_;

	int        currentHp_ = 0;  // 現在HP
	CalyxEngine::Vector3 lastCloneAnchor_{};

	CalyxEngine::Vector3 respawnPoint_{};
};
