#pragma once

#include <Enemy/BaseEnemy.h>

/*-----------------------------------------------------------------------------------------
 * Slime
 * - 基本の敵(仮)
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Slime", Placeable = false, SceneSerializable = false, PrefabSerializable = false)
class Slime
	: public BaseEnemy {
public:
	Slime();
	~Slime() override = default;

	void Initialize() override;

private:
	struct SlimeStats : EnemyStats {
		SlimeStats() {
			maxHp = 5;
			moveSpeed = 1.5f;
			contactDamage = 1;
			attackRange = 1.2f;
			detectionRange = 6.0f;
		}
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "Slime", "Actor/Enemy" };
		}
	};

	SlimeStats statsImpl_;
};
