#pragma once

#include "IEnemyAttack.h"

#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <Engine/Foundation/Math/Vector3.h>
#include <memory>

class Sword;

/*-----------------------------------------------------------------------------------------
 * ChargeAttack
 * - プレイヤーへ突進し、少し通り過ぎるまで突っ込む攻撃
 *---------------------------------------------------------------------------------------*/
class ChargeAttack : public IEnemyAttack {
public:
	ChargeAttack();

	void Update(BaseEnemy& self, const Actor* target, float dt) override;
	void Cancel() override;
	void ShowGui() override;
	bool IsAttacking() const override { return phase_ != Phase::Idle; }
	CalyxEngine::SerializableObject& SerializableParam() override;

private:
	enum class Phase {
		Idle,     // 待機
		Windup,   // 溜め
		Dash,     // 突進中
		Recover,  // 突進後の硬直
	};

	void StartCharge(BaseEnemy& self, const Actor* target);
	void UpdateWindup(BaseEnemy& self, float dt);
	void UpdateDash(BaseEnemy& self, float dt);
	void UpdateRecover(BaseEnemy& self, float dt);
	void EndCharge(BaseEnemy& self);

	void CreateHitbox(BaseEnemy& self);
	void UpdateHitbox(BaseEnemy& self);
	void RemoveHitbox();

	static float PlanarDistanceSq(const BaseEnemy& self, const Actor* target);

private:
	struct ChargeAttackParam : CalyxEngine::SerializableObject {
		ChargeAttackParam() {
			AddField("windupTime", windupTime)
				.Category("Charge").Tooltip("突進前の溜め時間");
			AddField("chargeSpeed", chargeSpeed)
				.Category("Charge").Tooltip("突進の速さ");
			AddField("overshootDistance", overshootDistance)
				.Category("Charge").Tooltip("プレイヤーをどれだけ通り過ぎるか");
			AddField("maxChargeTime", maxChargeTime)
				.Category("Charge").Tooltip("突進の最大時間（保険のタイムアウト）");
			AddField("recoverTime", recoverTime)
				.Category("Charge").Tooltip("突進後の硬直時間");

			AddField("hitboxSize", hitboxSize)
				.Category("Hitbox").Tooltip("突進中の当たり判定サイズ");
			AddField("hitboxHeightOffset", hitboxHeightOffset)
				.Category("Hitbox").Tooltip("当たり判定の高さ");
			AddField("drawHitbox", drawHitbox)
				.Category("Hitbox").Tooltip("当たり判定をデバッグ表示するか");
		}

		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "ChargeAttack", "Actor/Enemy/AttackParam" };
		}

		float windupTime = 0.4f;
		float chargeSpeed = 12.0f;
		float overshootDistance = 2.0f;
		float maxChargeTime = 1.2f;
		float recoverTime = 0.5f;

		CalyxEngine::Vector3 hitboxSize = { 1.0f, 0.8f, 1.0f };
		float hitboxHeightOffset = 1.0f;
		bool  drawHitbox = true;
	};

	ChargeAttackParam param_;

	Phase phase_ = Phase::Idle;
	float phaseTimer_ = 0.0f;
	float cooldownTimer_ = 0.0f;

	CalyxEngine::Vector3 chargeDir_{};        // 突進方向
	CalyxEngine::Vector3 chargeTargetPos_{};  // 通り過ぎる目標地点

	std::shared_ptr<Sword> hitbox_;
};