#pragma once

#include <Engine/Foundation/Serialization/SerializableObject.h>
#include "../../Demo/Input/PlayerInput.h"

class Player;
class PlayerClone;

/*-----------------------------------------------------------------------------------------
 * PlayerAbility
 * - 特殊技
 * - クローンを発生させる
 *---------------------------------------------------------------------------------------*/
class PlayerAbility{
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	/**
	* 更新処理
	*/
	void Update(Player& player, const PlayerInputState* input,float dt);

private:	
	//===================================================================*/
	//						private method
	//===================================================================*/
	/**
	* クローンを生成する
	*/
	void SpawnClone(Player& player);

private:
	//===================================================================*/
	//						private variables
	//===================================================================*/
	
	struct AbilityParam:CalyxEngine::SerializableObject{
		AbilityParam() {
			AddField("maxCloneCount", maxCloneCount).Category("Ability").Tooltip("最大クローン数");
		}

		CalyxEngine::ParamPath GetParamPath() const override { return { CalyxEngine::ParamDomain::Game, "PlayerAbility", "Actor/Player/AbilityParam" }; }

		int32_t maxCloneCount = 4; //!< 最大クローン数
	}param_;

	std::vector<std::weak_ptr<PlayerClone>> clones_; //!< 生成されたクローンのリスト
};

