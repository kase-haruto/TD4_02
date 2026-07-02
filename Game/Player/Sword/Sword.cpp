#include "Sword.h"

#include <Game/Collision/CollisionLayerUtil.h>

#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Objects/Collider/BoxCollider.h>

Sword::Sword() :BaseGameObject("PlayerSword.obj") {}
Sword::Sword(const std::string& modelName, std::optional<std::string> objectName) :BaseGameObject(modelName, objectName) {

}
Sword::~Sword() = default;

void Sword::ConfigureAsAttackHitbox(
	const CalyxEngine::Vector3& size,
	bool drawCollider,
	HitboxOwner owner) {

	SetDrawEnable(false);
	SetEnablePicking(false);
	SetCastShadow(false);
	SetTransient(true);

	const bool isPlayer = (owner == HitboxOwner::Player);
	const char* objectName = isPlayer ? "PlayerAttackHitbox" : "EnemyAttackHitbox";
	const char* colliderName = isPlayer ? "PlayerAttackHitboxCollider" : "EnemyAttackHitboxCollider";
	const char* layerName = isPlayer ? "PlayerAttack" : "EnemyAttack";
	const auto layerId = GameCollision::FindLayerId(layerName);

	SetName(objectName);
	InitializeCollider(ColliderKind::Box);
	if (auto* collider = GetCollider()) {
		ColliderConfig config;
		config.isCollisionEnabled = true;
		config.isTrigger = true;
		config.isDraw = drawCollider;
		config.size = size;
		if (layerId) {
			config.layerId = *layerId;
		}
		collider->ApplyConfig(config);
		collider->SetName(colliderName);
		if (auto* box = dynamic_cast<BoxCollider*>(collider)) {
			box->SetSize(size);
		}
	}
}
