#include "PlayerBase.h"

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor / dtor
/////////////////////////////////////////////////////////////////////////////////////////
PlayerBase::PlayerBase()
	: PlayerBase(PlayerModelSet::Player) {}

PlayerBase::PlayerBase(PlayerModelSet modelSet)
	: Actor(modelSet == PlayerModelSet::Spirit ? "Spirit_idle.gltf" : "Player_idle.gltf", "Player"),
	modelSet_(modelSet) {}

namespace {
	const char* GetPlayerAnimationModelName(PlayerModelSet modelSet, PlayerAnimationID animationId) {
		if (modelSet == PlayerModelSet::Spirit) {
			switch (animationId) {
			case PlayerAnimationID::Attack1: return "Spirit_attack1.gltf";
			case PlayerAnimationID::Attack2: return "Spirit_attack2.gltf";
			case PlayerAnimationID::Dodge:   return "Spirit_dodge.gltf";
			case PlayerAnimationID::MoveFront:
			case PlayerAnimationID::MoveBack:
			case PlayerAnimationID::MoveLeft:
			case PlayerAnimationID::MoveRight:
				return "Spirit_move.gltf";
			default:
				return "Spirit_idle.gltf";
			}
		}

		switch (animationId) {
		case PlayerAnimationID::Idle:
			return "Player_idle.gltf";
		case PlayerAnimationID::MoveFront:
			return "Player_move_front.gltf";
		case PlayerAnimationID::MoveBack:
			return "Player_move_back.gltf";
		case PlayerAnimationID::MoveLeft:
			return "Player_move_left.gltf";
		case PlayerAnimationID::MoveRight:
			return "Player_move_right.gltf";
		case PlayerAnimationID::Attack1:
			return "Player_attack1.gltf";
		case PlayerAnimationID::Attack2:
			return "Player_attack2.gltf";
		case PlayerAnimationID::Spirit:
			return "Player_spilit.gltf";
		case PlayerAnimationID::Dodge:
			return "Player_dodge.gltf";
		case PlayerAnimationID::Damage:
			return "Player_damage.gltf";
		default:
			return "Player_idle.gltf";
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
	if (UpdateKnockback(dt)) {
		Actor::Update(dt);
		return;
	}

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
	const std::string modelName = GetPlayerAnimationModelName(modelSet_, animationId);
	if (currentAnimationModel_ == modelName) {
		currentAnimationId_ = animationId;
		return;
	}

	currentAnimationId_ = animationId;
	currentAnimationModel_ = modelName;
	SetModelFileNameForEditor(modelName);
}

void PlayerBase::ApplyKnockback(const CalyxEngine::Vector3& velocity, float friction) {
	knockbackVelocity_ = velocity;
	knockbackFriction_ = friction;
}

bool PlayerBase::UpdateKnockback(float dt) {
	if (knockbackVelocity_.LengthSquared() <= 0.01f) {
		knockbackVelocity_ = {};
		return false;
	}
	GetWorldTransform().translation += knockbackVelocity_ * dt;
	float damp = 1.0f - knockbackFriction_ * dt;
	if (damp < 0.0f) damp = 0.0f;
	knockbackVelocity_ = knockbackVelocity_ * damp;
	return true;
}

void PlayerBase::DerivativeGui() {
	attack_.ShowGui();
}
