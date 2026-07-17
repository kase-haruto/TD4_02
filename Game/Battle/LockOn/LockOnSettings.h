#pragma once

#include <Engine/Foundation/Serialization/SerializableObject.h>

struct LockOnSettings : CalyxEngine::SerializableObject {
	LockOnSettings() {
		AddField("searchRadius", searchRadius).Category("Search");
		AddField("breakDistance", breakDistance).Category("Search");
		AddField("maximumAngleDegrees", maximumAngleDegrees).Category("Search");
		AddField("screenCenterWeight", screenCenterWeight).Category("Score");
		AddField("angleWeight", angleWeight).Category("Score");
		AddField("distanceWeight", distanceWeight).Category("Score");
		AddField("priorityWeight", priorityWeight).Category("Score");
		AddField("switchCooldownSeconds", switchCooldownSeconds).Category("Switch");
		AddField("lostTargetGraceSeconds", lostTargetGraceSeconds).Category("Lost");
		AddField("playerTurnSpeed", playerTurnSpeed).Category("Movement");
	}

	CalyxEngine::ParamPath GetParamPath() const override {
		return { CalyxEngine::ParamDomain::Game, "Battle", "LockOnSettings" };
	}

	float searchRadius = 18.0f;
	float breakDistance = 22.0f;
	float maximumAngleDegrees = 70.0f;
	float screenCenterWeight = 2.5f;
	float angleWeight = 1.0f;
	float distanceWeight = 0.75f;
	float priorityWeight = 0.35f;
	float switchCooldownSeconds = 0.25f;
	float lostTargetGraceSeconds = 0.15f;
	float playerTurnSpeed = 12.0f;
};
