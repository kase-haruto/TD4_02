#include "Door.h"

#include "TransitionIntent.h"
#include <Game/Player/Player.h>
#include <Game/Collision/CollisionLayerUtil.h>
#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Objects/Collider/BoxCollider.h>
#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Scene/Utility/SceneUtility.h>
#include <externals/imgui/imgui.h>
#include <cstdio>

#include "SpawnPoint.h"
#include <Engine/Scene/Context/SceneContext.h>

Door::Door()
	: BaseGameObject("debugCube.obj", "Door") {}

void Door::Initialize() {
	param_.ownerGuid_ = GetGuid();
	param_.LoadParams();
}

void Door::Update(float) {
	if (firstReset_) {
		firstReset_ = false;
		FirstReset();
	}
}

void Door::FirstReset() {
	InitializeCollider(ColliderKind::Box);
	if (auto* collider = GetCollider()) {
		ColliderConfig config;
		config.isCollisionEnabled = true;
		config.isTrigger = true;
		config.isDraw = true;
		config.size = param_.triggerSize;
		if (auto layer = GameCollision::FindLayerId("StageGimmick")) {
			config.layerId = *layer;
		}
		collider->ApplyConfig(config);
		collider->SetName("DoorCollider");
		collider->SetOwner(this);
		if (auto* box = dynamic_cast<BoxCollider*>(collider)) {
			box->SetSize(param_.triggerSize);
		}
	}
}

void Door::OnCollisionEnter(Collider* other) {
	if (requested_ || !other) {
		return;
	}
	auto* player = dynamic_cast<Player*>(other->GetOwner());
	if (!player) {
		return;
	}

	TransitionIntent::Get().Request(param_.targetSpawnId);
	SceneAPI::RequestSceneChange(param_.targetSceneGuid);
	requested_ = true;
}

void Door::DerivativeGui() {
	param_.ShowGui();

	if (!guidBufInit_) {
		std::snprintf(targetSceneGuidBuf_, sizeof(targetSceneGuidBuf_), "%s",
			param_.targetSceneGuid.ToString().c_str()); guidBufInit_ = true;
	}
	ImGui::InputText("targetScene(GUID)", targetSceneGuidBuf_, sizeof(targetSceneGuidBuf_));
	if (ImGui::Button("Set Scene GUID")) { param_.targetSceneGuid = Guid::FromString(targetSceneGuidBuf_); }
}