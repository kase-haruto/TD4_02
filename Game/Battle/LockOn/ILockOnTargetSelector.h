#pragma once

#include "LockOnSettings.h"
#include "LockOnTargetEntry.h"

#include <optional>
#include <vector>

class ILockOnTargetSelector {
public:
	virtual ~ILockOnTargetSelector() = default;

	virtual std::optional<LockOnCandidate> SelectBestTarget(
		const std::vector<LockOnCandidate>& candidates,
		const LockOnQueryContext& context,
		const LockOnSettings& settings) const = 0;

	virtual std::optional<LockOnCandidate> SelectSwitchTarget(
		const std::vector<LockOnCandidate>& candidates,
		const LockOnCandidate& current,
		LockOnSwitchDirection direction,
		const LockOnQueryContext& context,
		const LockOnSettings& settings) const = 0;
};
