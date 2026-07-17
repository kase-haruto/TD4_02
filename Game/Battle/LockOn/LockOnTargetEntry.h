#pragma once

#include "ILockOnTarget.h"

#include <Engine/Foundation/Math/Vector2.h>
#include <Engine/Foundation/Math/Vector3.h>
#include <Engine/Scene/Reference/TransformReference.h>

struct LockOnTargetEntry {
	CalyxEngine::TransformRef targetTransform;
	LockOnTargetType type = LockOnTargetType::Enemy;
	int priority = 0;
	bool isLockable = true;

	bool Matches(const CalyxEngine::TransformRef& other) const {
		return targetTransform.GetGuid() == other.GetGuid();
	}
};

struct LockOnCandidate {
	LockOnTargetEntry entry;
	CalyxEngine::Vector3 worldPosition{};
	CalyxEngine::Vector2 screenPosition{};
	float distance = 0.0f;
	float angleDegrees = 0.0f;
	bool hasScreenPosition = false;
};

struct LockOnQueryContext {
	CalyxEngine::Vector3 playerPosition{};
	CalyxEngine::Vector3 cameraPosition{};
	CalyxEngine::Vector3 cameraForward = CalyxEngine::Vector3::Forward();
};

enum class LockOnSwitchDirection {
	Left,
	Right,
};
