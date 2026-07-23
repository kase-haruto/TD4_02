#include "PlayerLockOnController.h"

#include "LockOnEvents.h"
#include "LockOnTargetRegistry.h"

#include <Engine/Foundation/Math/MathUtil.h>
#include <Engine/Graphics/Camera/3d/Camera3d.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <Engine/Objects/3D/Actor/SceneObject.h>
#include <Engine/Objects/Transform/Transform.h>
#include <Engine/Scene/Reference/SceneObjectReference.h>
#include <Engine/System/Event/EventBus.h>

#include <algorithm>

PlayerLockOnController::~PlayerLockOnController() {
	RestoreTargetVisual();
}

void PlayerLockOnController::Initialize() {
	settings_.LoadParams();
}

void PlayerLockOnController::Update(const CalyxEngine::Vector3& playerPosition, float dt) {
	if (switchCooldownRemaining_ > 0.0f) {
		switchCooldownRemaining_ -= dt;
		if (switchCooldownRemaining_ < 0.0f) {
			switchCooldownRemaining_ = 0.0f;
		}
	}

	if (!currentTarget_.IsAssigned()) {
		lostTargetTimer_ = 0.0f;
		return;
	}

	const WorldTransform* target = currentTarget_.Resolve();
	if (!target || !LockOnTargetRegistry::Get().IsLockable(currentTarget_)) {
		lostTargetTimer_ += dt;
		if (lostTargetTimer_ >= settings_.lostTargetGraceSeconds) {
			ClearTarget();
		}
		return;
	}

	const float distance = (target->GetWorldPosition() - playerPosition).Length();
	if (distance > settings_.breakDistance) {
		ClearTarget();
		return;
	}

	lostTargetTimer_ = 0.0f;
}

void PlayerLockOnController::HandleInput(
	const CalyxEngine::Vector3& playerPosition,
	bool lockOnPressed,
	bool unlockPressed,
	bool switchLeftPressed,
	bool switchRightPressed) {
	if (unlockPressed) {
		ClearTarget();
		return;
	}

	if (lockOnPressed) {
		if (IsLockingOn()) {
			ClearTarget();
		} else {
			TryLockOn(playerPosition);
		}
	}

	if (!IsLockingOn() || switchCooldownRemaining_ > 0.0f) {
		return;
	}

	if (switchLeftPressed) {
		TrySwitch(playerPosition, LockOnSwitchDirection::Left);
	} else if (switchRightPressed) {
		TrySwitch(playerPosition, LockOnSwitchDirection::Right);
	}
}

void PlayerLockOnController::ShowGui() {
	settings_.ShowGui();
}

std::vector<CalyxEngine::TransformRef> PlayerLockOnController::QueryVisibleTargets(
	const CalyxEngine::Vector3& playerPosition,
	size_t maxCount) const {
	const LockOnQueryContext context = BuildContext(playerPosition);
	auto candidates = query_->QueryTargets(context, settings_);
	std::sort(candidates.begin(), candidates.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.distance < rhs.distance;
	});

	std::vector<CalyxEngine::TransformRef> result;
	result.reserve(std::min<size_t>(maxCount, candidates.size()));
	for (const LockOnCandidate& candidate : candidates) {
		if (result.size() >= maxCount) {
			break;
		}
		result.push_back(candidate.entry.targetTransform);
	}
	return result;
}

std::optional<LockOnCandidate> PlayerLockOnController::PeekBestTarget(
	const CalyxEngine::Vector3& playerPosition) const {
	const LockOnQueryContext context = BuildContext(playerPosition);
	const auto candidates = query_->QueryTargets(context, settings_);
	return selector_->SelectBestTarget(candidates, context, settings_);
}

LockOnQueryContext PlayerLockOnController::BuildContext(const CalyxEngine::Vector3& playerPosition) const {
	LockOnQueryContext context;
	context.playerPosition = playerPosition;
	context.cameraPosition = playerPosition;
	context.cameraForward = CalyxEngine::Vector3::Forward();

	if (auto* camera = CameraManager::GetMain3d()) {
		context.cameraPosition = camera->GetWorldTransform().GetWorldPosition();
		context.cameraForward = camera->GetForward();
		if (context.cameraForward.LengthSquared() > 0.0001f) {
			context.cameraForward = context.cameraForward.Normalize();
		}
	}
	return context;
}

LockOnCandidate PlayerLockOnController::BuildCurrentCandidate(const CalyxEngine::Vector3& playerPosition) const {
	LockOnCandidate candidate;
	candidate.entry.targetTransform = currentTarget_;
	if (const WorldTransform* target = currentTarget_.Resolve()) {
		candidate.worldPosition = target->GetWorldPosition();
		candidate.distance = (candidate.worldPosition - playerPosition).Length();
		candidate.screenPosition = CalyxEngine::WorldToScreen(candidate.worldPosition);
		candidate.hasScreenPosition = true;
	}
	return candidate;
}

void PlayerLockOnController::TryLockOn(const CalyxEngine::Vector3& playerPosition) {
	if (const auto selected = PeekBestTarget(playerPosition)) {
		SetTarget(selected->entry.targetTransform);
	}
}

void PlayerLockOnController::TrySwitch(const CalyxEngine::Vector3& playerPosition, LockOnSwitchDirection direction) {
	const LockOnQueryContext context = BuildContext(playerPosition);
	const auto candidates = query_->QueryTargets(context, settings_);
	const LockOnCandidate current = BuildCurrentCandidate(playerPosition);
	const auto selected = selector_->SelectSwitchTarget(candidates, current, direction, context, settings_);
	if (selected) {
		SetTarget(selected->entry.targetTransform);
		switchCooldownRemaining_ = settings_.switchCooldownSeconds;
	}
}

void PlayerLockOnController::SetTarget(const CalyxEngine::TransformRef& target) {
	if (currentTarget_.IsAssigned() && currentTarget_.GetGuid() == target.GetGuid()) {
		return;
	}

	const CalyxEngine::TransformRef previous = currentTarget_;
	RestoreTargetVisual();
	currentTarget_ = target;
	ApplyTargetVisual(currentTarget_);
	lostTargetTimer_ = 0.0f;

	if (previous.IsAssigned()) {
		EventBus::Publish(LockOnChangedEvent{ previous, currentTarget_ });
	} else {
		EventBus::Publish(LockOnStartedEvent{ currentTarget_ });
	}
}

void PlayerLockOnController::ClearTarget() {
	if (!currentTarget_.IsAssigned()) {
		return;
	}

	const CalyxEngine::TransformRef previous = currentTarget_;
	RestoreTargetVisual();
	currentTarget_.Clear();
	lostTargetTimer_ = 0.0f;
	EventBus::Publish(LockOnClearedEvent{ previous });
}

void PlayerLockOnController::ApplyTargetVisual(const CalyxEngine::TransformRef& target) {
	CalyxEngine::SceneObjectRef<SceneObject> objectRef(target.GetGuid());
	const auto object = objectRef.Resolve();
	if (!object) {
		return;
	}

	targetVisual_.target = target;
	targetVisual_.outlineEnabled = object->IsOutlineEnabled();
	targetVisual_.outlineThickness = object->GetOutlineSettings().thickness;
	targetVisual_.outlineColor = object->GetOutlineSettings().color;
	targetVisual_.active = true;

	object->SetOutlineEnabled(true);
	object->SetOutlineColor({ 1.0f, 0.05f, 0.02f, 1.0f });
	object->SetOutlineThickness(0.07f);
}

void PlayerLockOnController::RestoreTargetVisual() {
	if (!targetVisual_.active) {
		return;
	}

	CalyxEngine::SceneObjectRef<SceneObject> objectRef(targetVisual_.target.GetGuid());
	const auto object = objectRef.Resolve();
	if (object) {
		object->SetOutlineEnabled(targetVisual_.outlineEnabled);
		object->SetOutlineColor(targetVisual_.outlineColor);
		object->SetOutlineThickness(targetVisual_.outlineThickness);
	}

	targetVisual_ = {};
}
