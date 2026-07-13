#pragma once

#include "IEnemyAttack.h"

#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <Engine/Foundation/Math/Vector3.h>

/*-----------------------------------------------------------------------------------------
 * RangedAttack
 * - 一定距離(attackRange)に入ったら矢(Arrow)を撃つ攻撃
 * - 矢の速度・寿命はここのパラメータで持つ
 *---------------------------------------------------------------------------------------*/
class RangedAttack : public IEnemyAttack {
public:
	RangedAttack();

	void Update(BaseEnemy& self, const Actor* target, float dt) override;
	void ShowGui() override;
	bool IsAttacking() const override { return isAiming_; }
	CalyxEngine::SerializableObject& SerializableParam() override;

private:
	void Fire(BaseEnemy& self, const Actor* target);
	static float PlanarDistanceSq(const BaseEnemy& self, const Actor* target);

private:
	struct RangedAttackParam : CalyxEngine::SerializableObject {
		RangedAttackParam() {
			AddField("aimDuration", aimDuration)
				.Category("Arrow").Tooltip("Time spent playing the aim animation before firing");
			AddField("arrowSpeed", arrowSpeed)
				.Category("Arrow").Tooltip("矢の速さ");
			AddField("arrowLifeTime", arrowLifeTime)
				.Category("Arrow").Tooltip("矢の寿命(秒)。この時間で消える");
			AddField("spawnForwardOffset", spawnForwardOffset)
				.Category("Arrow").Tooltip("発射位置の前方オフセット");
			AddField("spawnHeight", spawnHeight)
				.Category("Arrow").Tooltip("発射位置の高さ");
			AddField("arrowSize", arrowSize)
				.Category("Arrow").Tooltip("矢の当たり判定＆見た目サイズ");
			AddField("drawHitbox", drawHitbox)
				.Category("Arrow").Tooltip("当たり判定を線で表示するか");
		}

		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "RangedAttack", "Actor/Enemy/AttackParam" };
		}

		float aimDuration = 0.4f;
		float arrowSpeed = 8.0f;
		float arrowLifeTime = 3.0f;
		float spawnForwardOffset = 1.0f;
		float spawnHeight = 1.0f;
		CalyxEngine::Vector3 arrowSize = { 0.3f, 0.3f, 0.6f };
		bool  drawHitbox = false;
	};

	RangedAttackParam param_;
	float cooldownTimer_ = 0.0f;
	float aimTimer_ = 0.0f;
	bool isAiming_ = false;
};
