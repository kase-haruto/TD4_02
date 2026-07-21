#include "PlayerHPUI.h"

#include <algorithm>

namespace {
	// Material2D::fillMethod (0=none, 1=horizontal, 2=vertical, 3=mask)
	constexpr int kFillHorizontal = 1;
	// fillOrigin: (0,0)=左/下
	constexpr float kFillFromLeft = 0.0f;
}

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
	// 横に潰さず左から削れるように、原寸のまま塗りつぶし量で表現する
	worldTransform_.scale.x = fullWidth_;

	SpriteSceneObject2d::AlwaysUpdate(dt);

	if (!sprite_) {
		return;
	}
	SetFillMethod(kFillHorizontal);
	SetFillOrigin(kFillFromLeft, 0.0f);
	SetFillAmount(hpRatio_);
}

void PlayerHPUI::SetHp(int current, int max) {
	if (max <= 0) {
		hpRatio_ = 0.0f;
		return;
	}
	const float ratio = static_cast<float>(current) / static_cast<float>(max);
	hpRatio_ = std::clamp(ratio, 0.0f, 1.0f);
}
