#include "MenuUI.h"

#include <Engine/Foundation/Input/Input.h>
#include <Engine/Scene/Utility/SceneUtility.h>

#include <algorithm>
#include <cmath>

using namespace CalyxFoundation;

namespace {
	constexpr float kStickThreshold = 0.5f; // スティックを倒したとみなす閾値
}

/////////////////////////////////////////////////////////////////////////////////////////
//                    初期化
/////////////////////////////////////////////////////////////////////////////////////////
void MenuUI::Initialize(float baseX, float topY, float lineGap) {
	baseX_ = baseX;
	topY_ = topY;
	lineGap_ = lineGap;
	selected_ = 0;
}

void MenuUI::AddItem(const ItemDesc& desc) {
	Item item{};
	item.onDecide = desc.onDecide;

	item.sprite = SceneAPI::Instantiate<UiSprite>(desc.texturePath);
	item.sprite->SetAnchor({ 0.0f, 0.5f });
	item.sprite->SetSizePx(desc.width, desc.height);
	item.sprite->SetPositionPx(baseX_, ItemCenterY(static_cast<int>(items_.size())));

	items_.push_back(item);
}

void MenuUI::SetSelector(const std::string& texturePath, float width, float height) {
	selector_ = SceneAPI::Instantiate<UiSprite>(texturePath);
	selector_->SetAnchor({ 1.0f, 0.5f });
	selector_->SetSizePx(width, height);
}

/////////////////////////////////////////////////////////////////////////////////////////
//                    更新
/////////////////////////////////////////////////////////////////////////////////////////
void MenuUI::Update(float dt) {
	if (items_.empty()) {
		return;
	}
	UpdateInput();
	UpdateVisual(dt);
}

void MenuUI::UpdateInput() {
	int dir = 0;

	if (Input::TriggerKey(DIK_W) || Input::TriggerKey(DIK_UP)
		|| Input::TriggerGamepadButton(PadButton::DPAD_UP)) {
		dir = -1;
	}
	if (Input::TriggerKey(DIK_S) || Input::TriggerKey(DIK_DOWN)
		|| Input::TriggerGamepadButton(PadButton::DPAD_DOWN)) {
		dir = 1;
	}

	// 左スティックは倒した瞬間だけ拾う
	const float stickY = Input::GetLeftStick().y;
	const bool  tilted = std::fabs(stickY) > kStickThreshold;
	if (tilted && !stickTilted_) {
		dir = (stickY > 0.0f) ? -1 : 1;
	}
	stickTilted_ = tilted;

	if (dir != 0) {
		const int count = static_cast<int>(items_.size());
		selected_ = (selected_ + dir + count) % count;   // 端で反対側へ回る
	}

	const bool decide = Input::TriggerKey(DIK_SPACE) || Input::TriggerKey(DIK_RETURN)
		|| Input::TriggerGamepadButton(PadButton::A);
	if (decide && items_[static_cast<size_t>(selected_)].onDecide) {
		items_[static_cast<size_t>(selected_)].onDecide();
	}
}

void MenuUI::UpdateVisual(float dt) {
	const float t = std::clamp(offsetLerpSpeed_ * dt, 0.0f, 1.0f);

	for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
		Item& item = items_[static_cast<size_t>(i)];

		const float target = (i == selected_) ? selectedOffsetX_ : 0.0f;
		item.offsetX += (target - item.offsetX) * t;

		item.sprite->SetPositionPx(baseX_ + item.offsetX, ItemCenterY(i));
	}

	if (selector_) {
		const Item& sel = items_[static_cast<size_t>(selected_)];
		selector_->SetPositionPx(baseX_ + sel.offsetX - selectorGap_, ItemCenterY(selected_));
	}
}

float MenuUI::ItemCenterY(int index) const {
	return topY_ + lineGap_ * static_cast<float>(index);
}