#include "PlayerBase.h"

#include <array>

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor / dtor
/////////////////////////////////////////////////////////////////////////////////////////
PlayerBase::PlayerBase()
	: PlayerBase(PlayerModelSet::Player) {}

PlayerBase::PlayerBase(PlayerModelSet modelSet)
	: Actor(modelSet == PlayerModelSet::Spirit ? "Spirit_idle.gltf" : "Player_idle.gltf", "Player"),
	modelSet_(modelSet) {}

namespace {
	const char* GetPlayerAnimationName(PlayerAnimationID animationId) {
		switch (animationId) {
		case PlayerAnimationID::Idle:      return "Idle";
		case PlayerAnimationID::MoveFront: return "MoveFront";
		case PlayerAnimationID::MoveBack:  return "MoveBack";
		case PlayerAnimationID::MoveLeft:  return "MoveLeft";
		case PlayerAnimationID::MoveRight: return "MoveRight";
		case PlayerAnimationID::Attack1:   return "Attack1";
		case PlayerAnimationID::Attack2:   return "Attack2";
		case PlayerAnimationID::Spirit:    return "Spirit";
		case PlayerAnimationID::Dodge:     return "Dodge";
		case PlayerAnimationID::Damage:    return "Damage";
		default: return "Idle";
		}
	}

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

	//! 待機/移動系か(入力で細かく切り替わるグループ)
	bool IsLocomotionAnimation(PlayerAnimationID id) {
		switch (id) {
		case PlayerAnimationID::Idle:
		case PlayerAnimationID::MoveFront:
		case PlayerAnimationID::MoveBack:
		case PlayerAnimationID::MoveLeft:
		case PlayerAnimationID::MoveRight:
			return true;
		default:
			return false;
		}
	}

	//! 遷移の種類に応じたブレンド時間
	float GetPlayerAnimationBlendDuration(PlayerAnimationID from, PlayerAnimationID to) {
		if (IsLocomotionAnimation(from) && IsLocomotionAnimation(to)) {
			return kPlayerLocomotionBlendDuration;
		}
		return kPlayerAnimationBlendDuration;
	}

	int GetPlayerAnimationPriority(PlayerAnimationID id) {
		switch (id) {
		case PlayerAnimationID::Damage:    return 50;
		case PlayerAnimationID::Dodge:     return 40;
		case PlayerAnimationID::Attack1:
		case PlayerAnimationID::Attack2:   return 30;
		case PlayerAnimationID::Spirit:    return 20;
		case PlayerAnimationID::MoveFront:
		case PlayerAnimationID::MoveBack:
		case PlayerAnimationID::MoveLeft:
		case PlayerAnimationID::MoveRight: return 10;
		default:                           return 0;   // Idle
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
	dodge_.Initialize(this);
	RegisterPlayerAnimations();
	PlayAnimation(PlayerAnimationID::Idle);
}

/////////////////////////////////////////////////////////////////////////////////////////
//			更新
/////////////////////////////////////////////////////////////////////////////////////////
void PlayerBase::Update(float dt) {
	if (UpdateKnockback(dt)) {
		FlushAnimation(dt);
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

	FlushAnimation(dt);
	Actor::Update(dt);
}

void PlayerBase::RegisterPlayerAnimations() {
	if (playerAnimationsRegistered_) {
		return;
	}

	auto* model = AnimationModel();
	if (!model) {
		return;
	}

	constexpr std::array<PlayerAnimationID, 10> kAnimations = {
		PlayerAnimationID::Idle,
		PlayerAnimationID::MoveFront,
		PlayerAnimationID::MoveBack,
		PlayerAnimationID::MoveLeft,
		PlayerAnimationID::MoveRight,
		PlayerAnimationID::Attack1,
		PlayerAnimationID::Attack2,
		PlayerAnimationID::Spirit,
		PlayerAnimationID::Dodge,
		PlayerAnimationID::Damage,
	};

	for (PlayerAnimationID id : kAnimations) {
		RegisterAnimationClip(
			static_cast<int16_t>(id),
			GetPlayerAnimationName(id),
			GetPlayerAnimationModelName(modelSet_, id));
	}

	playerAnimationsRegistered_ = true;
}

void PlayerBase::PlayAnimation(PlayerAnimationID animationId, float blendDuration) {
	if (currentAnimationId_ == animationId) {
		return;
	}

	currentAnimationId_ = animationId;
	currentAnimationElapsed_ = 0.0f;

	auto* model = AnimationModel();
	if (!model) {
		SetModelFileNameForEditor(GetPlayerAnimationModelName(modelSet_, animationId));
		playerAnimationsRegistered_ = false;
		return;
	}

	RegisterPlayerAnimations();
	PlayRegisteredAnimation(static_cast<int16_t>(animationId), blendDuration);
}

void PlayerBase::ApplyKnockback(const CalyxEngine::Vector3& velocity, float friction) {
	knockbackVelocity_ = velocity;
	knockbackFriction_ = friction;
}

void PlayerBase::RequestAnimation(PlayerAnimationID animationId) {
	const int priority = GetPlayerAnimationPriority(animationId);
	if (priority < requestedPriority_) {
		return;
	}
	requestedPriority_ = priority;
	requestedAnimationId_ = animationId;
}

void PlayerBase::FlushAnimation(float dt) {
	currentAnimationElapsed_ += dt;

	if (requestedPriority_ >= 0) {
		const float base = GetPlayerAnimationBlendDuration(currentAnimationId_, requestedAnimationId_);
		const float blend = currentAnimationElapsed_ >= base ? base : 0.0f;
		PlayAnimation(requestedAnimationId_, blend);
	}
	requestedPriority_ = -1;
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
	attack_.ShowGui(*this);
	motor_.ShowGui();
	dodge_.ShowGui();
}
