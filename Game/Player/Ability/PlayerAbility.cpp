#include "PlayerAbility.h"

#include <algorithm>

//engine
#include <Engine/Scene/Utility/SceneUtility.h>

//game
#include "../Player.h"
#include "../Clone/Clone.h"

void PlayerAbility::Update(Player& player, const PlayerInputState* input, float dt) {
	if (input->cloneAbilityPressed) {
		SpawnClone(player);
	}
}

void PlayerAbility::SpawnClone(Player& player) {
	if (clones_.size() >= param_.maxCloneCount) {
		// 最大クローン数に達している場合は生成しない
		return;
	}

	//とりあえずplayerの位置にクローンを生成する
	CalyxEngine::Vector3 spawnPosition = player.GetWorldTransform().translation + CalyxEngine::Vector3(1.0f, 0.0f, 0.0f); // プレイヤーの右側に生成
	auto clone = SceneAPI::InstantiatePrefabRoot<PlayerClone>("PlayerClone.prefab", spawnPosition);
	if (!clone) {
		return;
	}
	clones_.push_back(clone);
	
}
