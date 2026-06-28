#pragma once

#include <Enemy/BaseEnemy.h>

/*-----------------------------------------------------------------------------------------
 * HeavySlime
 * - ノックバックしない敵(仮)
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "HeavySlime")
class HeavySlime
	: public BaseEnemy {
public:
	HeavySlime();
	~HeavySlime() override = default;

	void Initialize() override;

private:
	struct HeavySlimeStats : EnemyStats {
		HeavySlimeStats() {
			maxHp = 5;
			moveSpeed = 1.0f;
			contactDamage = 1;
			attackRange = 1.2f;
			detectionRange = 6.0f;
			knockbackInitialSpeed = 0.0f;
		}
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "HeavySlime", "Actor/Enemy" };
		}
	};

	HeavySlimeStats statsImpl_;
};