#include "PlayerAttack.h"

#include "../Base/PlayerBase.h"

void PlayerAttack::Update(PlayerBase& player, const PlayerInputState& input, float dt) {
	if (isAttacking_) {
		UpdateAttack(player, input, dt);
		return;
	}

	if (input.attackPressed) {
		StartAttack(player, 0);
	}
}

void PlayerAttack::StartAttack(PlayerBase& player, int comboIndex) {
	isAttacking_ = true;
	nextAttackReserved_ = false;

	comboIndex_ = comboIndex;
	attackTimer_ = 0.0f;

	if (comboIndex_ == 0) {
		player.PlayAnimation(PlayerAnimationID::Attack1);
	} else {
		player.PlayAnimation(PlayerAnimationID::Attack2);
	}

	// TODO:
	// - Swordの攻撃判定をONにする時間を設定
	// - 攻撃SEやエフェクトを出す
}

void PlayerAttack::UpdateAttack(PlayerBase& player, const PlayerInputState& input, float dt) {
	attackTimer_ += dt;

	// コンボ受付時間中に攻撃ボタンが押されたら、次段を予約
	if (input.attackPressed && IsInComboAcceptWindow()) {
		nextAttackReserved_ = true;
	}

	float duration = param_.attackDuration1;

	if (comboIndex_ == 1) {
		duration = param_.attackDuration2;
	}
	else if (comboIndex_ == 2) {
		duration = param_.attackDuration3;
	}

	if (attackTimer_ < duration) {
		return;
	}

	// 攻撃時間が終わったとき、次の攻撃が予約されていれば次段へ
	if (nextAttackReserved_ && comboIndex_ + 1 < param_.comboCount) {
		StartNextCombo(player);
		return;
	}

	EndAttack();
}

void PlayerAttack::StartNextCombo(PlayerBase& player) {
	StartAttack(player, comboIndex_ + 1);
}

void PlayerAttack::EndAttack() {
	isAttacking_ = false;
	nextAttackReserved_ = false;

	comboIndex_ = 0;
	attackTimer_ = 0.0f;
}

bool PlayerAttack::IsInComboAcceptWindow() const {
	return attackTimer_ >= param_.comboAcceptStart &&
		attackTimer_ <= param_.comboAcceptEnd;
}
