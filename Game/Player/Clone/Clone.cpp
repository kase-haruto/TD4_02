#include "Clone.h"

#include <Game/Collision/CollisionLayerUtil.h>

#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Scene/Context/SceneContext.h>

PlayerClone::PlayerClone() = default;

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

	if (auto* context = SceneContext::Current()) {
		context->RemoveObject(std::static_pointer_cast<SceneObject>(shared_from_this()));
	}
}
