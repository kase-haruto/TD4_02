#pragma once

#include <Engine/Scene/Reference/TransformReference.h>

class WorldTransform;

class ILockOnStateReader {
public:
	virtual ~ILockOnStateReader() = default;

	virtual bool IsLockingOn() const = 0;
	virtual const CalyxEngine::TransformRef& GetCurrentTarget() const = 0;
	virtual const WorldTransform* ResolveCurrentTarget() const = 0;
	virtual float GetPlayerTurnSpeed() const = 0;
};
