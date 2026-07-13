#include "SpawnPoint.h"

#include "TransitionIntent.h"
#include <Game/Player/Player.h>
#include <Engine/Scene/Context/SceneContext.h>

SpawnPoint::SpawnPoint()
	: BaseGameObject("debugCube.obj", "SpawnPoint") {}

void SpawnPoint::Initialize() {
	param_.ownerGuid_ = GetGuid();
	param_.LoadParams();
}

void SpawnPoint::Update(float) {
	if (!firstReset_) return;
	firstReset_ = false;

	auto& intent = TransitionIntent::Get();
	if (!intent.HasPending() || intent.SpawnId() != param_.spawnId) return;

	if (auto* ctx = SceneContext::Current()) {
		if (auto player = ctx->FindFirst<Player>()) {
			player->SetPosition(worldTransform_.translation);   // 位置は出す
		}
	}
	intent.Clear();
}

void SpawnPoint::DerivativeGui() {
	param_.ShowGui();
}