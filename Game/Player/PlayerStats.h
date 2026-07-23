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

		AddField("lowHpRatio", lowHpRatio).Category("LowHp").Tooltip("この割合以下でリムライトが点滅する");
		AddField("lowHpRimIntensityMin", lowHpRimIntensityMin).Category("LowHp").Tooltip("点滅の一番暗いときの強さ");
		AddField("lowHpRimIntensityMax", lowHpRimIntensityMax).Category("LowHp").Tooltip("点滅の一番明るいときの強さ");
		AddField("lowHpRimSpeed", lowHpRimSpeed).Category("LowHp").Tooltip("点滅の速さ。HPが低いほど最大2倍まで加速する");
	}

	CalyxEngine::ParamPath GetParamPath() const override {
		return { CalyxEngine::ParamDomain::Game, "Player", "Actor/Player" };
	}

	int32_t   maxHp = 10;    // 最大HP

	float knockbackFriction = 8.0f;

	float damageInvincibleTime = 0.5f;
	float damageFlashInterval = 0.05f;
	float respawnInvincibleTime = 1.5f;

	float lowHpRatio = 0.3f;           // HPがこの割合以下で点滅開始
	float lowHpRimIntensityMin = 0.0f;
	float lowHpRimIntensityMax = 6.0f;
	float lowHpRimSpeed = 10.0f;
};
