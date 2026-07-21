#pragma once

#include <Engine/Foundation/Math/Vector3.h>

/*-----------------------------------------------------------------------------------------
 * KillPlane
 * - 落下死のフェイルセーフ
 * - 床のすき間やコライダーの取りこぼしで落ちたキャラを、この高さで死亡扱いにする
 *---------------------------------------------------------------------------------------*/
namespace KillPlane {

	//! この高さより下に落ちたら死亡扱い
	constexpr float kDeathY = -20.0f;

	inline bool IsFallenOut(const CalyxEngine::Vector3& worldPosition) {
		return worldPosition.y < kDeathY;
	}

} // namespace KillPlane
