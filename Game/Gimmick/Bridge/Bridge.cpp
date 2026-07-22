#include "Bridge.h"

#include "InteractZone.h"

#include <Game/Player/Player.h>
#include <Game/Collision/CollisionLayerUtil.h>
#include <Game/World/WorldState.h>

#include <Engine/Objects/Collider/Collider.h>
#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Objects/Collider/BoxCollider.h>
#include <Engine/Physics/PhysicsBody.h>
#include <Engine/Foundation/Input/Input.h>
#include <Engine/Scene/Utility/SceneUtility.h>

namespace {
	constexpr float kDeg2Rad = 3.14159265f / 180.0f;
}

Bridge::Bridge()
	: BaseGameObject("Darwbridge.obj", "Bridge") {}

Bridge::~Bridge() = default;

void Bridge::Initialize() {
	param_.ownerGuid_ = GetGuid();
	param_.LoadParams();
	firstReset_ = true;
	angleDeg_ = param_.raisedAngleDeg;
	worldTransform_.rotationSource = RotationSource::Euler;
	ApplyBridgeRotation();

	// レバーを生成
	zone_ = SceneAPI::Instantiate<InteractZone>();
	zone_->Configure(param_.zoneSize, true);
	zone_->SetParent(shared_from_this());
	zone_->GetWorldTransform().inheritRotate = false;
	zone_->GetWorldTransform().inheritScale = false;
	zone_->SetTranslate(param_.leverOffset);
}

void Bridge::Update(float dt) {
	if (firstReset_) {
		FirstReset();   // 初期化をここでやる
		firstReset_ = false;
	}

	UpdateDodgeSuppression();

	// 範囲内でAが押されたら起動
	Player* p = zone_ ? zone_->GetPlayer() : nullptr;
	if (p && !activated_ && p->IsDodgeButtonTriggered()) {
		activated_ = true;
		WorldState::Get().SetActivated(GetGuid());
	}

	// 起動後、水平へ倒れていく
	if (activated_ && angleDeg_ > param_.loweredAngleDeg) {
		angleDeg_ -= param_.fallSpeedDeg * dt;
		if (angleDeg_ < param_.loweredAngleDeg) {
			angleDeg_ = param_.loweredAngleDeg;
		}
		ApplyBridgeRotation();
	}
}

void Bridge::DerivativeGui() {
	param_.ShowGui();
	if (ImGui::Button("Apply to Zone")) {
		ApplyZoneParams();
	}
	if (ImGui::Button("Reset Bridge Pose")) {
		ResetBridgePose();
	}
}

void Bridge::FirstReset() {

	InitializeCollider(ColliderKind::Box);
	const auto obstacle = GameCollision::FindLayerId("Obstacle");
	if (auto* collider = GetCollider()) {
		ColliderConfig config;
		config.isCollisionEnabled = true;
		config.isTrigger = false;
		config.isDraw = false;
		config.size = param_.bodySize;
		if (obstacle) {
			config.layerId = *obstacle;
		}
		collider->ApplyConfig(config);
		collider->SetName("BridgeBodyCollider");
		collider->SetOwner(this);
		if (auto* box = dynamic_cast<BoxCollider*>(collider)) {
			box->SetSize(param_.bodySize);
		}
	}

	// 初期は跳ね橋
	angleDeg_ = param_.raisedAngleDeg;
	worldTransform_.rotationSource = RotationSource::Euler;
	if (WorldState::Get().IsActivated(GetGuid())) {
		activated_ = true;
		angleDeg_ = param_.loweredAngleDeg;
	}
	ApplyBridgeRotation();
}

void Bridge::UpdateDodgeSuppression() {
	Player* p = zone_ ? zone_->GetPlayer() : nullptr;

	if (p && p != suppressedPlayer_) {
		p->SetDodgeEnabled(false);              // 範囲に入ったら回避を封じる
		suppressedPlayer_ = p;
	} else if (!p && suppressedPlayer_) {
		suppressedPlayer_->SetDodgeEnabled(true);
		suppressedPlayer_ = nullptr;
	}
}

void Bridge::ApplyBridgeRotation() {
	SetRotate(param_.fallAxis * (angleDeg_ * kDeg2Rad));
	if (param_.fallAxis.z == 1.0f) {
		
	}
}

void Bridge::ApplyZoneParams() {
	if (!zone_) {
		return;
	}
	if (auto* box = dynamic_cast<BoxCollider*>(zone_->GetCollider())) {
		box->SetSize(param_.zoneSize);
	}
	zone_->SetTranslate(param_.leverOffset);
}

void Bridge::ResetBridgePose() {
	activated_ = false;
	angleDeg_ = param_.raisedAngleDeg;
	ApplyBridgeRotation();
}
