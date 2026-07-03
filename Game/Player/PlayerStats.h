#pragma once

#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <cstdint>

struct PlayerStats : CalyxEngine::SerializableObject {
	PlayerStats() {
		AddField("maxHp", maxHp).Category("Status").Tooltip("最大HP");

		AddField("knockbackFriction", knockbackFriction).Category("Knockback").Tooltip("ノックバック減衰。大きいほど早く止まる");
	}

	CalyxEngine::ParamPath GetParamPath() const override {
		return { CalyxEngine::ParamDomain::Game, "Player", "Actor/Player" };
	}

	int32_t   maxHp = 10;    // 最大HP

	float knockbackFriction = 8.0f;
};
