#pragma once

#include <Enemy/Enemies/ShieldEnemyBase.h>

/*-----------------------------------------------------------------------------------------
 * ShieldEnemy
 * - 盾持ち
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "ShieldEnemy")
class ShieldEnemy
	: public ShieldEnemyBase {
public:
	ShieldEnemy();
	~ShieldEnemy() override = default;

	void Initialize() override;

private:
	struct ShieldEnemyStats : EnemyStats {
		ShieldEnemyStats() {
			maxHp = 5;
			moveSpeed = 1.5f;
			contactDamage = 1;
			attackRange = 1.2f;
			detectionRange = 6.0f;
		}
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "ShieldEnemy", "Actor/Enemy" };
		}
	};

	ShieldEnemyStats statsImpl_;
};