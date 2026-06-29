#pragma once

#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <cstdint>

struct PlayerStats : CalyxEngine::SerializableObject {
	PlayerStats() {
		AddField("maxHp", maxHp).Category("Status").Tooltip("最大HP");
		AddField("moveSpeed", moveSpeed).Category("Status").Tooltip("移動速度");
	}

	CalyxEngine::ParamPath GetParamPath() const override {
		return { CalyxEngine::ParamDomain::Game, "Player", "Actor/Player" };
	}

	int32_t   maxHp = 10;    // 最大HP
	float moveSpeed = 2.0f;  // 移動速度
};