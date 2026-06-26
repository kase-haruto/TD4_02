#include "PlayerBase.h"

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor / dtor
/////////////////////////////////////////////////////////////////////////////////////////
PlayerBase::PlayerBase()
	: Actor("PlayerIdle.gltf", "Player") {
	SerializableParamObjectsMutable().push_back(&attack_.SerializableParam());
}

namespace {
	const char* GetPlayerAnimationModelName(PlayerAnimationID animationId) {
		switch (animationId) {
		case PlayerAnimationID::Idle:
			return "PlayerIdle.gltf";
		case PlayerAnimationID::Walk:
			return "PlayerWalk.gltf";
		case PlayerAnimationID::Attack1:
			return "PlayerAttack1.gltf";
		case PlayerAnimationID::Attack2:
			return "PlayerAttack2.gltf";
		case PlayerAnimationID::Dodge:
			return "PlayerDodge.gltf";
		default:
			return "PlayerIdle.gltf";
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void PlayerBase::Initialize() {
	Actor::Initialize();

	// 初期化
	motor_.Initialize(this);
	PlayAnimation(PlayerAnimationID::Idle);
}

/////////////////////////////////////////////////////////////////////////////////////////
//			更新
/////////////////////////////////////////////////////////////////////////////////////////
void PlayerBase::Update(float dt) {
	input_.Update();

	const PlayerInputState& in = input_.GetState();

	// 回避を先に処理
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

void PlayerBase::PlayAnimation(PlayerAnimationID animationId) {
	if (currentAnimationId_ == animationId) {
		return;
	}

	currentAnimationId_ = animationId;
	SetModelFileNameForEditor(GetPlayerAnimationModelName(animationId));
}

void PlayerBase::DerivativeGui() {
	attack_.ShowGui();
}
