#pragma once

#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <Engine/Foundation/Math/Vector3.h>
#include "../../Demo/Input/PlayerInput.h"

#include <memory>
#include <vector>

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
	PlayerAbility();
	/**
	* 更新処理
	*/
	void Update(Player& player, const PlayerInputState* input,float dt);
	void ShowGui();
	CalyxEngine::SerializableObject& SerializableParam();

private:	
	//===================================================================*/
	//						private method
	//===================================================================*/
	/**
	* クローンを生成する
	*/
	void SpawnClone(Player& player);
	void SpawnClone(Player& player, float spawnDistance);

private:
	//===================================================================*/
	//						private variables
	//===================================================================*/
	
	struct AbilityParam:CalyxEngine::SerializableObject{
		AbilityParam() {
			AddField("maxCloneCount", maxCloneCount).Category("Ability").Tooltip("最大クローン数");
			AddField("minCloneDistance", minCloneDistance).Category("Ability").Tooltip("最小クローン生成距離");
			AddField("maxCloneDistance", maxCloneDistance).Category("Ability").Tooltip("最大クローン生成距離");
			AddField("chargeTimeToMax", chargeTimeToMax).Category("Ability").Tooltip("最大距離までのチャージ時間");
			AddField("showCloneGhost", showCloneGhost).Category("Clone Ghost").Tooltip("長押し中にクローンの生成位置を表示する");
			AddField("cloneGhostAlpha", cloneGhostAlpha).Category("Clone Ghost").Tooltip("クローン位置表示の透明度");
		}

		CalyxEngine::ParamPath GetParamPath() const override { return { CalyxEngine::ParamDomain::Game, "PlayerAbility", "Actor/Player/AbilityParam" }; }

		int32_t maxCloneCount = 4; //!< 最大クローン数
		float minCloneDistance = 1.0f; //!< 最小クローン生成距離
		float maxCloneDistance = 5.0f; //!< 最大クローン生成距離
		float chargeTimeToMax = 1.0f; //!< 最大距離までのチャージ時間
		bool showCloneGhost = true; //!< 長押し中にクローン位置を表示する
		float cloneGhostAlpha = 0.35f; //!< クローン位置表示の透明度
	}param_;

	std::vector<std::weak_ptr<PlayerClone>> clones_; //!< 生成されたクローンのリスト
	std::weak_ptr<PlayerClone> cloneGhost_; //!< 長押し中に表示する生成位置のプレビュー
	float cloneChargeTime_ = 0.0f;

	void RefreshClones();
	float CalculateCloneSpawnDistance() const;
	CalyxEngine::Vector3 CalculateCloneSpawnPosition(Player& player, float spawnDistance) const;
	void UpdateCloneGhost(Player& player, float spawnDistance);
	void ClearCloneGhost();
	bool CanSpawnClone() const;
};

