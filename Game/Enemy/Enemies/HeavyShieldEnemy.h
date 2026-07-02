#pragma once

#include <Enemy/Enemies/ShieldEnemyBase.h>

/*-----------------------------------------------------------------------------------------
 * HeavyShieldEnemy
 * - 盾持ち(ノックバックしない) 
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "HeavyShieldEnemy")
class HeavyShieldEnemy
	: public ShieldEnemyBase {
public:
	HeavyShieldEnemy();
	~HeavyShieldEnemy() override = default;

	void Initialize() override;

private:
	struct HeavyShieldEnemyStats : EnemyStats {
		HeavyShieldEnemyStats() {
			maxHp = 5;
			moveSpeed = 1.5f;
			contactDamage = 1;
			attackRange = 1.2f;
			detectionRange = 6.0f;
			knockbackInitialSpeed = 0.0f;
		}
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "HeavyShieldEnemy", "Actor/Enemy" };
		}
	};

	HeavyShieldEnemyStats statsImpl_;
};