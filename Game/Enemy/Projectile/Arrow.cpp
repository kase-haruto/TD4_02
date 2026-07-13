#include "Arrow.h"

#include <Game/Collision/CollisionLayerUtil.h>

#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Scene/Context/SceneContext.h>

Arrow::Arrow()
	: Sword("Arrow.obj") {}

Arrow::Arrow(const std::string& modelName, std::optional<std::string> objectName)
	: Sword(modelName, objectName) {}

void Arrow::Launch(const CalyxEngine::Vector3& velocity, float lifeTime) {
	velocity_ = velocity;
	lifeTimer_ = lifeTime;
}

void Arrow::Update(float dt) {
	if (pendingRemove_) {
		RemoveSelf();
		return;
	}

	// 移動
	GetWorldTransform().translation =
		GetWorldTransform().translation + velocity_ * dt;

	// 寿命
	lifeTimer_ -= dt;
	if (lifeTimer_ <= 0.0f) {
		RemoveSelf();
	}
}

void Arrow::OnCollisionEnter(Collider* other) {
	if (!other) {
		return;
	}

	// プレイヤー or 障害物に当たったら消す
	const auto playerLayer = GameCollision::FindLayerId("Player");
	const auto obstacleLayer = GameCollision::FindLayerId("Obstacle");
	const auto layer = other->GetLayerId();

	if ((playerLayer && layer == *playerLayer) ||
		(obstacleLayer && layer == *obstacleLayer)) {
		pendingRemove_ = true;   // 次の Update で消す
	}
}

void Arrow::RemoveSelf() {
	if (auto* context = SceneContext::Current()) {
		context->RemoveObject(
			std::static_pointer_cast<SceneObject>(shared_from_this()));
	}
}
