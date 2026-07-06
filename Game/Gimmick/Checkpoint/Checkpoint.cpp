#include "Checkpoint.h"

#include <Game/Player/Player.h>
#include <Engine/Objects/Collider/Collider.h>
#include <Game/Collision/CollisionLayerUtil.h>
#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Objects/Collider/BoxCollider.h>

Checkpoint::Checkpoint()
	: BaseGameObject("debugCube.obj", "Checkpoint") {}

void Checkpoint::Initialize() {
	const float kActivateRadius = 2.5f;

	InitializeCollider(ColliderKind::Box);

	const auto layerId = GameCollision::FindLayerId("Default");

	if (auto* collider = GetCollider()) {
		ColliderConfig config;
		config.isCollisionEnabled = true;
		config.isTrigger = true;
		config.isDraw = true;
		config.radius = kActivateRadius;
		if (layerId) {
			config.layerId = *layerId;
		}
		collider->ApplyConfig(config);
		collider->SetName("CheckpointCollider");
		collider->SetOwner(this);
		if (auto* sphere = dynamic_cast<BoxCollider*>(collider)) {
			sphere->SetSize({ kActivateRadius ,kActivateRadius, kActivateRadius });
		}
	}
}

void Checkpoint::OnCollisionEnter(Collider* other) {
	if (!other) {
		return;
	}
	// 触れた相手が Player か
	auto* player = dynamic_cast<Player*>(other->GetOwner());
	if (!player) {
		return;
	}

	// この位置をチェックポイントとして登録
	player->SetRespawnPoint(GetWorldPosition());

	if (!activated_) {
		activated_ = true;
		SetColor({ 0.4f, 1.0f, 0.4f, 1.0f });
	}
}