#pragma once

#include <Engine/Objects/2D/Object2d/SpriteSceneObject2d.h>

#include <string>

/*-----------------------------------------------------------------------------------------
 * HpBarLayout
 * - HPバー本体と背景で共有するレイアウト
 *---------------------------------------------------------------------------------------*/
namespace HpBarLayout {
	constexpr float kWidth   = 400.0f;                 //!< 満タン時の横幅(px)
	constexpr float kHeight  = 225.0f;                  //!< 高さ(px)
	constexpr float kLeftX   = 640.0f - kWidth * 0.5f; //!< 左端X(画面中央寄せ)
	constexpr float kCenterY = 665.0f;                 //!< 中心Y
}

/*-----------------------------------------------------------------------------------------
 * PlayerHPUI
 * - プレイヤーのHPゲージ
 *---------------------------------------------------------------------------------------*/
class PlayerHPUI
	: public CalyxEngine::SpriteSceneObject2d {
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	/// 空文字を渡すと白1x1のまま
	explicit PlayerHPUI(const std::string& texturePath = std::string{});
	~PlayerHPUI() override = default;

	void AlwaysUpdate(float dt) override;

	/// 現在HP/最大HP を渡すと 0..1 の割合として反映する
	void SetHp(int current, int max);

	float GetFullWidth() const { return fullWidth_; }
	float GetHeight() const { return height_; }

private:
	//===================================================================*/
	//						private variables
	//===================================================================*/
	float hpRatio_   = 1.0f;                 // 0..1
	float fullWidth_ = HpBarLayout::kWidth;  // 満タン時の横幅(px)
	float height_    = HpBarLayout::kHeight; // 高さ(px)
};
