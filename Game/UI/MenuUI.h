#pragma once

#include <Player/UI/UiSprite.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

/*-----------------------------------------------------------------------------------------
 * MenuUI
 * - 選択メニュー(Title/Clear共通)
 *---------------------------------------------------------------------------------------*/
class MenuUI {
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	/// 項目を追加するときに渡す情報
	struct ItemDesc {
		std::string           texturePath;          //!< 項目の画像(空なら白のまま)
		float                 width = 320.0f;
		float                 height = 80.0f;
		std::function<void()> onDecide;             //!< 決定されたときの処理
	};

	/// baseX:項目の左端X / topY:1番目の中心Y / lineGap:項目の間隔
	void Initialize(float baseX, float topY, float lineGap);
	void AddItem(const ItemDesc& desc);
	void SetSelector(const std::string& texturePath, float width, float height);
	void Update(float dt);

	int  GetSelectedIndex() const { return selected_; }

private:
	//===================================================================*/
	//						private methods
	//===================================================================*/
	void  UpdateInput();
	void  UpdateVisual(float dt);
	float ItemCenterY(int index) const;

private:
	//===================================================================*/
	//						private variables
	//===================================================================*/
	struct Item {
		std::shared_ptr<UiSprite> sprite;
		std::function<void()>     onDecide;
		float                     offsetX = 0.0f;   //!< 現在の横ずれ量(補間用)
	};

	std::vector<Item>         items_;
	std::shared_ptr<UiSprite> selector_;

	int   selected_ = 0;
	float baseX_ = 0.0f;
	float topY_ = 0.0f;
	float lineGap_ = 100.0f;

	float selectedOffsetX_ = 24.0f;   //!< 選択中に右へずらす量(px)
	float offsetLerpSpeed_ = 12.0f;   //!< ずれの追従速度
	float selectorGap_ = 20.0f;       //!< 項目の左端からセレクターまでの距離

	bool  stickTilted_ = false;       //!< 前フレームでスティックが倒れていたか
};

/*-----------------------------------------------------------------------------------------
 * MenuLayout
 * - Title/Clear共通のレイアウト(画面左半分に縦並び)
 *---------------------------------------------------------------------------------------*/
namespace MenuLayout {
	constexpr float kBaseX = 120.0f;        //!< ロゴ・項目の左端X
	constexpr float kLogoY = 140.0f;        //!< ロゴの中心Y
	constexpr float kLogoW = 420.0f;
	constexpr float kLogoH = 140.0f;
	constexpr float kFirstItemY = 330.0f;   //!< 1番目の項目の中心Y
	constexpr float kLineGap = 110.0f;      //!< 項目どうしの間隔
	constexpr float kItemW = 300.0f;
	constexpr float kItemH = 80.0f;
	constexpr float kSelectorW = 40.0f;
	constexpr float kSelectorH = 40.0f;
}