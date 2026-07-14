#pragma once

#include "ILockOnTargetQuery.h"

class DefaultLockOnTargetQuery final : public ILockOnTargetQuery {
public:
	std::vector<LockOnCandidate> QueryTargets(
		const LockOnQueryContext& context,
		const LockOnSettings& settings) const override;
};
