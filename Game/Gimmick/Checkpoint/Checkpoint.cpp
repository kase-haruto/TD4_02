#include "Checkpoint.h"

#include <Game/Player/Player.h>
#include <Game/Collision/CollisionLayerUtil.h>
#include <Game/World/RespawnState.h>
#include <Engine/Objects/Collider/Collider.h>
#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Objects/Collider/BoxCollider.h>
#include <Engine/Scene/Context/SceneContext.h>

Checkpoint::Checkpoint()
	: BaseGameObject("debugCube.obj", "Checkpoint") {}

void Checkpoint::Initialize() {
	const float kActivateRadius = 2.5f;

	InitializeCollider(ColliderKind::Box);

	const auto layerId = GameCollision::FindLayerId("StageGimmick");

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

	RefreshVisual();
}

void Checkpoint::Update(float dt) {
	RefreshVisual();
}

void Checkpoint::OnCollisionEnter(Collider* other) {
	if (!other) return;
	auto* player = dynamic_cast<Player*>(other->GetOwner());
	if (!player) return;

	if (auto* ctx = SceneContext::Current()) {
		RespawnState::Get().SetCheckpoint(ctx->GetScenePath(), GetWorldPosition(), GetGuid());
	}
}

void Checkpoint::RefreshVisual() {
	auto& rs = RespawnState::Get();
	const bool active = rs.Has() && rs.ActivateGuid() == GetGuid();

	if (visualApplied_ && active == isActiveVisual_) return;  // 変化なし即return
	visualApplied_ = true;
	isActiveVisual_ = active;

	SetColor(active ? CalyxEngine::Vector4{ 0.4f, 1.0f, 0.4f, 1.0f } : CalyxEngine::Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });;
}
