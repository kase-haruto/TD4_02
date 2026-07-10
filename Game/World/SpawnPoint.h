#pragma once
#include <Engine/Objects/3D/Actor/BaseGameObject.h>
#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <cstdint>

CALYX_OBJECT(Category = GameObject, DisplayName = "SpawnPoint", Icon = "UI/Tool/cube.dds")
class SpawnPoint : public BaseGameObject {
public:
	SpawnPoint();
	~SpawnPoint() override = default;

	void Initialize() override;
	void Update(float dt) override;
	void DerivativeGui() override;

	int32_t GetSpawnId() const { return param_.spawnId; }

private:
	struct SpawnData : CalyxEngine::SerializableObject {
		SpawnData() {
			AddField("spawnId", spawnId).Tooltip("この出現位置のID。DoorのtargetSpawnIdと一致で使われる");
		}

		Guid ownerGuid_;
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, ownerGuid_.ToString(), "Actor/World/SpawnPoint" };
		}
		int32_t spawnId = 0;
	};

	SpawnData param_;
	bool firstReset_ = true;
};
