#pragma once

#include <Engine/Objects/3D/Actor/BaseGameObject.h>
#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <cstdint>
#include <memory>

class Player;
class InteractZone;

/*-----------------------------------------------------------------------------------------
 * Bridge
 * - プレイヤーが渡る橋
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Bridge", Icon = "UI/Tool/cube.dds")
class Bridge : public BaseGameObject {
public:
	Bridge();
	~Bridge() override;

	void Initialize() override;
	void Update(float dt) override;
	void DerivativeGui() override;

private:
	void FirstReset();
	void UpdateDodgeSuppression();
	void ApplyBridgeRotation();
	void ApplyZoneParams();
	void ResetBridgePose();

private:

	struct BridgeData : CalyxEngine::SerializableObject {
		BridgeData() {
			AddField("raisedAngleDeg", raisedAngleDeg)
				.Category("Motion").Tooltip("起動前の角度");
			AddField("loweredAngleDeg", loweredAngleDeg)
				.Category("Motion").Tooltip("倒れ切った目標角度");
			AddField("fallSpeedDeg", fallSpeedDeg)
				.Category("Motion").Tooltip("1秒あたり何度倒れるか");
			AddField("fallAxis", fallAxis)
				.Category("Motion").Tooltip("どの軸まわりに倒れるか。符号で向きも決まる（例 {1,0,0}=前, {-1,0,0}=後ろ）");

			AddField("leverOffset", leverOffset)
				.Category("Zone").Tooltip("レバー範囲の位置");
			AddField("zoneSize", zoneSize)
				.Category("Zone").Tooltip("レバー範囲の大きさ");
			AddField("bodySize", bodySize)
				.Category("Body").Tooltip("橋板の当たり");
		}

		Guid ownerGuid_;
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, ownerGuid_.ToString(), "Actor/Gimmick/Bridge" };
		}

		float raisedAngleDeg = 90.0f;   // 起動前の角度
		float loweredAngleDeg = 0.0f;    // 目標角度（倒れ切った角度）
		float fallSpeedDeg = 120.0f;  // 倒れる速度

		CalyxEngine::Vector3 fallAxis = { 1.0f, 0.0f, 0.0f };  // 倒れる軸、向き

		CalyxEngine::Vector3 leverOffset = { 2.0f, 0.0f, 2.0f };
		CalyxEngine::Vector3 zoneSize = { 2.5f, 2.5f, 2.5f };
		CalyxEngine::Vector3 bodySize = { 4.0f, 1.0f, 10.0f };
	};

	BridgeData param_;

	std::shared_ptr<InteractZone> zone_;
	Player* suppressedPlayer_ = nullptr;

	bool  activated_ = false;
	float angleDeg_ = 90.0f;

	bool firstReset_ = true;
};