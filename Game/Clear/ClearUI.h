#pragma once

#include <Engine/Objects/3D/Actor/BaseGameObject.h>
#include <Player/UI/UiSprite.h>
#include <UI/MenuUI.h>
#include <UI/TimeDisplay.h>

#include <memory>
#include <vector>

CALYX_OBJECT(Category = GameObject, DisplayName = "ClearUI", Icon = "UI/Tool/cube.dds")
class ClearUI : public BaseGameObject {
public:
	ClearUI();
	~ClearUI() override = default;

	void Initialize() override;
	void Update(float dt) override;

private:
	void OnTitle();
	void OnExit();

	/// 画面右側にランキングと自分のタイムを並べる
	void BuildRecordPanel();

	std::shared_ptr<UiSprite> logo_;
	MenuUI                    menu_;

	float clearTime_ = -1.0f; //!< 今回のクリアタイム(秒)
	int   clearRank_ = -1;    //!< 今回の順位(0始まり)。-1 = 圏外

	std::shared_ptr<UiSprite> timeLogo_;                        //!< 「クリアタイム」の見出し
	std::vector<std::unique_ptr<TimeDisplay>> rankNumbers_;     //!< 各行の順位
	std::vector<std::unique_ptr<TimeDisplay>> rankTimes_;       //!< 各行のタイム
	TimeDisplay ownTime_;                                       //!< 今回のタイム
};