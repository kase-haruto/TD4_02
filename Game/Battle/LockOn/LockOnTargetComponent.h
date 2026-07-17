#pragma once

#include "ILockOnTarget.h"
#include "LockOnTargetEntry.h"

#include <Engine/Foundation/Serialization/SerializableObject.h>

class SceneObject;

class LockOnTargetComponent final : public ILockOnTarget, public CalyxEngine::SerializableObject {
public:
	LockOnTargetComponent();
	~LockOnTargetComponent() override;

	void Initialize(SceneObject& owner);
	void Register();
	void Unregister();
	void SetOwnerAlive(bool alive);

	LockOnTargetType GetLockOnTargetType() const override { return targetType_; }
	bool IsLockable() const override { return isLockable_ && ownerIsAlive_; }
	int GetPriority() const override { return priority_; }
	const CalyxEngine::TransformRef& GetTargetTransform() const override { return targetTransform_; }

	CalyxEngine::Vector3 GetWorldPosition() const;

private:
	LockOnTargetEntry BuildEntry() const;

	LockOnTargetType targetType_ = LockOnTargetType::Enemy;
	CalyxEngine::TransformRef targetTransform_;
	bool isLockable_ = true;
	bool ownerIsAlive_ = true;
	int priority_ = 0;
	bool registered_ = false;
};
