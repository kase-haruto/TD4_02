#pragma once

#include <Engine/Objects/3D/Actor/Actor.h>

// game
#include <Demo/Input/PlayerInput.h>
#include "../PlayerMotor.h"
#include "../PlayerDodge.h"
#include "../Sword/Sword.h"
#include "../Attack/PlayerAttack.h"
#include "../Ability/PlayerAbility.h"

enum class PlayerAnimationID {
	Idle,
	Walk,
	Attack1,
	Attack2,
	Dodge,
};

/*-----------------------------------------------------------------------------------------
 * PlayerBase
 * - Playerの基底クラス
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "PlayerBase", Icon = "UI/Tool/cube.dds")
class PlayerBase
	:public Actor {
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	PlayerBase();
	~PlayerBase() override = default;

	virtual void Initialize() override;
	virtual void Update(float dt) override;
	void DerivativeGui() override;
	void PlayAnimation(PlayerAnimationID animationId);
	PlayerAnimationID GetCurrentAnimationId() const { return currentAnimationId_; }

protected:
	//===================================================================*/
	//						private variables
	//===================================================================*/
	PlayerInput input_;
	PlayerMotor motor_;
	PlayerDodge dodge_;
	PlayerAttack attack_;
	PlayerAnimationID currentAnimationId_ = PlayerAnimationID::Idle;
};
