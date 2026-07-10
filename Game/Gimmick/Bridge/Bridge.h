#pragma once

#include <Engine/Objects/3D/Actor/BaseGameObject.h>
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

private:
    void UpdateDodgeSuppression();

    std::shared_ptr<InteractZone> zone_;
    Player* suppressedPlayer_ = nullptr;

    bool  activated_ = false;
    float angleDeg_ = 90.0f;

    static constexpr float kRaisedAngleDeg = 90.0f;   // 起動前の角度
    static constexpr float kFallSpeedDeg = 120.0f;     // 1秒あたり何度倒れるか

    CalyxEngine::Vector3 leverOffset_{ 2.0f, 0.0f, 2.0f };  // レバー範囲の位置(橋からのオフセット)
    CalyxEngine::Vector3 zoneSize_{ 2.5f, 2.5f, 2.5f };     // レバー範囲の大きさ
    CalyxEngine::Vector3 bodySize_{ 4.0f, 1.0f, 10.0f };    // 橋板の当たり(倒れた時の床)
};