#pragma once

#include "LockOnSettings.h"
#include "LockOnTargetEntry.h"

#include <vector>

class ILockOnTargetQuery {
public:
	virtual ~ILockOnTargetQuery() = default;

	virtual std::vector<LockOnCandidate> QueryTargets(
		const LockOnQueryContext& context,
		const LockOnSettings& settings) const = 0;
};
