#include "PlayerHPUI.h"

#include <algorithm>

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor / dtor
/////////////////////////////////////////////////////////////////////////////////////////
PlayerHPUI::PlayerHPUI(const std::string& texturePath) {
	if (!texturePath.empty()) {
		texturePath_ = texturePath; // sprite_ が作られる前に入れておく必要がある
	}

	worldTransform_.translation = { HpBarLayout::kLeftX, HpBarLayout::kCenterY, 0.0f }; // 画面中央下（左端基準で中央寄せ）
	worldTransform_.scale       = { fullWidth_, height_, 1.0f };

	color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // 色は乗算なので、絵をそのまま出すなら白
	SetAnchor({ 0.0f, 0.5f });
}

/////////////////////////////////////////////////////////////////////////////////////////
//			更新
/////////////////////////////////////////////////////////////////////////////////////////
void PlayerHPUI::AlwaysUpdate(float dt) {
	worldTransform_.scale.x = fullWidth_ * hpRatio_;
	SpriteSceneObject2d::AlwaysUpdate(dt);
}

void PlayerHPUI::SetHp(int current, int max) {
	if (max <= 0) {
		hpRatio_ = 0.0f;
		return;
	}
	const float ratio = static_cast<float>(current) / static_cast<float>(max);
	hpRatio_ = std::clamp(ratio, 0.0f, 1.0f);
}
