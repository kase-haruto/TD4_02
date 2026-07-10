#include "Bridge.h"

#include "InteractZone.h"

#include <Game/Player/Player.h>
#include <Engine/Objects/Collider/Collider.h>
#include <Game/Collision/CollisionLayerUtil.h>
#include <Data/Engine/Configs/Scene/Objects/Collider/ColliderConfig.h>
#include <Engine/Objects/Collider/BoxCollider.h>
#include <Engine/Physics/PhysicsBody.h>
#include <Engine/Foundation/Input/Input.h>
#include <Engine/Scene/Utility/SceneUtility.h>

namespace {
	constexpr float kDeg2Rad = 3.14159265f / 180.0f;
}

Bridge::Bridge()
	: BaseGameObject("debugCube.obj", "Bridge") {}

Bridge::~Bridge() = default;

void Bridge::Initialize() {
	InitializeCollider(ColliderKind::Box);
	const auto obstacle = GameCollision::FindLayerId("Obstacle");
	if (auto* collider = GetCollider()) {
		ColliderConfig config;
		config.isCollisionEnabled = true;
		config.isTrigger = false;
		config.isDraw = false;
		config.size = bodySize_;
		if (obstacle) {
			config.layerId = *obstacle;
		}
		collider->ApplyConfig(config);
		collider->SetName("BridgeBodyCollider");
		collider->SetOwner(this);
		if (auto* box = dynamic_cast<BoxCollider*>(collider)) {
			box->SetSize(bodySize_);
		}
	}

	// レバーを生成
	zone_ = SceneAPI::Instantiate<InteractZone>();
	zone_->Configure(zoneSize_, true);
	zone_->SetTranslate(leverOffset_);
	zone_->SetParent(shared_from_this());
	zone_->GetWorldTransform().inheritRotate = false;
	zone_->GetWorldTransform().inheritScale = false;

	// 初期は跳ね橋（直立90度）
	angleDeg_ = kRaisedAngleDeg;
	worldTransform_.rotationSource = RotationSource::Euler;
	SetRotate(CalyxEngine::Vector3{ angleDeg_ * kDeg2Rad, 0.0f, 0.0f });
}

void Bridge::Update(float dt) {
	UpdateDodgeSuppression();

	// 範囲内でAが押されたら起動
	Player* p = zone_ ? zone_->GetPlayer() : nullptr;
	if (p && !activated_ && p->IsDodgeButtonTriggered()) {
		activated_ = true;
	}

	// 起動後、水平へ倒れていく
	if (activated_ && angleDeg_ > 0.0f) {
		angleDeg_ -= kFallSpeedDeg * dt;
		if (angleDeg_ < 0.0f) {
			angleDeg_ = 0.0f;
		}
		SetRotate(CalyxEngine::Vector3{ angleDeg_ * kDeg2Rad, 0.0f, 0.0f });
	}
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