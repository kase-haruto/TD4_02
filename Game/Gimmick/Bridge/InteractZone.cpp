#include "InteractZone.h"

#include <Game/Player/Player.h>
#include <Game/Collision/CollisionLayerUtil.h>
#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Objects/Collider/BoxCollider.h>

InteractZone::InteractZone()
	: BaseGameObject("debugCube.obj", "InteractZone") {}

void InteractZone::Configure(const CalyxEngine::Vector3& size, bool draw) {
	SetDrawEnable(draw);
	SetEnablePicking(true);
	SetCastShadow(true);
	//SetTransient(true);

	InitializeCollider(ColliderKind::Box);
	const auto layerId = GameCollision::FindLayerId("StageGimmick");
	if (auto* collider = GetCollider()) {
		ColliderConfig config;
		config.isCollisionEnabled = true;
		config.isTrigger = true;
		config.isDraw = draw;
		config.size = size;
		if (layerId) {
			config.layerId = *layerId;
		}
		collider->ApplyConfig(config);
		collider->SetName("InteractZoneCollider");
		collider->SetOwner(this);
		if (auto* box = dynamic_cast<BoxCollider*>(collider)) {
			box->SetSize(size);
		}
	}
}

void InteractZone::OnCollisionEnter(Collider* other) {
	if (!other) {
		return;
	}
	if (auto* p = dynamic_cast<Player*>(other->GetOwner())) {
		player_ = p;
	}
}

void InteractZone::OnCollisionExit(Collider* other) {
	if (!other) {
		return;
	}
	auto* p = dynamic_cast<Player*>(other->GetOwner());
	if (p && p == player_) {
		player_ = nullptr;
	}
}