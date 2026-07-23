#pragma once

#include "IEnemyAttack.h"

#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <Engine/Foundation/Math/Vector3.h>
#include <memory>

class Sword;

/*-----------------------------------------------------------------------------------------
 * PunchAttack
 * - 多くの敵が共通で使う近接攻撃
 *---------------------------------------------------------------------------------------*/
class PunchAttack : public IEnemyAttack {
public:
	PunchAttack();

	void Update(BaseEnemy& self, const Actor* target, float dt)override;
	void Cancel()override;
	void ShowGui()override;
	bool IsAttacking() const override { return isAttacking_; }
	CalyxEngine::SerializableObject& SerializableParam()override;

private:

	void StartAttack(BaseEnemy& self, const Actor* target);
	void UpdateAttack(BaseEnemy& self, float dt);
	void EndAttack(BaseEnemy& self);

	bool IsHitboxActive() const;
	void CreateAttackHitbox(BaseEnemy& self);
	void UpdateAttackHitbox(BaseEnemy& self);
	void RemoveAttackHitbox();

	static float PlanarDistanceSq(const BaseEnemy& self, const Actor* target);

private:
	struct PunchAttackParam : CalyxEngine::SerializableObject {
		PunchAttackParam() {
			AddField("attackDuration", attackDuration)
				.Category("Attack").Tooltip("攻撃モーション全体の時間");

			AddField("hitboxActiveStart", hitboxActiveStart)
				.Category("AttackHitbox").Tooltip("攻撃判定を出し始める時間（振りかぶり）");
			AddField("hitboxActiveEnd", hitboxActiveEnd)
				.Category("AttackHitbox").Tooltip("攻撃判定を消す時間");

			AddField("hitboxForwardOffset", hitboxForwardOffset)
				.Category("AttackHitbox").Tooltip("攻撃判定の前方距離");
			AddField("hitboxHeightOffset", hitboxHeightOffset)
				.Category("AttackHitbox").Tooltip("攻撃判定の高さ");
			AddField("hitboxSize", hitboxSize)
				.Category("AttackHitbox").Tooltip("攻撃判定のサイズ");
			AddField("drawHitbox", drawHitbox)
				.Category("AttackHitbox").Tooltip("攻撃判定をデバッグ表示するか");
			AddField("knockbackPower", knockbackPower)
				.Category("Attack").Tooltip("命中時にプレイヤーを吹き飛ばす初速");
		}

		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "PunchAttack", "Actor/Enemy/AttackParam" };
		}

		float attackDuration = 1.00f;
		float hitboxActiveStart = 0.55f;
		float hitboxActiveEnd = 0.70f;
		float hitboxForwardOffset = 1.2f;
		float hitboxHeightOffset = 1.0f;
		CalyxEngine::Vector3 hitboxSize = { 1.6f, 1.2f, 1.6f };
		bool drawHitbox = true;
		float knockbackPower = 18.0f;
	};

	PunchAttackParam param_;

	bool isAttacking_ = false;
	float attackTimer_ = 0.0f;
	float cooldownTimer_ = 0.0f;

	std::shared_ptr<Sword> attackHitbox_;
};