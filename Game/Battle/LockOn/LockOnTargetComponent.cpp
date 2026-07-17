#include "LockOnTargetComponent.h"

#include "LockOnTargetRegistry.h"

#include <Engine/Objects/3D/Actor/SceneObject.h>

LockOnTargetComponent::LockOnTargetComponent() {
	AddField("targetTransform", targetTransform_).Category("LockOn");
	AddField("isLockable", isLockable_).Category("LockOn");
	AddField("priority", priority_).Category("LockOn");
}

LockOnTargetComponent::~LockOnTargetComponent() {
	Unregister();
}

void LockOnTargetComponent::Initialize(SceneObject& owner) {
	if (!targetTransform_.IsAssigned()) {
		targetTransform_.SetGuid(owner.GetGuid());
	}
	Register();
}

void LockOnTargetComponent::Register() {
	if (!targetTransform_.IsAssigned()) {
		return;
	}
	LockOnTargetRegistry::Get().Register(BuildEntry());
	registered_ = true;
}

void LockOnTargetComponent::Unregister() {
	if (!registered_) {
		return;
	}
	LockOnTargetRegistry::Get().Unregister(targetTransform_);
	registered_ = false;
}

void LockOnTargetComponent::SetOwnerAlive(bool alive) {
	ownerIsAlive_ = alive;
	if (registered_) {
		LockOnTargetRegistry::Get().Register(BuildEntry());
	}
}

CalyxEngine::Vector3 LockOnTargetComponent::GetWorldPosition() const {
	if (const WorldTransform* transform = targetTransform_.Resolve()) {
		return transform->GetWorldPosition();
	}
	return {};
}

LockOnTargetEntry LockOnTargetComponent::BuildEntry() const {
	LockOnTargetEntry entry;
	entry.targetTransform = targetTransform_;
	entry.type = targetType_;
	entry.priority = priority_;
	entry.isLockable = IsLockable();
	return entry;
}
