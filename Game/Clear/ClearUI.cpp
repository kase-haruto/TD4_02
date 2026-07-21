#include "ClearUI.h"

#include <Engine/Scene/Utility/SceneUtility.h>
#include <CalyxEngine/Project.h>

ClearUI::ClearUI()
	: BaseGameObject("debugCube.obj", "ClearUI") {}

void ClearUI::Initialize() {
	SetDrawEnable(false);

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