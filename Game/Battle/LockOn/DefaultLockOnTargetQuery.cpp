#include "DefaultLockOnTargetQuery.h"

#include "LockOnTargetRegistry.h"

#include <Engine/Foundation/Math/MathUtil.h>
#include <Engine/Graphics/Camera/3d/Camera3d.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <Engine/Objects/3D/Actor/SceneObject.h>
#include <Engine/Objects/Transform/Transform.h>
#include <Engine/Scene/Reference/SceneObjectReference.h>

#include <cmath>

namespace {
	float Clamp(float value, float minValue, float maxValue) {
		if (value < minValue) return minValue;
		if (value > maxValue) return maxValue;
		return value;
	}
}

std::vector<LockOnCandidate> DefaultLockOnTargetQuery::QueryTargets(
	const LockOnQueryContext& context,
	const LockOnSettings& settings) const {
	LockOnTargetRegistry::Get().PruneInvalid();

	std::vector<LockOnCandidate> candidates;
	for (const LockOnTargetEntry& entry : LockOnTargetRegistry::Get().GetEntries()) {
		if (!entry.isLockable) {
			continue;
		}

		const WorldTransform* transform = entry.targetTransform.Resolve();
		if (!transform) {
			continue;
		}

		const CalyxEngine::Vector3 targetPosition = transform->GetWorldPosition();
		const CalyxEngine::Vector3 toTarget = targetPosition - context.playerPosition;
		const float distance = toTarget.Length();
		if (distance > settings.searchRadius) {
			continue;
		}

		if (auto* camera = CameraManager::GetMain3d()) {
			CalyxEngine::SceneObjectRef<SceneObject> objectRef(entry.targetTransform.GetGuid());
			const auto targetObject = objectRef.Resolve();
			if (!targetObject || !camera->IsVisible(targetObject->GetWorldAABB())) {
				continue;
			}
		}

		CalyxEngine::Vector3 cameraToTarget = targetPosition - context.cameraPosition;
		if (cameraToTarget.LengthSquared() <= 0.0001f) {
			continue;
		}
		cameraToTarget = cameraToTarget.Normalize();

		const float dot = Clamp(CalyxEngine::Vector3::Dot(context.cameraForward, cameraToTarget), -1.0f, 1.0f);
		const float angleDegrees = std::acos(dot) * 180.0f / CalyxEngine::kPi;

		LockOnCandidate candidate;
		candidate.entry = entry;
		candidate.worldPosition = targetPosition;
		candidate.distance = distance;
		candidate.angleDegrees = angleDegrees;
		candidate.screenPosition = CalyxEngine::WorldToScreen(targetPosition);
		candidate.hasScreenPosition = true;
		candidates.push_back(candidate);
	}

	return candidates;
}
