#pragma once

#include <Demo/Input/PlayerInput.h>
#include <Engine/Foundation/Math/Vector3.h>

class PlayerBase;

/*-----------------------------------------------------------------------------------------
 * PlayerDodge
 * - プレイヤーの回避
 *---------------------------------------------------------------------------------------*/
class PlayerDodge {
public:
	void Update(PlayerBase* player, const PlayerInputState& input, float dt);

	bool IsDodging() const { return isDodging_; }
	bool IsInvincible() const { return invincibleTimer_ > 0.0f; }

private:
	void StartDodge(PlayerBase* player);

private:
	bool  isDodging_ = false;
	float dodgeTimer_ = 0.0f; // 回避の経過時間
	float invincibleTimer_ = 0.0f; // 無敵の残り時間
	float cooldownTimer_ = 0.0f; // 再回避までの残り時間
	CalyxEngine::Vector3 dodgeDir_ = { 0.0f, 0.0f, 0.0f }; // 回避方向（開始時の向き）

	// パラメータ（とりあえず固定値。後で調整 or Json化）
	static constexpr float kDodgeDuration = 0.30f; // 回避時間
	static constexpr float kInvincibleTime = 0.25f; // 無敵時間
	static constexpr float kCooldown = 0.40f; // クールダウン
};