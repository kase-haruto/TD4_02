#pragma once

#include <Engine/Scene/Reference/TransformReference.h>

enum class LockOnTargetType {
	Enemy,
};

class ILockOnTarget {
public:
	virtual ~ILockOnTarget() = default;

	virtual LockOnTargetType GetLockOnTargetType() const = 0;
	virtual bool IsLockable() const = 0;
	virtual int GetPriority() const = 0;
	virtual const CalyxEngine::TransformRef& GetTargetTransform() const = 0;
};
