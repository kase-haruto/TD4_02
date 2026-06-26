#include "Sword.h"

#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Objects/Collider/BoxCollider.h>

Sword::Sword() :BaseGameObject("PlayerSword.obj") {}
Sword::Sword(const std::string& modelName, std::optional<std::string> objectName) :BaseGameObject(modelName, objectName) {

}
Sword::~Sword() = default;

void Sword::ConfigureAsAttackHitbox(
	const CalyxEngine::Vector3& size,
	bool drawCollider) {
	SetName("PlayerAttackHitbox");
	SetDrawEnable(false);
	SetEnablePicking(false);
	SetCastShadow(false);
	SetTransient(true);

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
}
