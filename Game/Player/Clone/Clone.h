#pragma once

// クローン
#include "../Base/PlayerBase.h"
// game
#include <Demo/Input/PlayerInput.h>
#include "../PlayerMotor.h"
#include "../PlayerDodge.h"
#include "../Sword/Sword.h"
#include "../Ability/PlayerAbility.h"

#include <Engine/Application/Effects/EffectAsset.h>
#include <Engine/Application/Effects/EffectPlayer.h>

/*-----------------------------------------------------------------------------------------
 * PlayerClone
 * - Playerのクローン
 * - プレイヤーのコピーであり、プレイヤーと同じ動作をする
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "PlayerClone")
class PlayerClone
	:public PlayerBase {
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	PlayerClone();
	~PlayerClone() override = default;
	void Update(float dt) override;
	void OnCollisionEnter(Collider* other) override;
	void SetGhost(bool isGhost) { isGhost_ = isGhost; }
	bool IsGhost() const { return isGhost_; }

	void SetAimOrigin(const PlayerBase* origin) { motor_.SetAimOrigin(origin); }

	//! ロックオン状態を共有する。ロックオン中はクローンもターゲットを向く
	void SetLockOnState(const ILockOnStateReader* reader) { motor_.SetLockOnStateReader(reader); }

	void SetOwnerAbility(PlayerAbility* ability) { ownerAbility_ = ability; }

	void AddWorldOffset(const CalyxEngine::Vector3& delta);

private:
	//===================================================================*/
	//						private methods
	//===================================================================*/
	void StartVanish();          //!< 消滅演出を開始する
	void UpdateVanish(float dt); //!< 消滅演出の更新

	//===================================================================*/
	//						private variables
	//===================================================================*/
	bool isGhost_ = false;
	PlayerAbility* ownerAbility_ = nullptr;

	bool isVanishing_ = false;                                   //!< 消滅演出中か
	float vanishTime_ = 0.0f;                                    //!< 演出の経過時間
	CalyxEngine::Vector3 vanishBaseScale_ = {1.0f, 1.0f, 1.0f};  //!< 演出開始時のスケール

	CalyxEngine::EffectAsset deleteEf_;

};
