#pragma once

#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <cstdint>

struct PlayerStats : CalyxEngine::SerializableObject {
	PlayerStats() {
		AddField("maxHp", maxHp).Category("Status").Tooltip("最大HP");

		AddField("knockbackFriction", knockbackFriction).Category("Knockback").Tooltip("ノックバック減衰。大きいほど早く止まる");

		AddField("damageInvincibleTime", damageInvincibleTime).Category("Damage").Tooltip("被弾時に点滅する時間(無敵時間)");
		AddField("damageFlashInterval", damageFlashInterval).Category("Damage").Tooltip("点滅の切り替え間隔。小さいほど速く点滅する");

		AddField("respawnInvincibleTime", respawnInvincibleTime).Category("Damage").Tooltip("リスポーン直後の無敵時間");
	}

	CalyxEngine::ParamPath GetParamPath() const override {
		return { CalyxEngine::ParamDomain::Game, "Player", "Actor/Player" };
	}

	int32_t   maxHp = 10;    // 最大HP

	float knockbackFriction = 8.0f;

	float damageInvincibleTime = 0.5f;
	float damageFlashInterval = 0.05f;
	float respawnInvincibleTime = 1.5f;
};
