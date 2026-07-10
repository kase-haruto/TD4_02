#pragma once

#include <Enemy/BaseEnemy.h>

/*-----------------------------------------------------------------------------------------
 * Archer
 * - 弓兵
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Archer", Placeable = false, SceneSerializable = false, PrefabSerializable = false)
class Archer
	: public BaseEnemy {
public:
	Archer();
	~Archer() override = default;

	void Initialize() override;

private:
	struct ArcherStats : EnemyStats {
		ArcherStats() {
			maxHp = 5;
			moveSpeed = 2.0f;
			contactDamage = 1;
			attackDamage = 2;
			attackRange = 6.0f;
			attackInterval = 1.5f;
			detectionRange = 12.0f;
		}
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "Archer", "Actor/Enemy" };
		}
	};

	ArcherStats statsImpl_;
};
