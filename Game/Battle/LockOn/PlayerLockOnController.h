#pragma once

#include "DefaultLockOnTargetQuery.h"
#include "DefaultLockOnTargetSelector.h"
#include "ILockOnStateReader.h"
#include "LockOnSettings.h"

#include <Engine/Foundation/Math/Vector4.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

class PlayerLockOnController final : public ILockOnStateReader {
public:
	~PlayerLockOnController();

	void Initialize();
	void Update(const CalyxEngine::Vector3& playerPosition, float dt);
	void HandleInput(
		const CalyxEngine::Vector3& playerPosition,
		bool lockOnPressed,
		bool unlockPressed,
		bool switchLeftPressed,
		bool switchRightPressed);
	void ShowGui();
	std::vector<CalyxEngine::TransformRef> QueryVisibleTargets(
		const CalyxEngine::Vector3& playerPosition,
		size_t maxCount) const;
	/// 今ロックオンしたら選ばれる対象を、確定させずに覗く
	std::optional<LockOnCandidate> PeekBestTarget(const CalyxEngine::Vector3& playerPosition) const;

	bool IsLockingOn() const override { return currentTarget_.IsAssigned() && currentTarget_.Resolve(); }
	const CalyxEngine::TransformRef& GetCurrentTarget() const override { return currentTarget_; }
	const WorldTransform* ResolveCurrentTarget() const override { return currentTarget_.Resolve(); }
	float GetPlayerTurnSpeed() const override { return settings_.playerTurnSpeed; }

private:
	LockOnQueryContext BuildContext(const CalyxEngine::Vector3& playerPosition) const;
	LockOnCandidate BuildCurrentCandidate(const CalyxEngine::Vector3& playerPosition) const;
	void TryLockOn(const CalyxEngine::Vector3& playerPosition);
	void TrySwitch(const CalyxEngine::Vector3& playerPosition, LockOnSwitchDirection direction);
	void SetTarget(const CalyxEngine::TransformRef& target);
	void ClearTarget();
	void ApplyTargetVisual(const CalyxEngine::TransformRef& target);
	void RestoreTargetVisual();

	struct TargetVisualState {
		CalyxEngine::TransformRef target;
		bool outlineEnabled = false;
		float outlineThickness = 0.0f;
		CalyxEngine::Vector4 outlineColor{};
		bool active = false;
	};

	LockOnSettings settings_;
	DefaultLockOnTargetQuery defaultQuery_;
	DefaultLockOnTargetSelector defaultSelector_;
	ILockOnTargetQuery* query_ = &defaultQuery_;
	ILockOnTargetSelector* selector_ = &defaultSelector_;

	CalyxEngine::TransformRef currentTarget_;
	TargetVisualState targetVisual_;
	float switchCooldownRemaining_ = 0.0f;
	float lostTargetTimer_ = 0.0f;
};
