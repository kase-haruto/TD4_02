#pragma once
#include <Engine/Objects/3D/Actor/Actor.h>

// game
#include <Demo/Input/PlayerInput.h>
#include "PlayerMotor.h"
#include "Sword/Sword.h"

/*-----------------------------------------------------------------------------------------
 * Player
 * - メインキャラ
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Player", Icon = "UI/Tool/cube.dds")
class Player
	:public Actor{
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	Player();
	~Player() override = default;

	void Initialize() override;
	void Update(float dt) override;

private:
	//===================================================================*/
	//						private variables
	//===================================================================*/
	PlayerInput input_;
	PlayerMotor motor_;
	std::shared_ptr<Sword> sword_;
};
