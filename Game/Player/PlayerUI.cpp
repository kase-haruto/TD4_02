#include "PlayerUI.h"

#include <Engine/Scene/Utility/SceneUtility.h>


PlayerUI::PlayerUI() {}
PlayerUI::~PlayerUI() {}

void PlayerUI::Initialize(int maxCloneCount) {
	if (!hp_) {
		hp_ = SceneAPI::Instantiate<PlayerHPUI>();
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
