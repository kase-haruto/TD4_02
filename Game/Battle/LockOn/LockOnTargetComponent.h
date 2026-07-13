#pragma once
#include "ILockOnTarget.h"
#include <Engine/Objects/Transform/Transform.h>
 
/*------------------------------------------------------------------------------------------
 * LockOnTargetComponent
 * - ロックオン対象のコンポーネント
 * - このコンポーネントを持つアクターはロックオン対象となる
 -----------------------------------------------------------------------------------------*/

class LockOnTargetComponent final : 
	public ILockOnTarget {
public:
	//===================================================================*/
	//                    public methods
	//===================================================================*/
	~LockOnTargetComponent() override = default;

	/**
	 * @brief ロックオン対象の種類を取得する
	 * @return LockOnTargetType ロックオン対象の種類
	 */
	LockOnTargetType GetLockOnTargetType() const override;

	/**
	 * @brief ロックオン可能かどうかを取得する
	 * @return bool ロックオン可能かどうか
	 */
	bool IsLockable() const override;

	/**
	 * @brief ロックオンの基準となるトランスフォームを取得する
	 * @return BaseTransform* ロックオンの基準となるトランスフォーム
	 */
	CalyxEngine::Vector3 GetWorldPosition() const;

private:
	//===================================================================*/
	//                    private methods
	//===================================================================*/
	LockOnTargetType targetType_ = LockOnTargetType::Enemy;	//< ロックオン対象の種類
	BaseTransform* lockOnAnchor_ = nullptr;	//< ロックオンの基準となるトランスフォーム

	bool isLockable_ = true;	//< ロックオン可能かどうか
	bool ownerIsAlive_ = true;	//< 所有者が生存しているかどうか
};

