#pragma once

#include "UiSprite.h"

#include <Game/Player/Ability/PlayerAbility.h> // CloneSlotView

#include <memory>
#include <vector>

/*-----------------------------------------------------------------------------------------
 * PlayerCloneUI
 * - 分身の使用状況を表示する
 * - 2枚のスプライトで構成
 *---------------------------------------------------------------------------------------*/
class PlayerCloneUI {
public:
	void Initialize(int maxCount);
	void Update(const std::vector<PlayerAbility::CloneSlotView>& slots);

private:
	std::vector<std::shared_ptr<UiSprite>> fills_; // 状態色
	std::vector<std::shared_ptr<UiSprite>> tops_;  // 灰
	int maxCount_ = 0;
};
