#include "PlayerInput.h"

using namespace CalyxFoundation;

namespace {
	// トリガーを「押した」とみなす押し込み量。
	// 半分より深く握った時だけ反応させ、指を添えただけで暴発しないようにする。
	constexpr float kTriggerPressThreshold = 0.5f;
}

PlayerInput::PlayerInput() {
	ResetBindings();
}

void PlayerInput::Update() {
	// フレームごとに入力状態を作り直す。
	// 古い押下情報が残ると、ジャンプなどの一回だけ処理したい入力が連続発火してしまうため。
	state_ = {};

	// キーボードとゲームパッド十字キーのデジタル入力を取得する。
	CalyxEngine::Vector2 digitalMove = BuildDigitalMove();

	// 左スティックのアナログ入力を取得する。
	// Input側ですでにデッドゾーン処理済みの値が返る前提で扱う。
	CalyxEngine::Vector2 analogMove = Input::GetLeftStick();

	// デジタル入力とアナログ入力を合成する。
	// 片方だけでも動けるようにし、同時入力時は後段で長さを1に制限する。
	state_.move = ClampMoveLength(digitalMove + analogMove);

	// 向く方向を取得
	CalyxEngine::Vector2 analogLook = Input::GetRightStick();
	state_.look = ClampMoveLength(analogLook);

	// マウスカーソルのスクリーン座標を保持する。
	state_.aimScreen = Input::GetMousePosition();
	// 狙いデバイスの選択（最後に使ったデバイスを優先する）
	constexpr float kAimThresholdSq = 0.04f;
	if (state_.look.LengthSquared() > kAimThresholdSq || analogMove.LengthSquared() > kAimThresholdSq) {
		aimWithMouse_ = false;
	} else if (Input::GetMouseDelta().LengthSquared() > 0.0f) {
		aimWithMouse_ = true;
	}
	state_.aimWithMouse = aimWithMouse_;

	// ジャンプは押された瞬間だけを状態に残す。
	// CharacterMovementComponent側で接地中かどうかを判定するため、ここでは入力事実だけを扱う。
	//state_.jumpPressed = IsTriggerAction(InputAction::Jump) || IsTriggerGamepadAction(InputAction::Jump);

	// 攻撃も押された瞬間だけを状態に残す。
	// 現時点ではDemoPlayer側で未使用だが、入力責務として状態を作っておく。
	state_.attackPressed = IsTriggerAction(InputAction::Attack) || IsTriggerGamepadAction(InputAction::Attack) || Input::TriggerMouseButton(CalyxFoundation::MouseButton::Left);

	// 回避を押した時
	state_.dodgePressed = IsTriggerAction(InputAction::Dash) || IsTriggerGamepadAction(InputAction::Dash) || Input::TriggerMouseButton(CalyxFoundation::MouseButton::Right);

	// クローン生成を押した時
	state_.cloneAbilityPressed = IsTriggerAction(InputAction::Ability) || IsTriggerGamepadAction(InputAction::Ability);
	state_.cloneAbilityHeld = IsPushAction(InputAction::Ability) || IsPushGamepadAction(InputAction::Ability);
	state_.cloneAbilityReleased = prevCloneAbilityHeld_ && !state_.cloneAbilityHeld;
	prevCloneAbilityHeld_ = state_.cloneAbilityHeld;

	state_.lockOnPressed = IsTriggerAction(InputAction::LockOn) || IsTriggerGamepadAction(InputAction::LockOn);
	state_.unlockPressed = IsTriggerAction(InputAction::UnlockLockOn) || IsTriggerGamepadAction(InputAction::UnlockLockOn);
	
	const bool leftTriggerPressed = ToTriggerPress(Input::GetLeftTrigger(), prevLeftTriggerHeld_);
	const bool rightTriggerPressed = ToTriggerPress(Input::GetRightTrigger(), prevRightTriggerHeld_);
	state_.switchLeftPressed = IsTriggerAction(InputAction::SwitchLockOnLeft) || leftTriggerPressed;
	state_.switchRightPressed = IsTriggerAction(InputAction::SwitchLockOnRight) || rightTriggerPressed;

	// ダッシュは押し続け状態として扱う。
	// 移動速度変更などの継続効果で使えるように、トリガーではなくPushを参照する。
	state_.dashHeld = IsPushAction(InputAction::Dash) || IsPushGamepadAction(InputAction::Dash) || Input::PushMouseButton(CalyxFoundation::MouseButton::Right);
}

void PlayerInput::ShowGui() {
}

void PlayerInput::ResetBindings() {
	keyboardBindings_ = {
		{InputAction::MoveForward, DIK_W},
		{InputAction::MoveBackward, DIK_S},
		{InputAction::MoveLeft, DIK_A},
		{InputAction::MoveRight, DIK_D},
		//{InputAction::Jump, DIK_SPACE},
		{InputAction::Ability, DIK_SPACE},
		{InputAction::Attack, DIK_K},
		{InputAction::Dash, DIK_LSHIFT},
		{InputAction::LockOn, DIK_TAB},
		{InputAction::UnlockLockOn, DIK_R},
		{InputAction::SwitchLockOnLeft, DIK_Q},
		{InputAction::SwitchLockOnRight, DIK_E},
	};

	gamepadBindings_ = {
		{InputAction::MoveForward, PadButton::DPAD_UP},
		{InputAction::MoveBackward, PadButton::DPAD_DOWN},
		{InputAction::MoveLeft, PadButton::DPAD_LEFT},
		{InputAction::MoveRight, PadButton::DPAD_RIGHT},
		//{InputAction::Jump, PadButton::A},
		{InputAction::Attack, PadButton::X},
		{InputAction::Ability, PadButton::B},
		{InputAction::Dash, PadButton::A},
		{InputAction::LockOn, PadButton::LB},
		{InputAction::UnlockLockOn, PadButton::RB},
		/*{InputAction::SwitchLockOnLeft, PadButton::DPAD_LEFT},
		{InputAction::SwitchLockOnRight, PadButton::DPAD_RIGHT},*/
	};
}

CalyxEngine::Vector2 PlayerInput::BuildDigitalMove() const {
	CalyxEngine::Vector2 move{0.0f, 0.0f};

	// 前後入力をY成分へ加算する。
	// 前方向を正、後方向を負として、移動変換側がそのままワールドZへ割り当てられるようにする。
	if (IsPushAction(InputAction::MoveForward) || IsPushGamepadAction(InputAction::MoveForward)) {
		move.y += 1.0f;
	}
	if (IsPushAction(InputAction::MoveBackward) || IsPushGamepadAction(InputAction::MoveBackward)) {
		move.y -= 1.0f;
	}

	// 左右入力をX成分へ加算する。
	// 右方向を正、左方向を負として、入力状態の意味を明確に保つ。
	if (IsPushAction(InputAction::MoveRight) || IsPushGamepadAction(InputAction::MoveRight)) {
		move.x += 1.0f;
	}
	if (IsPushAction(InputAction::MoveLeft) || IsPushGamepadAction(InputAction::MoveLeft)) {
		move.x -= 1.0f;
	}

	return ClampMoveLength(move);
}

CalyxEngine::Vector2 PlayerInput::BuildDigitalLook() const {
	CalyxEngine::Vector2 look{ 0.0f, 0.0f };

	if (Input::PushKey(DIK_UP)) {
		look.y += 1.0f;
	}
	if (Input::PushKey(DIK_DOWN)) {
		look.y -= 1.0f;
	}
	if (Input::PushKey(DIK_RIGHT)) {
		look.x += 1.0f;
	}
	if (Input::PushKey(DIK_LEFT)) {
		look.x -= 1.0f;
	}

	// 斜め入力で長さが1を超えないようにする。
	return ClampMoveLength(look);
}

CalyxEngine::Vector2 PlayerInput::ClampMoveLength(const CalyxEngine::Vector2& move) const {
	// 何も入力されていない場合は、そのままゼロを返す。
	if (move.LengthSquared() <= 0.0f) {
		return {0.0f, 0.0f};
	}

	// 長さが1以下ならアナログ入力の強弱を維持する。
	if (move.LengthSquared() <= 1.0f) {
		return move;
	}

	// 複数入力で長さが1を超えた場合だけ正規化する。
	// 斜め移動やキーボード+スティック同時入力で速度が増えないようにする。
	CalyxEngine::Vector2 clamped = move;
	clamped.Normalize();
	return clamped;
}

bool PlayerInput::ToTriggerPress(float value, bool& prevHeld) {
	// しきい値以上なら押されている状態として扱う。
	const bool held = value >= kTriggerPressThreshold;

	// 前フレームが離れていた時だけ、押した瞬間として返す。
	// これでボタンのTrigger判定と同じ挙動になる。
	const bool pressed = held && !prevHeld;

	prevHeld = held;
	return pressed;
}
