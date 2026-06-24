#pragma once

#include <Engine/Foundation/Math/Vector3.h>


class BaseEnemy;

/*-----------------------------------------------------------------------------------------
 * IEnemyMovement
 * - 敵の動きのインターフェース
 *---------------------------------------------------------------------------------------*/
class IEnemyMovement {
public:
	virtual ~IEnemyMovement() = default;

	virtual void Update(BaseEnemy& self, const CalyxEngine::Vector3& targetPosition, float dt) = 0;
};