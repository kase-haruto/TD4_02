#include "Clone.h"

#include <Game/Collision/CollisionLayerUtil.h>

#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Scene/Context/SceneContext.h>

PlayerClone::PlayerClone()
	: PlayerBase(PlayerModelSet::Spirit) {
	SetAppliesMovement(false);
}

void PlayerClone::Update(float dt) {
	if (isGhost_) {
		Actor::Update(dt);
		return;
	}

	PlayerBase::Update(dt);
}

void PlayerClone::OnCollisionEnter(Collider* other) {
	if (isGhost_) {
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
		ownerAbility_->OnCloneWallDeath();   // Abilityに消えたを伝える
	}
	if (auto* context = SceneContext::Current()) {
		context->RemoveObject(std::static_pointer_cast<SceneObject>(shared_from_this()));
	}
}

void PlayerClone::AddWorldOffset(const CalyxEngine::Vector3& delta) {
	if (isGhost_) {
		return;
	}
	GetWorldTransform().translation += delta;
}
