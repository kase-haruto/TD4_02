#include "PlayerAbility.h"

#include <algorithm>

//engine
#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Graphics/Pipeline/BlendMode/BlendMode.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Scene/Context/SceneContext.h>
#include <Engine/Scene/Utility/SceneUtility.h>
#include <Engine/Foundation/Clock/ClockManager.h>

//game
#include "../Player.h"
#include "../Clone/Clone.h"

PlayerAbility::PlayerAbility() {
	param_.LoadParams();
	EnsureSlots();
}

void PlayerAbility::Update(Player& player, const PlayerInputState* input, float dt) {
	EnsureSlots();
	ReconcileSlots();
	UpdateSlotCooldowns(ClockManager::GetInstance()->GetDeltaTime());

	if (!input) {
		// 入力を受け付けない状態（回避中など）。チャージを中断して元に戻す
		if (cloneChargeTime_ > 0.0f) {
			ClockManager::GetInstance()->SetTimeScale(1.0f);
			cloneChargeTime_ = 0.0f;
		}
		ClearCloneGhost();
		return;
	}

	if (input->cloneAbilityHeld) {
		cloneChargeTime_ += dt;
		ClockManager::GetInstance()->SetTimeScale(0.5f);
		UpdateCloneGhost(player, CalculateCloneSpawnDistance());
	}

	if (input->cloneAbilityReleased) {
		const float spawnDistance = CalculateCloneSpawnDistance();
		ClockManager::GetInstance()->SetTimeScale(1.0f);
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

void PlayerAbility::ApplyKnockbackToClones(const CalyxEngine::Vector3& velocity, float friction) {
	for (auto& slot : slots_) {
		if (slot.state != CloneSlot::State::InUse) {
			continue;
		}
		if (auto clone = slot.clone.lock()) {
			clone->ApplyKnockback(velocity, friction);
		}
	}
}

void PlayerAbility::MoveClones(const CalyxEngine::Vector3& delta) {
	if (delta.LengthSquared() <= 0.0f) {
		return;
	}
	for (auto& slot : slots_) {
		if (slot.state != CloneSlot::State::InUse) {
			continue;
		}
		if (auto clone = slot.clone.lock()) {
			clone->AddWorldOffset(delta);
		}
	}
}

void PlayerAbility::ClearClones() {
	for (auto& slot : slots_) {
		if (auto clone = slot.clone.lock()) {
			if (auto* context = SceneContext::Current()) {
				context->RemoveObject(std::static_pointer_cast<SceneObject>(clone));
			}
		}
		slot.state = CloneSlot::State::Free;
		slot.clone.reset();
		slot.cooldown = 0.0f;
	}
	ClearCloneGhost();
}

void PlayerAbility::OnCloneWallDeath(PlayerClone* clone) {
	// 壁で消えたクローンのスロットを特定してクールタイム開始（その場で回復させる）
	for (auto& slot : slots_) {
		if (slot.state == CloneSlot::State::InUse && slot.clone.lock().get() == clone) {
			slot.state = CloneSlot::State::Cooldown;
			slot.cooldown = param_.cloneLockDuration;
			slot.clone.reset();
			return;
		}
	}
}

void PlayerAbility::SpawnClone(Player& player) {
	SpawnClone(player, param_.minCloneDistance);
}

void PlayerAbility::SpawnClone(Player& player, float spawnDistance) {
	EnsureSlots();
	ReconcileSlots();

	// 先頭の空きスロットを使う
	CloneSlot* freeSlot = nullptr;
	for (auto& slot : slots_) {
		if (slot.state == CloneSlot::State::Free) {
			freeSlot = &slot;
			break;
		}
	}
	if (!freeSlot) {
		// 空きスロットが無い（最大数 or クール中で埋まっている）
		return;
	}

	CalyxEngine::Vector3 spawnPosition = CalculateCloneSpawnPosition(player, spawnDistance);
	auto clone = SceneAPI::InstantiatePrefabRoot<PlayerClone>("PlayerClone.prefab", spawnPosition);
	if (!clone) {
		return;
	}
	clone->SetAimOrigin(&player);
	clone->SetOwnerAbility(this);
	clone->SetRotate(player.GetWorldTransform().rotation);

	freeSlot->state = CloneSlot::State::InUse;
	freeSlot->clone = clone;
}

void PlayerAbility::EnsureSlots() {
	const int want = param_.maxCloneCount < 0 ? 0 : param_.maxCloneCount;
	if (static_cast<int>(slots_.size()) < want) {
		slots_.resize(static_cast<size_t>(want));
	}
}

void PlayerAbility::ReconcileSlots() {
	// 壁以外の理由（シーン破棄など）でクローンが消えたスロットを空きに戻す
	for (auto& slot : slots_) {
		if (slot.state == CloneSlot::State::InUse && slot.clone.expired()) {
			slot.state = CloneSlot::State::Free;
			slot.clone.reset();
		}
	}
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
	for (const auto& slot : slots_) {
		if (slot.state == CloneSlot::State::Free) {
			return true;
		}
	}
	return false;
}

void PlayerAbility::UpdateSlotCooldowns(float dt) {
	for (auto& slot : slots_) {
		if (slot.state != CloneSlot::State::Cooldown) {
			continue;
		}
		slot.cooldown -= dt;
		if (slot.cooldown <= 0.0f) {
			slot.cooldown = 0.0f;
			slot.state = CloneSlot::State::Free;
		}
	}
}

std::vector<PlayerAbility::CloneSlotView> PlayerAbility::BuildSlotViews() const {
	std::vector<CloneSlotView> views;
	views.reserve(slots_.size());

	const float duration = param_.cloneLockDuration > 0.0f ? param_.cloneLockDuration : 1.0f;

	// スロット順そのまま（ピップ i ⇄ スロット i）。並び替えなし
	for (const auto& slot : slots_) {
		CloneSlotView view;
		switch (slot.state) {
		case CloneSlot::State::Free:
			view.state = CloneSlotView::State::Usable;
			break;
		case CloneSlot::State::InUse:
			// 既に消えている場合は空き扱い（次のUpdateで解放される）
			view.state = slot.clone.expired()
				? CloneSlotView::State::Usable
				: CloneSlotView::State::InUse;
			break;
		case CloneSlot::State::Cooldown:
			view.state = CloneSlotView::State::Locked;
			view.cooldownRatio = std::clamp(slot.cooldown / duration, 0.0f, 1.0f);
			break;
		}
		views.push_back(view);
	}
	return views;
}
