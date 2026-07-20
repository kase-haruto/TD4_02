#include "PlayerUI.h"

#include <Engine/Scene/Utility/SceneUtility.h>


PlayerUI::PlayerUI() {}
PlayerUI::~PlayerUI() {}

void PlayerUI::Initialize(int maxCloneCount) {
	// 背景(HPバーと同じ矩形に置く)
	if (!hpFrame_) {
		hpFrame_ = SceneAPI::Instantiate<UiSprite>("Textures/Game/UI/HP_back.png");
		hpFrame_->SetAnchor({ 0.0f, 0.5f });
		hpFrame_->SetPositionPx(HpBarLayout::kLeftX + 10.0f, HpBarLayout::kCenterY);
		hpFrame_->SetSizePx(HpBarLayout::kWidth, HpBarLayout::kHeight);
	}
	// HPバー本体
	if (!hp_) {
		hp_ = SceneAPI::Instantiate<PlayerHPUI>("Textures/Game/UI/HP.png");
	}
	clones_.Initialize(maxCloneCount);
}

void PlayerUI::Update(int currentHp, int maxHp,
	const std::vector<PlayerAbility::CloneSlotView>& cloneSlots) {
	if (hp_) {
		hp_->SetHp(currentHp, maxHp);
	}
	clones_.Update(cloneSlots);
}
