#pragma once
#include <Engine/Objects/3D/Actor/BaseGameObject.h>
#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <Engine/Foundation/Utility/Guid/Guid.h>
#include <Engine/Foundation/Math/Vector3.h>
#include <cstdint>

class Collider;

CALYX_OBJECT(Category = GameObject, DisplayName = "Door", Icon = "UI/Tool/cube.dds")
class Door : public BaseGameObject {
public:
	Door();
	~Door() override = default;

	void Initialize() override;
	void Update(float dt) override;
	void OnCollisionEnter(Collider* other) override;
	void DerivativeGui() override;

private:
	void FirstReset();

	struct DoorData : CalyxEngine::SerializableObject {
		DoorData() {
			AddField("targetScene", targetSceneGuid).Tooltip("遷移先シーンのGUID");
			AddField("targetSpawnId", targetSpawnId).Tooltip("遷移先で出るSpawnPointのID");
			AddField("triggerSize", triggerSize).Tooltip("扉トリガーの大きさ");
		}

		Guid ownerGuid_;
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, ownerGuid_.ToString(), "Actor/World/Door" };
		}

		Guid    targetSceneGuid;
		int32_t targetSpawnId = 0;
		CalyxEngine::Vector3 triggerSize = { 1.0f, 2.0f, 1.0f };
	};
	DoorData param_;

	bool firstReset_ = true;
	bool requested_ = false;

	char targetSceneGuidBuf_[64] = {};
	bool guidBufInit_ = false;
};
