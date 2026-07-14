#pragma once

#include "LockOnTargetEntry.h"

#include <Engine/System/Event/EventBus.h>

#include <vector>

class LockOnTargetRegistry {
public:
	static LockOnTargetRegistry& Get();

	void Register(const LockOnTargetEntry& entry);
	void Unregister(const CalyxEngine::TransformRef& target);
	void PruneInvalid();

	std::vector<LockOnTargetEntry> GetEntries() const;
	bool IsLockable(const CalyxEngine::TransformRef& target) const;

private:
	LockOnTargetRegistry();
	void OnObjectRemoved(const ObjectRemoved& event);

	std::vector<LockOnTargetEntry> entries_;
	EventBus::Connection objectRemovedConnection_;
};
