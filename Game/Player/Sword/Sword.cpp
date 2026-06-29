#include "Sword.h"

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

	switch (owner) {
	case Sword::HitboxOwner::Player:
		SetName("PlayerAttackHitbox");
		InitializeCollider(ColliderKind::Box);
		if (auto* collider = GetCollider()) {
			ColliderConfig config;
			config.isCollisionEnabled = true;
			config.isTrigger = true;
			config.isDraw = drawCollider;
			config.colliderType = static_cast<int>(ColliderType::Type_PlayerAttack);
			config.targetType = static_cast<int>(ColliderType::Type_Enemy);
			config.size = size;
			collider->ApplyConfig(config);
			collider->SetName("PlayerAttackHitboxCollider");
			if (auto* box = dynamic_cast<BoxCollider*>(collider)) {
				box->SetSize(size);
			}
		}
		break;
	case Sword::HitboxOwner::Enemy:
		SetName("EnemyAttackHitbox");
		InitializeCollider(ColliderKind::Box);
		if (auto* collider = GetCollider()) {
			ColliderConfig config;
			config.isCollisionEnabled = true;
			config.isTrigger = true;
			config.isDraw = drawCollider;
			config.colliderType = static_cast<int>(ColliderType::Type_EnemyAttack);
			config.targetType = static_cast<int>(ColliderType::Type_Player);
			config.size = size;
			collider->ApplyConfig(config);
			collider->SetName("EnemyAttackHitboxCollider");
			if (auto* box = dynamic_cast<BoxCollider*>(collider)) {
				box->SetSize(size);
			}
		}
		break;
	default:
		break;
	}
}
