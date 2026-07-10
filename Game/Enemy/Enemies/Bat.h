#pragma once

#include <Enemy/BaseEnemy.h>

/*-----------------------------------------------------------------------------------------
 * Bat
 * - 突撃する敵(仮)
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Bat", Placeable = false, SceneSerializable = false, PrefabSerializable = false)
class Bat
	: public BaseEnemy {
public:
	Bat();
	~Bat() override = default;

	void Initialize() override;

protected:
	bool AllowMovement() const override { return !(attack_ && attack_->IsAttacking()); }

private:
	struct BatStats : EnemyStats {
		BatStats() {
			maxHp = 5;
			moveSpeed = 3.0f;
			contactDamage = 1;
			attackDamage = 2;
			attackRange = 5.0f;     // この距離で突進を開始
			attackInterval = 2.0f;  // 突進のクールダウン
			detectionRange = 10.0f;
		}
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "Bat", "Actor/Enemy" };
		}
	};

	BatStats statsImpl_;
};
