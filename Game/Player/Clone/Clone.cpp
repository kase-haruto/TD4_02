#include "Clone.h"

#include <Game/Collision/CollisionLayerUtil.h>
#include <Game/World/KillPlane.h>

#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Graphics/Pipeline/BlendMode/BlendMode.h>
#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Scene/Context/SceneContext.h>

#include <algorithm>
#include <cmath>

namespace {
	constexpr float kVanishDuration = 0.25f;    //!< 消えきるまでの秒数
	constexpr float kVanishExpandScale = 1.4f;  //!< 最終的な膨張倍率
}

PlayerClone::PlayerClone()
	: PlayerBase(PlayerModelSet::Spirit) {
	SetAppliesMovement(false);
	deleteEf_.Load("CloneDelete");
}

void PlayerClone::Update(float dt) {
	if (isGhost_) {
		Actor::Update(dt);
		return;
	}

	// 消滅演出中は通常の更新をしない
	if (isVanishing_) {
		UpdateVanish(dt);
		return;
	}

	// 落下死。ステージ外に落ちたら消す。
	if (KillPlane::IsFallenOut(GetWorldPosition())) {
		if (auto* context = SceneContext::Current()) {
			context->RemoveObject(std::static_pointer_cast<SceneObject>(shared_from_this()));
		}
		return;
	}

	PlayerBase::Update(dt);
}

void PlayerClone::OnCollisionEnter(Collider* other) {
	if (isGhost_ || isVanishing_) {
		return;
	}

	if (!other) {
		return;
	}

	const auto obstacleLayer = GameCollision::FindLayerId("Obstacle");
	if (!obstacleLayer || other->GetLayerId() != *obstacleLayer) {
		return;
	}

	if (ownerAbility_) {
		ownerAbility_->OnCloneWallDeath(this);   // どのスロットが消えたかAbilityへ伝える
	}
	StartVanish();   // 即消しではなく、膨張しながらフェードで消す
}

void PlayerClone::StartVanish() {
	if (isVanishing_) {
		return;
	}
	isVanishing_ = true;
	vanishTime_ = 0.0f;
	vanishBaseScale_ = GetWorldTransform().scale;

	// 半透明で描けるようにする
	SetBlendMode(BlendMode::ALPHA);
	SetCastShadow(false);

	if (auto* collider = GetCollider()) {
		ColliderConfig config = collider->ExtractConfig();
		config.isCollisionEnabled = false;
		config.isDraw = false;
		collider->ApplyConfig(config);
	}
}

void PlayerClone::UpdateVanish(float dt) {
	vanishTime_ += dt;
	const float t = std::clamp(vanishTime_ / kVanishDuration, 0.0f, 1.0f);

	// 膨張
	const float ease = 1.0f - std::pow(1.0f - t, 3.0f);
	const float scaleRate = 1.0f + (kVanishExpandScale - 1.0f) * ease;
	SetScale(vanishBaseScale_ * scaleRate);

	// 透明化
	const float alpha = (1.0f - t) * (1.0f - t);
	SetColor({1.0f, 1.0f, 1.0f, alpha});

	Actor::Update(dt);

	if (t >= 1.0f) {
		if (!deleteEf_.GetData().emitters.empty()) {
			EffectAPI::Play(deleteEf_, GetWorldPosition());
		}
		if (auto* context = SceneContext::Current()) {
			context->RemoveObject(std::static_pointer_cast<SceneObject>(shared_from_this()));
		}
	}
}

void PlayerClone::AddWorldOffset(const CalyxEngine::Vector3& delta) {
	if (isGhost_) {
		return;
	}
	GetWorldTransform().translation += delta;
}
