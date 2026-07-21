#include "PlayerCloneUI.h"

#include <Engine/Scene/Utility/SceneUtility.h>

namespace {
	constexpr float kScreenW     = 1280.0f;
	constexpr float kCenterX     = kScreenW * 0.5f;
	constexpr float kPipSize     = 80.0f;  // 1個の一辺(px)
	constexpr float kPipGap      = 14.0f;  // 隙間(px)
	constexpr float kPipsBottomY = 640.0f; // 下端のY

	// 色
	constexpr float kGray[4] = { 0.28f, 0.28f, 0.30f, 1.0f }; // 灰（使用中）
	constexpr float kBlue[4] = { 0.25f, 0.55f, 1.00f, 1.0f }; // 使える
	constexpr float kRed[4]  = { 0.90f, 0.25f, 0.20f, 1.0f }; // クールタイム中

	// Material2D::fillMethod (0=none, 1=horizontal, 2=vertical, 3=mask)
	constexpr int kFillMethod = 1;
	// fillOrigin: 0=左/下, 1=右/上
	constexpr float kFillFromStart = 0.0f; // 下(縦) / 左(横)
	constexpr float kFillFromEnd   = 1.0f; // 上(縦) / 右(横)

	// index番目の中心X。全体を画面中央に揃える
	float PipCenterX(int index, int count) {
		const float rowWidth    = count * kPipSize + (count - 1) * kPipGap;
		const float firstCenter = kCenterX - rowWidth * 0.5f + kPipSize * 0.5f;
		return firstCenter + index * (kPipSize + kPipGap);
	}
}

void PlayerCloneUI::Initialize(int maxCount) {
	if (!fills_.empty()) {
		return; // 二重初期化を防ぐ
	}
	maxCount_ = maxCount < 0 ? 0 : maxCount;

	for (int i = 0; i < maxCount_; ++i) {
		const float cx = PipCenterX(i, maxCount_);

		// 状態色。サイズは常に原寸で、見せる範囲は塗りつぶし量で決める
		auto fill = SceneAPI::Instantiate<UiSprite>("Textures/Game/UI/tamashi_main.png");
		fill->SetAnchor({ 0.5f, 1.0f });
		fill->SetPositionPx(cx, kPipsBottomY);
		fill->SetSizePx(kPipSize, kPipSize);
		fill->SetOrderInLayer(1);
		fills_.push_back(fill);

		// 灰。状態色と同じ矩形に重ね、互いに補い合う範囲だけを描く
		auto top = SceneAPI::Instantiate<UiSprite>("Textures/Game/UI/tamashi_gray.png");
		top->SetAnchor({ 0.5f, 1.0f });
		top->SetPositionPx(cx, kPipsBottomY);
		top->SetSizePx(kPipSize, kPipSize);
		top->SetColorRGBA(kGray[0], kGray[1], kGray[2], kGray[3]);
		top->SetVisible(false);
		tops_.push_back(top);
	}
}

void PlayerCloneUI::Update(const std::vector<PlayerAbility::CloneSlotView>& slots) {
	for (int i = 0; i < maxCount_; ++i) {
		auto& fill = fills_[static_cast<size_t>(i)];
		auto& top  = tops_[static_cast<size_t>(i)];

		float        ratio     = 1.0f;   // 状態色の高さ割合
		const float* fillColor = kBlue;

		if (i < static_cast<int>(slots.size())) {
			const PlayerAbility::CloneSlotView& slot = slots[static_cast<size_t>(i)];
			switch (slot.state) {
			case PlayerAbility::CloneSlotView::State::Usable:
				fillColor = kBlue; ratio = 1.0f; break;
			case PlayerAbility::CloneSlotView::State::InUse:
				fillColor = kGray; ratio = 1.0f; break;
			case PlayerAbility::CloneSlotView::State::Locked:
				fillColor = kRed;  ratio = slot.cooldownRatio; break;
			}
		}

		// スケールを変えると絵が縦に潰れるので、原寸のまま塗りつぶし量で見せる範囲を変える
		fill->SetColorRGBA(fillColor[0], fillColor[1], fillColor[2], fillColor[3]);
		fill->SetFill(kFillMethod, kFillFromStart, kFillFromStart, ratio);
		fill->SetVisible(ratio > 0.001f);

		const float topRatio = 1.0f - ratio;
		top->SetFill(kFillMethod, kFillFromEnd, kFillFromEnd, topRatio);
		top->SetVisible(topRatio > 0.001f);
	}
}
