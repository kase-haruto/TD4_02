#include "PlayerAbility.h"

#include <algorithm>

//engine
#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Graphics/Pipeline/BlendMode/BlendMode.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Scene/Context/SceneContext.h>
#include <Engine/Scene/Utility/SceneUtility.h>

//game
#include "../Player.h"
#include "../Clone/Clone.h"

PlayerAbility::PlayerAbility() {
	param_.LoadParams();
}

void PlayerAbility::Update(Player& player, const PlayerInputState* input, float dt) {
	if (input->cloneAbilityHeld) {
		cloneChargeTime_ += dt;
		UpdateCloneGhost(player, CalculateCloneSpawnDistance());
	}

	if (input->cloneAbilityReleased) {
		const float spawnDistance = CalculateCloneSpawnDistance();
		ClearCloneGhost();
		SpawnClone(player, spawnDistance);
		cloneChargeTime_ = 0.0f;
	} else if (!input->cloneAbilityHeld) {
		ClearCloneGhost();
	}
}

void PlayerAbility::ShowGui() {
	param_.ShowGui();
}

CalyxEngine::SerializableObject& PlayerAbility::SerializableParam() {
	return param_;
}

void PlayerAbility::SpawnClone(Player& player) {
	SpawnClone(player, param_.minCloneDistance);
}

void PlayerAbility::SpawnClone(Player& player, float spawnDistance) {
	RefreshClones();

	if (!CanSpawnClone()) {
		// 最大クローン数に達している場合は生成しない
		return;
	}

	CalyxEngine::Vector3 spawnPosition = CalculateCloneSpawnPosition(player, spawnDistance);
	auto clone = SceneAPI::InstantiatePrefabRoot<PlayerClone>("PlayerClone.prefab", spawnPosition);
	if (!clone) {
		return;
	}
	clone->SetAimOrigin(&player);
	clone->SetRotate(player.GetWorldTransform().rotation);
	clones_.push_back(clone);
}

void PlayerAbility::RefreshClones() {
	clones_.erase(
		std::remove_if(clones_.begin(), clones_.end(), [](const std::weak_ptr<PlayerClone>& clone) {
			return clone.expired();
		}),
		clones_.end());
}

float PlayerAbility::CalculateCloneSpawnDistance() const {
	const float chargeRate = param_.chargeTimeToMax > 0.0f
		? std::clamp(cloneChargeTime_ / param_.chargeTimeToMax, 0.0f, 1.0f)
		: 1.0f;
	return param_.minCloneDistance + (param_.maxCloneDistance - param_.minCloneDistance) * chargeRate;
}

CalyxEngine::Vector3 PlayerAbility::CalculateCloneSpawnPosition(Player& player, float spawnDistance) const {
	CalyxEngine::Vector3 forward =
		CalyxEngine::Quaternion::RotateVector(
			CalyxEngine::Vector3::Forward(),
			player.GetWorldTransform().rotation);
	forward.y = 0.0f;
	if (forward.LengthSquared() <= 1.0e-6f) {
		forward = CalyxEngine::Vector3::Forward();
	} else {
		forward = forward.Normalize();
	}

	return player.GetWorldTransform().translation + forward * spawnDistance;
}

void PlayerAbility::UpdateCloneGhost(Player& player, float spawnDistance) {
	RefreshClones();

	if (!param_.showCloneGhost || !CanSpawnClone()) {
		ClearCloneGhost();
		return;
	}

	const CalyxEngine::Vector3 spawnPosition = CalculateCloneSpawnPosition(player, spawnDistance);
	auto ghost = cloneGhost_.lock();
	if (!ghost) {
		ghost = SceneAPI::InstantiatePrefabRoot<PlayerClone>("PlayerClone.prefab", spawnPosition);
		if (!ghost) {
			return;
		}

		ghost->SetName("PlayerCloneGhost");
		ghost->SetGhost(true);
		ghost->SetTransient(true);
		ghost->SetEnablePicking(false);
		ghost->SetCastShadow(false);
		ghost->SetBlendMode(BlendMode::ALPHA);

		if (auto* collider = ghost->GetCollider()) {
			ColliderConfig config = collider->ExtractConfig();
			config.isCollisionEnabled = false;
			config.isDraw = false;
			collider->ApplyConfig(config);
		}

		cloneGhost_ = ghost;
	}

	ghost->SetTranslate(spawnPosition);
	ghost->SetRotate(player.GetWorldTransform().rotation);
	ghost->SetColor({1.0f, 1.0f, 1.0f, std::clamp(param_.cloneGhostAlpha, 0.0f, 1.0f)});
	ghost->SetBlendMode(BlendMode::ALPHA);
}

void PlayerAbility::ClearCloneGhost() {
	auto ghost = cloneGhost_.lock();
	if (!ghost) {
		cloneGhost_.reset();
		return;
	}

	if (auto* context = SceneContext::Current()) {
		context->RemoveObject(std::static_pointer_cast<SceneObject>(ghost));
	}
	cloneGhost_.reset();
}

bool PlayerAbility::CanSpawnClone() const {
	const int32_t maxCloneCount = param_.maxCloneCount < 0 ? 0 : param_.maxCloneCount;
	return clones_.size() < static_cast<size_t>(maxCloneCount);
}
