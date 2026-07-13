#include "LockOnTargetComponent.h"

LockOnTargetType LockOnTargetComponent::GetLockOnTargetType() const{
	return targetType_;
}

bool LockOnTargetComponent::IsLockable() const{
	return isLockable_ && ownerIsAlive_;
}

