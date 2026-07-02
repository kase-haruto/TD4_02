#pragma once

#include "IEnemyAttack.h"

#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <Engine/Foundation/Math/Vector3.h>
#include <memory>

class Sword;

/*-----------------------------------------------------------------------------------------
 * MeleeAttack
 * - 多くの敵が共通で使う近接攻撃
 *---------------------------------------------------------------------------------------*/
class MeleeAttack : public IEnemyAttack {
public:
	MeleeAttack();

	void Update(BaseEnemy& self, const Actor* target, float dt)override;
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
	struct MeleeAttackParam : CalyxEngine::SerializableObject {
		MeleeAttackParam() {
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
		}

		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "MeleeAttack", "Actor/Enemy/AttackParam" };
		}

		float attackDuration = 0.60f;
		float hitboxActiveStart = 0.25f;
		float hitboxActiveEnd = 0.40f;
		float hitboxForwardOffset = 1.2f;
		float hitboxHeightOffset = 1.0f;
		CalyxEngine::Vector3 hitboxSize = { 1.0f, 0.8f, 1.0f };
		bool drawHitbox = true;
	};

	MeleeAttackParam param_;

	bool isAttacking_ = false;
	float attackTimer_ = 0.0f;
	float cooldownTimer_ = 0.0f;

	std::shared_ptr<Sword> attackHitbox_;
};