#pragma once

#include <Engine/Objects/Transform/Transform.h>

enum class LockOnTargetType {
	Enemy,
};

/*-----------------------------------------------------------------------------------------
 * ILockOnTarget
 * - ロックオン対象のインターフェースクラス
 *---------------------------------------------------------------------------------------*/
class ILockOnTarget{

	/**
	 * @brief デストラクタ
	 */
	virtual ~ILockOnTarget() = default;

	/**
	 * @brief ロックオン対象の種類を取得する
	 * @return LockOnTargetType ロックオン対象の種類
	 */
	virtual LockOnTargetType GetLockOnTargetType() const = 0;

	/**
	 * @brief ロックオン可能かどうかを取得する
	 * @return bool ロックオン可能かどうか
	 */
	virtual bool IsLockable() const = 0;

	virtual BaseTransform* GetPosition() const = 0;
};

