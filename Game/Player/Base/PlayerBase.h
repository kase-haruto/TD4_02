#pragma once

#include <Engine/Objects/3D/Actor/Actor.h>

// game
#include <Demo/Input/PlayerInput.h>
#include "../PlayerMotor.h"
#include "../PlayerDodge.h"
#include "../Sword/Sword.h"
#include "../Attack/PlayerAttack.h"
#include "../Ability/PlayerAbility.h"

#include <string>

enum class PlayerAnimationID {
	Idle,
	MoveFront,
	MoveBack,
	MoveLeft,
	MoveRight,
	Attack1,
	Attack2,
	Spirit,
	Dodge,
	Damage,
};

enum class PlayerModelSet {
	Player,
	Spirit,
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
	void RegisterPlayerAnimations();
	PlayerAnimationID GetCurrentAnimationId() const { return currentAnimationId_; }
	CalyxEngine::Vector3 GetMoveDir() const { return motor_.GetMoveDir(); }

	void ApplyKnockback(const CalyxEngine::Vector3& velocity, float friction);
	bool IsKnockedBack() const { return knockbackVelocity_.LengthSquared() > 0.01f; }

	bool AppliesMovement() const { return appliesMovement_; }

protected:
	explicit PlayerBase(PlayerModelSet modelSet);
	bool UpdateKnockback(float dt);

	void SetAppliesMovement(bool v) { appliesMovement_ = v; }

	//===================================================================*/
	//						private variables
	//===================================================================*/
	PlayerInput input_;
	PlayerMotor motor_;
	PlayerDodge dodge_;
	PlayerAttack attack_;
	PlayerAnimationID currentAnimationId_ = PlayerAnimationID::Idle;
	PlayerModelSet modelSet_ = PlayerModelSet::Player;
	bool playerAnimationsRegistered_ = false;

	CalyxEngine::Vector3 knockbackVelocity_{};
	float knockbackFriction_ = 0.0f;

	bool appliesMovement_ = true;
};
