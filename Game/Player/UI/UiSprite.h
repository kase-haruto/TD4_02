#pragma once

#include <Engine/Objects/2D/Object2d/SpriteSceneObject2d.h>
#include <string>

/*-----------------------------------------------------------------------------------------
 * UiSprite
 * - 色/位置/サイズ/表示を動かす汎用2Dスプライト
 *---------------------------------------------------------------------------------------*/
class UiSprite
	: public CalyxEngine::SpriteSceneObject2d {
public:
	UiSprite() = default;
	/// 生成時にテクスチャを指定する(空なら白1x1のまま)
	UiSprite(const std::string& texturePath) {
		if (!texturePath.empty()) {
			texturePath_ = texturePath;
		}
	}
	~UiSprite() override = default;

	void SetPositionPx(float x, float y) { worldTransform_.translation = { x, y, 0.0f }; }
	void SetSizePx(float w, float h) { worldTransform_.scale = { w, h, 1.0f }; }
	void SetColorRGBA(float r, float g, float b, float a = 1.0f) { color_ = { r, g, b, a }; }
	void SetVisible(bool visible) { SetDrawEnable(visible); }
};
