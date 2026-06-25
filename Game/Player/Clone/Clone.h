#pragma once

// クローン
#include "../Base/PlayerBase.h"
// game
#include <Demo/Input/PlayerInput.h>
#include "../PlayerMotor.h"
#include "../PlayerDodge.h"
#include "../Sword/Sword.h"
#include "../Ability/PlayerAbility.h"

/*-----------------------------------------------------------------------------------------
 * PlayerClone
 * - Playerのクローン
 * - プレイヤーのコピーであり、プレイヤーと同じ動作をする
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "PlayerClone")
class PlayerClone
	:public PlayerBase {
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	PlayerClone();
	~PlayerClone() override = default;
};
