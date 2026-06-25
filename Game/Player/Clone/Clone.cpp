#include "Clone.h"

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

	const BaseGameObject* owner = other->GetOwner();
	const std::string& objectName = owner ? owner->GetName() : other->GetName();
	const bool isObstacle =
		other->GetType() == ColliderType::Type_Impediment ||
		other->GetType() == ColliderType::Type_StageGimmick ||
		objectName.find("wall") != std::string::npos ||
		objectName.find("Wall") != std::string::npos;
	if (!isObstacle) {
		return;
	}

	if (auto* context = SceneContext::Current()) {
		context->RemoveObject(std::static_pointer_cast<SceneObject>(shared_from_this()));
	}
}
