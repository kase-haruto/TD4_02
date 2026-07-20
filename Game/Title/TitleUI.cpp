#include "TitleUI.h"

#include <Engine/Scene/Utility/SceneUtility.h>
#include <CalyxEngine/Project.h>


TitleUI::TitleUI()
	: BaseGameObject("debugCube.obj", "TitleUI") {}

void TitleUI::Initialize() {
	SetDrawEnable(false);

	// タイトルロゴ
	logo_ = SceneAPI::Instantiate<UiSprite>();
	logo_->SetAnchor({ 0.0f, 0.5f });
	logo_->SetPositionPx(MenuLayout::kBaseX, MenuLayout::kLogoY);
	logo_->SetSizePx(MenuLayout::kLogoW, MenuLayout::kLogoH);

	// メニュー
	menu_.Initialize(MenuLayout::kBaseX, MenuLayout::kFirstItemY, MenuLayout::kLineGap);
	menu_.SetSelector("", MenuLayout::kSelectorW, MenuLayout::kSelectorH);

	MenuUI::ItemDesc start{};
	start.width = MenuLayout::kItemW;
	start.height = MenuLayout::kItemH;
	start.onDecide = [this] { OnStart(); };
	menu_.AddItem(start);

	MenuUI::ItemDesc exit{};
	exit.width = MenuLayout::kItemW;
	exit.height = MenuLayout::kItemH;
	exit.onDecide = [this] { OnExit(); };
	menu_.AddItem(exit);
}

void TitleUI::Update(float dt) {
	menu_.Update(dt);
}

void TitleUI::OnStart() {
	SceneAPI::RequestSceneChange(Calyx::ResolveAssetPath("Scenes/Area001.scene"));
}

void TitleUI::OnExit() {
	// アプリ終了処理
}