#include "LockOnTargetRegistry.h"

#include <Engine/Objects/3D/Actor/SceneObject.h>

#include <algorithm>

LockOnTargetRegistry& LockOnTargetRegistry::Get() {
	static LockOnTargetRegistry instance;
	return instance;
}

LockOnTargetRegistry::LockOnTargetRegistry() {
	objectRemovedConnection_ = EventBus::Subscribe<ObjectRemoved>(
		[this](const ObjectRemoved& event) { OnObjectRemoved(event); });
}

void LockOnTargetRegistry::Register(const LockOnTargetEntry& entry) {
	if (!entry.targetTransform.IsAssigned()) {
		return;
	}

	auto it = std::find_if(entries_.begin(), entries_.end(), [&](const LockOnTargetEntry& current) {
		return current.Matches(entry.targetTransform);
	});
	if (it != entries_.end()) {
		*it = entry;
		return;
	}
	entries_.push_back(entry);
}

void LockOnTargetRegistry::Unregister(const CalyxEngine::TransformRef& target) {
	entries_.erase(std::remove_if(entries_.begin(), entries_.end(), [&](const LockOnTargetEntry& entry) {
		return entry.Matches(target);
	}), entries_.end());
}

void LockOnTargetRegistry::PruneInvalid() {
	entries_.erase(std::remove_if(entries_.begin(), entries_.end(), [](const LockOnTargetEntry& entry) {
		return !entry.targetTransform.Resolve();
	}), entries_.end());
}

std::vector<LockOnTargetEntry> LockOnTargetRegistry::GetEntries() const {
	std::vector<LockOnTargetEntry> result;
	result.reserve(entries_.size());
	for (const auto& entry : entries_) {
		if (entry.targetTransform.Resolve()) {
			result.push_back(entry);
		}
	}
	return result;
}

bool LockOnTargetRegistry::IsLockable(const CalyxEngine::TransformRef& target) const {
	auto it = std::find_if(entries_.begin(), entries_.end(), [&](const LockOnTargetEntry& entry) {
		return entry.Matches(target);
	});
	return it != entries_.end() && it->isLockable && it->targetTransform.Resolve();
}

void LockOnTargetRegistry::OnObjectRemoved(const ObjectRemoved& event) {
	if (!event.sp) {
		return;
	}

	const Guid& removedGuid = event.sp->GetGuid();
	entries_.erase(std::remove_if(entries_.begin(), entries_.end(), [&](const LockOnTargetEntry& entry) {
		return entry.targetTransform.GetGuid() == removedGuid || !entry.targetTransform.Resolve();
	}), entries_.end());
}
