#pragma once

#include <Game/Player/UI/UiSprite.h>

#include <Engine/Foundation/Math/MathUtil.h>
#include <Engine/Foundation/Math/Vector2.h>
#include <Engine/Foundation/Math/Vector3.h>
#include <Engine/Graphics/Camera/3d/Camera3d.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <Engine/Scene/Utility/SceneUtility.h>

#include <memory>
#include <string>

/*-----------------------------------------------------------------------------------------
 * WorldMarkerUI
 * - ワールド上の一点を画面座標へ変換して追従させる2Dマーカー(頭上のボタン表示など)
 *---------------------------------------------------------------------------------------*/
class WorldMarkerUI {
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	/// テクスチャと表示サイズ(px)を決めてスプライトを1枚だけ作る
	void Initialize(const std::string& texturePath, float sizePx) {
		if (sprite_) {
			return; // 二重初期化を防ぐ
		}
		sprite_ = SceneAPI::Instantiate<UiSprite>(texturePath);
		sprite_->SetAnchor({ 0.5f, 0.5f });
		sprite_->SetSizePx(sizePx, sizePx);
		sprite_->SetOrderInLayer(10); // HPバーなどより手前
		sprite_->SetVisible(false);
	}

	/// worldPos から heightOffset だけ上に出す
	void ShowAt(const CalyxEngine::Vector3& worldPos, float heightOffset) {
		if (!sprite_) {
			return;
		}
		const CalyxEngine::Vector3 target{ worldPos.x, worldPos.y + heightOffset, worldPos.z };

		// カメラの後ろは WorldToScreen が反転した座標を返すので、自前で弾く
		if (!IsInFrontOfCamera(target)) {
			sprite_->SetVisible(false);
			return;
		}

		const CalyxEngine::Vector2 screen = CalyxEngine::WorldToScreen(target);
		sprite_->SetPositionPx(screen.x, screen.y);
		sprite_->SetVisible(true);
	}

	void Hide() {
		if (sprite_) {
			sprite_->SetVisible(false);
		}
	}

private:
	//===================================================================*/
	//						private methods
	//===================================================================*/
	static bool IsInFrontOfCamera(const CalyxEngine::Vector3& worldPos) {
		auto* camera = CameraManager::GetMain3d();
		if (!camera) {
			return true;
		}
		const CalyxEngine::Vector3 toTarget =
			worldPos - camera->GetWorldTransform().GetWorldPosition();
		return CalyxEngine::Vector3::Dot(toTarget, camera->GetForward()) > 0.0f;
	}

	//===================================================================*/
	//						private variables
	//===================================================================*/
	std::shared_ptr<UiSprite> sprite_;
};
