#include "Player.h"

#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <Engine/Physics/Character/CharacterMovementComponent.h>
#include <Engine/Scene/Utility/SceneUtility.h>

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor / dtor
/////////////////////////////////////////////////////////////////////////////////////////
Player::Player() {
	SerializableParamObjectsMutable().push_back(&ability_.SerializableParam());
}


/////////////////////////////////////////////////////////////////////////////////////////
//			更新
/////////////////////////////////////////////////////////////////////////////////////////
void Player::Update(float dt) {
	input_.Update();

	const PlayerInputState& in = input_.GetState();
	ability_.Update(*this, &in, dt);

	// 回避を先に処理,回避中は移動/向き/ジャンプを受け付けない
	dodge_.Update(this, in, dt);

	// 回避中は攻撃しない
	if (!dodge_.IsDodging()) {
		attack_.Update(*this, in, dt);
	}

	// 回避中・攻撃中は通常移動しない
	if (!dodge_.IsDodging() && !attack_.BlocksMovement()) {
		motor_.Update(this, in, dt);
	}
	Actor::Update(dt);
}

void Player::DerivativeGui(){
	PlayerBase::DerivativeGui();
	ability_.ShowGui();
}
