#pragma once

#include <Enemy/BaseEnemy.h>

/*-----------------------------------------------------------------------------------------
 * DoomEnemy
 * - パンチの敵
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "DoomEnemy", Placeable = false, SceneSerializable = false, PrefabSerializable = false)
class DoomEnemy
	: public BaseEnemy {
public:
	DoomEnemy();
	~DoomEnemy() override = default;

	void Initialize() override;

private:
	struct DoomEnemyStats : EnemyStats {
		DoomEnemyStats() {
			maxHp = 5;
			moveSpeed = 1.5f;
			contactDamage = 1;
			attackRange = 1.2f;
			detectionRange = 6.0f;
			attackDamage = 5;
			attackInterval = 2.0f;
		}
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "DoomEnemy", "Actor/Enemy" };
		}
	};

	DoomEnemyStats statsImpl_;
}; 
