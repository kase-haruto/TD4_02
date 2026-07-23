#pragma once

#include "UI/PlayerHPUI.h"
#include "UI/PlayerCloneUI.h"
#include "UI/UiSprite.h"
#include "UI/UiSprite.h"

#include <Game/Player/Ability/PlayerAbility.h> // CloneSlotView

#include <memory>
#include <vector>


class PlayerUI {
public:
	PlayerUI();
	~PlayerUI();

	void Initialize(int maxCloneCount);
	void Update(int currentHp, int maxHp,
		const std::vector<PlayerAbility::CloneSlotView>& cloneSlots);

private:

	std::shared_ptr<PlayerHPUI> hp_;
	std::shared_ptr<UiSprite>   hpFrame_;
	std::shared_ptr<UiSprite>   controlUI_;
	PlayerCloneUI               clones_;
};
