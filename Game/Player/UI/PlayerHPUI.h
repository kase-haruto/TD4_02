#pragma once

#include <Engine/Objects/2D/Object2d/SpriteSceneObject2d.h>

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
	PlayerHPUI();
	~PlayerHPUI() override = default;

	void AlwaysUpdate(float dt) override;

	/// 現在HP/最大HP を渡すと 0..1 の割合として反映する
	void SetHp(int current, int max);

private:
	//===================================================================*/
	//						private variables
	//===================================================================*/
	float hpRatio_   = 1.0f;   // 0..1
	float fullWidth_ = 400.0f; // 満タン時の横幅(px)
	float height_    = 32.0f;  // 高さ(px)
};
