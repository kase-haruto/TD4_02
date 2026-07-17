#pragma once

#include "ILockOnTargetSelector.h"

class DefaultLockOnTargetSelector final : public ILockOnTargetSelector {
public:
	std::optional<LockOnCandidate> SelectBestTarget(
		const std::vector<LockOnCandidate>& candidates,
		const LockOnQueryContext& context,
		const LockOnSettings& settings) const override;

	std::optional<LockOnCandidate> SelectSwitchTarget(
		const std::vector<LockOnCandidate>& candidates,
		const LockOnCandidate& current,
		LockOnSwitchDirection direction,
		const LockOnQueryContext& context,
		const LockOnSettings& settings) const override;
};
