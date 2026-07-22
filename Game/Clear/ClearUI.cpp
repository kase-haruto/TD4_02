#include "ClearUI.h"

#include <Engine/Scene/Utility/SceneUtility.h>
#include <CalyxEngine/Project.h>

#include <Game/World/WorldState.h>
#include <Game/World/ClearTimeRecords.h>

namespace {
	// 画面右半分(1280x720想定)。左半分はロゴとメニュー
	constexpr float kPanelX = 700.0f;      //!< 右パネルの左端X
	constexpr float kPanelLogoY = 130.0f;  //!< 「クリアタイム」見出しの中心Y
	constexpr float kPanelLogoW = 320.0f;
	constexpr float kPanelLogoH = 90.0f;

	constexpr float kFirstRowY = 250.0f;   //!< 1位の行の中心Y
	constexpr float kRowGap = 62.0f;       //!< 行の間隔
	constexpr float kRankX = kPanelX;      //!< 順位の数字の左端
	constexpr float kTimeX = kPanelX + 70.0f; //!< タイムの左端
	constexpr float kOwnRowGap = 44.0f;    //!< ランキングと自分の記録の間隔

	// 色。ランクインした行と自分の記録は黄色で目立たせる
	constexpr float kRowColor[4] = { 0.88f, 0.90f, 0.95f, 1.0f };
	constexpr float kOwnColor[4] = { 1.00f, 0.82f, 0.15f, 1.0f };
}

ClearUI::ClearUI()
	: BaseGameObject("debugCube.obj", "ClearUI") {}

void ClearUI::Initialize() {
	SetDrawEnable(false);

	// クリア画面に来た時点で計測終了
	WorldState::Get().StopPlayTime();
	if (WorldState::Get().TryMarkCleared()) {
		clearTime_ = WorldState::Get().PlayTime();
		clearRank_ = ClearTimeRecords::Get().Submit(clearTime_);
	} else {
		clearTime_ = ClearTimeRecords::Get().LastTime();
		clearRank_ = ClearTimeRecords::Get().LastRank();
	}

	logo_ = SceneAPI::Instantiate<UiSprite>("Textures/Game/Text/clear.png");
	logo_->SetAnchor({ 0.0f, 0.5f });
	logo_->SetPositionPx(MenuLayout::kBaseX, MenuLayout::kLogoY);
	logo_->SetSizePx(MenuLayout::kLogoW, MenuLayout::kLogoH);

	menu_.Initialize(MenuLayout::kBaseX, MenuLayout::kFirstItemY, MenuLayout::kLineGap);
	menu_.SetSelector("Textures/Game/UI/select.png", MenuLayout::kSelectorW, MenuLayout::kSelectorH);

	MenuUI::ItemDesc toTitle{};
	toTitle.width = MenuLayout::kItemW;
	toTitle.height = MenuLayout::kItemH;
	toTitle.texturePath = "Textures/Game/Text/titleback.png";
	toTitle.onDecide = [this] { OnTitle(); };
	menu_.AddItem(toTitle);

	MenuUI::ItemDesc exit{};
	exit.width = MenuLayout::kItemW;
	exit.height = MenuLayout::kItemH;
	exit.texturePath = "Textures/Game/Text/owaru.png";
	exit.onDecide = [this] { OnExit(); };
	menu_.AddItem(exit);

	BuildRecordPanel();
}

void ClearUI::BuildRecordPanel() {
	// 見出し
	timeLogo_ = SceneAPI::Instantiate<UiSprite>("Textures/Game/Text/clearTime.png");
	timeLogo_->SetAnchor({ 0.0f, 0.5f });
	timeLogo_->SetPositionPx(kPanelX, kPanelLogoY);
	timeLogo_->SetSizePx(kPanelLogoW, kPanelLogoH);

	// 上位5件。ランクインした行だけ色を変える
	const std::vector<float>& times = ClearTimeRecords::Get().Times();
	for (size_t i = 0; i < times.size(); ++i) {
		const float y = kFirstRowY + static_cast<float>(i) * kRowGap;
		const bool  isOwn = (static_cast<int>(i) == clearRank_);
		const float* color = isOwn ? kOwnColor : kRowColor;

		auto rank = std::make_unique<TimeDisplay>();
		rank->BuildNumber(static_cast<int>(i) + 1, 1, kRankX, y);
		rank->SetColorRGBA(color[0], color[1], color[2], color[3]);

		auto time = std::make_unique<TimeDisplay>();
		time->BuildTime(times[i], kTimeX, y);
		time->SetColorRGBA(color[0], color[1], color[2], color[3]);

		rankNumbers_.push_back(std::move(rank));
		rankTimes_.push_back(std::move(time));
	}

	// 自分の記録は圏外でも出す
	if (clearTime_ >= 0.0f) {
		const float y = kFirstRowY
			+ static_cast<float>(ClearTimeRecords::kMaxRecords) * kRowGap
			+ kOwnRowGap;
		ownTime_.BuildTime(clearTime_, kTimeX, y);
		ownTime_.SetColorRGBA(kOwnColor[0], kOwnColor[1], kOwnColor[2], kOwnColor[3]);
	}
}

void ClearUI::Update(float dt) {
	menu_.Update(dt);
}

void ClearUI::OnTitle() {
	SceneAPI::RequestSceneChange(Calyx::ResolveAssetPath("Scenes/TitleScene.scene"));
}

void ClearUI::OnExit() {
	// アプリ終了処理
}