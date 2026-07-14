#include "DefaultLockOnTargetSelector.h"

#include <Engine/Application/System/Environment.h>

#include <algorithm>
#include <cmath>

namespace {
	float Clamp01(float value) {
		if (value < 0.0f) return 0.0f;
		if (value > 1.0f) return 1.0f;
		return value;
	}

	float ScreenCenterDistance(const LockOnCandidate& candidate) {
		if (!candidate.hasScreenPosition) {
			return 1.0f;
		}
		const CalyxEngine::Vector2 center{
			static_cast<float>(kGameWidth) * 0.5f,
			static_cast<float>(kGameHeight) * 0.5f
		};
		const CalyxEngine::Vector2 delta = candidate.screenPosition - center;
		const float maxDistance = std::sqrt(center.x * center.x + center.y * center.y);
		return maxDistance > 0.0f ? Clamp01(delta.Length() / maxDistance) : 1.0f;
	}

	float Score(const LockOnCandidate& candidate, const LockOnSettings& settings) {
		const float angleNormalized = settings.maximumAngleDegrees > 0.0f
			? Clamp01(candidate.angleDegrees / settings.maximumAngleDegrees)
			: 1.0f;
		const float distanceNormalized = settings.searchRadius > 0.0f
			? Clamp01(candidate.distance / settings.searchRadius)
			: 1.0f;

		return ScreenCenterDistance(candidate) * settings.screenCenterWeight
			+ angleNormalized * settings.angleWeight
			+ distanceNormalized * settings.distanceWeight
			- static_cast<float>(candidate.entry.priority) * settings.priorityWeight;
	}
}

std::optional<LockOnCandidate> DefaultLockOnTargetSelector::SelectBestTarget(
	const std::vector<LockOnCandidate>& candidates,
	[[maybe_unused]] const LockOnQueryContext& context,
	const LockOnSettings& settings) const {
	auto it = std::min_element(candidates.begin(), candidates.end(), [&](const auto& lhs, const auto& rhs) {
		return Score(lhs, settings) < Score(rhs, settings);
	});
	if (it == candidates.end()) {
		return std::nullopt;
	}
	return *it;
}

std::optional<LockOnCandidate> DefaultLockOnTargetSelector::SelectSwitchTarget(
	const std::vector<LockOnCandidate>& candidates,
	const LockOnCandidate& current,
	LockOnSwitchDirection direction,
	[[maybe_unused]] const LockOnQueryContext& context,
	const LockOnSettings& settings) const {
	if (!current.hasScreenPosition) {
		return SelectBestTarget(candidates, context, settings);
	}

	constexpr float kSwitchEpsilon = 8.0f;
	std::optional<LockOnCandidate> best;
	float bestScore = 0.0f;

	for (const LockOnCandidate& candidate : candidates) {
		if (candidate.entry.Matches(current.entry.targetTransform) || !candidate.hasScreenPosition) {
			continue;
		}

		const float dx = candidate.screenPosition.x - current.screenPosition.x;
		if (direction == LockOnSwitchDirection::Right && dx <= kSwitchEpsilon) {
			continue;
		}
		if (direction == LockOnSwitchDirection::Left && dx >= -kSwitchEpsilon) {
			continue;
		}

		const float dy = candidate.screenPosition.y - current.screenPosition.y;
		const float switchScore = std::abs(dx) * 0.004f
			+ std::abs(dy) * 0.002f
			+ Score(candidate, settings);

		if (!best || switchScore < bestScore) {
			best = candidate;
			bestScore = switchScore;
		}
	}

	return best;
}
