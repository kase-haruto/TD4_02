#pragma once

#include <Engine/Objects/3D/Actor/Actor.h>

// game
#include <Demo/Input/PlayerInput.h>
#include "../PlayerMotor.h"
#include "../PlayerDodge.h"
#include "../Sword/Sword.h"
#include "../Ability/PlayerAbility.h"

/*-----------------------------------------------------------------------------------------
 * PlayerBase
 * - Playerの基底クラス
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "PlayerBase", Icon = "UI/Tool/cube.dds")
class PlayerBase
	:public Actor {
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	PlayerBase();
	~PlayerBase() override = default;

	virtual void Initialize() override;
	virtual void Update(float dt) override;

private:
	//===================================================================*/
	//						private variables
	//===================================================================*/
	PlayerInput input_;
	PlayerMotor motor_;
	PlayerDodge dodge_;
};
