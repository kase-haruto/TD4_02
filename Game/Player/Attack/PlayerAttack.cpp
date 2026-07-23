#include "PlayerAttack.h"

#include "../Base/PlayerBase.h"
#include "../Sword/Sword.h"

#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Scene/Context/SceneContext.h>
#include <Engine/Scene/Utility/SceneUtility.h>
#include <externals/imgui/imgui.h>
#include <algorithm>
#include <optional>
#include <string>

PlayerAttack::PlayerAttack(){
	param_.LoadParams();
}

void PlayerAttack::Update(PlayerBase& player, const PlayerInputState& input, float dt) {
	if (isAttacking_) {
		UpdateAttack(player, input, dt);
		return;
	}

	if (repeatPreview_) {
		StartAttack(player, previewAttackIndex_);
		return;
	}

	if (input.attackPressed) {
		StartAttack(player, 0);
	}
}

void PlayerAttack::ShowGui(PlayerBase& player) {
	param_.ShowGui();

	if (!ImGui::CollapsingHeader("Attack Effect Preview", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	const int32_t availableAttackCount = std::clamp(param_.comboCount, 1, kAttackCount);
	previewAttackIndex_ = std::clamp(previewAttackIndex_, 0, availableAttackCount - 1);

	ImGui::Checkbox("Repeat Selected Attack", &repeatPreview_);
	int32_t previewAttackNumber = previewAttackIndex_ + 1;
	if (ImGui::SliderInt("Preview Attack", &previewAttackNumber, 1, availableAttackCount, "Attack %d")) {
		previewAttackIndex_ = previewAttackNumber - 1;
	}
	if (ImGui::Button("Replay Now")) {
		EndAttack();
		StartAttack(player, previewAttackIndex_);
	}

	ImGui::TextDisabled("Effect rotation and offset are configured in AttackRotation.");
}

CalyxEngine::SerializableObject& PlayerAttack::SerializableParam() {
	return param_;
}

void PlayerAttack::StartAttack(PlayerBase& player, int comboIndex) {
	isAttacking_ = true;
	nextAttackReserved_ = false;

	comboIndex_ = comboIndex;
	attackTimer_ = 0.0f;

	const CalyxEngine::Quaternion& playerRotation = player.GetWorldTransform().rotation;

	if (comboIndex_ == 0) {
		player.RequestAnimation(PlayerAnimationID::Attack1);
		const CalyxEngine::Vector3 worldOffset =
			CalyxEngine::Quaternion::RotateVector(param_.attack1Offset_, playerRotation);
		const CalyxEngine::Quaternion worldRotation =
			CalyxEngine::Quaternion::Multiply(playerRotation, param_.attack1Rotation_);
		EffectAPI::PlayFromName(
			"slashEffect",
			player.GetWorldTransform().translation + worldOffset,
			worldRotation);
	} else {
		player.RequestAnimation(PlayerAnimationID::Attack2);
		const CalyxEngine::Vector3 worldOffset =
			CalyxEngine::Quaternion::RotateVector(param_.attack2Offset_, playerRotation);
		const CalyxEngine::Quaternion worldRotation =
			CalyxEngine::Quaternion::Multiply(playerRotation, param_.attack2Rotation_);
		EffectAPI::PlayFromName(
			"slashEffect",
			player.GetWorldTransform().translation + worldOffset,
			worldRotation);
	}

	// TODO:
	// - Swordの攻撃判定をONにする時間を設定
	// - 攻撃SEやエフェクトを出す
}

void PlayerAttack::UpdateAttack(PlayerBase& player, const PlayerInputState& input, float dt) {
	attackTimer_ += dt;
	player.RequestAnimation(comboIndex_ == 0 ? PlayerAnimationID::Attack1 : PlayerAnimationID::Attack2);

	// コンボ受付時間中に攻撃ボタンが押されたら、次段を予約
	if (!repeatPreview_ && input.attackPressed && IsInComboAcceptWindow()) {
		nextAttackReserved_ = true;
	}

	const float duration = GetCurrentAttackDuration();

	if (IsHitboxActive()) {
		CreateAttackHitbox(player);
		UpdateAttackHitbox(player);
	} else {
		RemoveAttackHitbox();
	}

	if (attackTimer_ < duration) {
		return;
	}

	// 攻撃時間が終わったとき、次の攻撃が予約されていれば次段へ
	if (nextAttackReserved_ && comboIndex_ + 1 < param_.comboCount) {
		StartNextCombo(player);
		return;
	}

	if (repeatPreview_) {
		EndAttack();
		StartAttack(player, previewAttackIndex_);
	} else {
		EndAttack();
	}
}

void PlayerAttack::StartNextCombo(PlayerBase& player) {
	StartAttack(player, comboIndex_ + 1);
}

void PlayerAttack::EndAttack() {
	isAttacking_ = false;
	nextAttackReserved_ = false;

	comboIndex_ = 0;
	attackTimer_ = 0.0f;
	RemoveAttackHitbox();
}

bool PlayerAttack::IsInComboAcceptWindow() const {
	return attackTimer_ >= param_.comboAcceptStart &&
		attackTimer_ <= param_.comboAcceptEnd;
}

bool PlayerAttack::BlocksMovement() const {
	return isAttacking_ &&
		attackTimer_ >= param_.movementBlockStart &&
		attackTimer_ <= param_.movementBlockEnd;
}

void PlayerAttack::Reset() {
	RemoveAttackHitbox();
	isAttacking_ = false;
	nextAttackReserved_ = false;
	comboIndex_ = 0;
	attackTimer_ = 0.0f;
}

float PlayerAttack::GetCurrentAttackDuration() const {
	if (comboIndex_ == 1) {
		return param_.attackDuration2;
	}
	if (comboIndex_ == 2) {
		return param_.attackDuration3;
	}
	return param_.attackDuration1;
}

bool PlayerAttack::IsHitboxActive() const {
	return isAttacking_ &&
		attackTimer_ >= param_.hitboxActiveStart &&
		attackTimer_ <= param_.hitboxActiveEnd;
}

void PlayerAttack::CreateAttackHitbox(PlayerBase& player) {
	if (attackHitbox_) {
		return;
	}

	attackHitbox_ = SceneAPI::Instantiate<Sword>(
		std::string("PlayerSword.obj"),
		std::optional<std::string>("PlayerAttackHitbox"));
	if (attackHitbox_) {
		attackHitbox_->ConfigureAsAttackHitbox(param_.hitboxSize, param_.drawHitbox);
		attackHitbox_->SetDamage(param_.damage);
		UpdateAttackHitbox(player);
	}
}

void PlayerAttack::UpdateAttackHitbox(PlayerBase& player) {
	if (!attackHitbox_) {
		return;
	}

	CalyxEngine::Vector3 forward =
		CalyxEngine::Quaternion::RotateVector(
			CalyxEngine::Vector3::Forward(),
			player.GetWorldTransform().rotation);
	forward.y = 0.0f;
	forward = forward.LengthSquared() > 1.0e-6f ? forward.Normalize() : CalyxEngine::Vector3::Forward();

	const CalyxEngine::Vector3 position =
		player.GetWorldTransform().translation +
		forward * param_.hitboxForwardOffset +
		CalyxEngine::Vector3(0.0f, param_.hitboxHeightOffset, 0.0f);

	attackHitbox_->SetTranslate(position);
	attackHitbox_->SetRotate(player.GetWorldTransform().rotation);
}

void PlayerAttack::RemoveAttackHitbox() {
	if (!attackHitbox_) {
		return;
	}

	if (auto* context = SceneContext::Current()) {
		context->RemoveObject(std::static_pointer_cast<SceneObject>(attackHitbox_));
	}
	attackHitbox_.reset();
}
