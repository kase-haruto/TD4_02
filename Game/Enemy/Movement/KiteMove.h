#pragma once

#include <Enemy/IEnemyMovement.h>

/*-----------------------------------------------------------------------------------------
 * KiteMove
 * - 弓兵向けの間合いを取る動き
 *---------------------------------------------------------------------------------------*/
class KiteMove
    : public IEnemyMovement {
public:
    void Update(BaseEnemy& self, const CalyxEngine::Vector3& targetPos, float dt) override;

private:
    int   strafeSide_ = 1;       // 回り込む向き
    float strafeTimer_ = 0.0f;   // 向きを切り替えるまでの計測
};