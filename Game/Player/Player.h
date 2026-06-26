#pragma once

#include "Base/PlayerBase.h"

// game
#include <Demo/Input/PlayerInput.h>
#include "PlayerMotor.h"
#include "PlayerDodge.h"
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

	void Update(float dt) override;

	/**
	 * \brief 派生パラメータGUI
	 */
	void DerivativeGui() override;

private:
	//===================================================================*/
	//						private variables
	//===================================================================*/
	PlayerAbility ability_;

};
