#pragma once

#include <Enemy/IEnemyMovement.h>

/*-----------------------------------------------------------------------------------------
 * HomingMove
 * - プレイヤーを索敵したら最短距離で直進して追いかける動き
 * - detectionRange より遠ければ動かない / attackRange 手前で止まる
 *---------------------------------------------------------------------------------------*/
class HomingMove
	: public IEnemyMovement {
public:
	void Update(BaseEnemy& self, const CalyxEngine::Vector3& targetPos, float dt) override;
};