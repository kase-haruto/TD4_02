#pragma once

#include <Demo/Input/PlayerInput.h>
#include <Engine/Foundation/Math/Vector3.h>
#include <Engine/Foundation/Serialization/SerializableObject.h>

class PlayerBase;

/*-----------------------------------------------------------------------------------------
 * PlayerDodge
 * - プレイヤーの回避
 *---------------------------------------------------------------------------------------*/
class PlayerDodge {
public:

	void Initialize(PlayerBase* player);
	void Update(PlayerBase* player, const PlayerInputState& input, float dt);
	void ShowGui();

	bool IsDodging() const { return isDodging_; }
	bool IsInvincible() const { return invincibleTimer_ > 0.0f; }

	void Reset();

	void SetEnabled(bool e) { enabled_ = e; }

private:
	void StartDodge(PlayerBase* player);

private:
	struct PlayerDodgeParam : CalyxEngine::SerializableObject {
		PlayerDodgeParam() {
			AddField("dodgeSpeed", dodgeSpeed).Category("Dodge").Tooltip("回避速度");
			AddField("dodgeDuration", dodgeDuration).Category("Dodge").Tooltip("回避時間");
			AddField("invincibleTime", invincibleTime).Category("Dodge").Tooltip("無敵時間");
			AddField("cooldown", cooldown).Category("Dodge").Tooltip("クールダウン");
		}

		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "PlayerDodge", "Actor/Player/Dodge" };
		}

		float dodgeSpeed = 10.0f; // 回避速度
		float dodgeDuration = 0.30f; // 回避時間
		float invincibleTime = 0.25f; // 無敵時間
		float cooldown = 0.40f; // クールダウン
	}param_;

	bool  isDodging_ = false;
	bool  enabled_ = true;   // 橋などから一時的に回避を封じる
	float dodgeTimer_ = 0.0f; // 回避の経過時間
	float invincibleTimer_ = 0.0f; // 無敵の残り時間
	float cooldownTimer_ = 0.0f; // 再回避までの残り時間
	CalyxEngine::Vector3 dodgeDir_ = { 0.0f, 0.0f, 0.0f }; // 回避方向（開始時の向き）
};