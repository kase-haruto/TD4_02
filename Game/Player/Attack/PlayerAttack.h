#pragma once

#include <Demo/Input/PlayerInput.h>
#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <memory>

class PlayerBase;
class Sword;

/*-----------------------------------------------------------------------------------------
 * PlayerAttack
 * - 通常攻撃
 * - 将来的にコンボ攻撃へ拡張する
 *---------------------------------------------------------------------------------------*/
class PlayerAttack {
public:
	//===================================================================*/
	//                      public methods
	//===================================================================*/
	PlayerAttack();
	/**
	 * 更新処理
	 */
	void Update(PlayerBase& player, const PlayerInputState& input, float dt);
	void ShowGui(PlayerBase& player);
	CalyxEngine::SerializableObject& SerializableParam();

	/**
	 * 攻撃中か
	 */
	bool IsAttacking() const { return isAttacking_; }

	/**
	 * 移動を止めるか
	 */
	bool BlocksMovement() const;

	void Reset();

private:
	//===================================================================*/
	//                      private methods
	//===================================================================*/

	/**
	 * 攻撃開始
	 */
	void StartAttack(PlayerBase& player, int comboIndex);

	/**
	 * 攻撃中更新
	 */
	void UpdateAttack(PlayerBase& player, const PlayerInputState& input, float dt);

	/**
	 * 次のコンボへ進む
	 */
	void StartNextCombo(PlayerBase& player);

	/**
	 * 攻撃終了
	 */
	void EndAttack();
	float GetCurrentAttackDuration() const;
	bool IsHitboxActive() const;
	void CreateAttackHitbox(PlayerBase& player);
	void UpdateAttackHitbox(PlayerBase& player);
	void RemoveAttackHitbox();

	/**
	 * 次の攻撃入力を受け付ける時間か
	 */
	bool IsInComboAcceptWindow() const;

private:
	//===================================================================*/
	//                      private variables
	//===================================================================*/

	struct AttackParam : CalyxEngine::SerializableObject {
		AttackParam() {
			AddField("comboCount", comboCount)
				.Category("Attack")
				.Tooltip("最大コンボ数");

			AddField("attackDuration1", attackDuration1)
				.Category("Attack")
				.Tooltip("1段目の攻撃時間");

			AddField("attackDuration2", attackDuration2)
				.Category("Attack")
				.Tooltip("2段目の攻撃時間");

			AddField("attackDuration3", attackDuration3)
				.Category("Attack")
				.Tooltip("3段目の攻撃時間");

			AddField("comboAcceptStart", comboAcceptStart)
				.Category("Attack")
				.Tooltip("次の攻撃入力を受け付け始める時間");

			AddField("comboAcceptEnd", comboAcceptEnd)
				.Category("Attack")
				.Tooltip("次の攻撃入力を受け付け終了する時間");

			AddField("movementBlockStart", movementBlockStart)
				.Category("Attack")
				.Tooltip("移動停止を開始する時間");

			AddField("movementBlockEnd", movementBlockEnd)
				.Category("Attack")
				.Tooltip("移動停止を終了する時間");

			AddField("hitboxForwardOffset", hitboxForwardOffset)
				.Category("AttackHitbox")
				.Tooltip("攻撃判定の前方距離");

			AddField("hitboxHeightOffset", hitboxHeightOffset)
				.Category("AttackHitbox")
				.Tooltip("攻撃判定の高さ");

			AddField("hitboxSize", hitboxSize)
				.Category("AttackHitbox")
				.Tooltip("攻撃判定のサイズ");

			AddField("hitboxActiveStart", hitboxActiveStart)
				.Category("AttackHitbox")
				.Tooltip("攻撃判定を出し始める時間");

			AddField("hitboxActiveEnd", hitboxActiveEnd)
				.Category("AttackHitbox")
				.Tooltip("攻撃判定を消す時間");

			AddField("drawHitbox", drawHitbox)
				.Category("AttackHitbox")
				.Tooltip("攻撃判定をデバッグ表示するか");

			AddField("damage", damage)
				.Category("AttackDamage")
				.Tooltip("攻撃力");

			AddField("attack1Offset_", attack1Offset_)
				.Category("AttackEffect1")
				.Tooltip("1段目の攻撃エフェクト位置オフセット");
			AddField("attack1Rotation_", attack1Rotation_)
				.Category("AttackEffect1")
				.Tooltip("1段目の攻撃エフェクト回転（ラジアン）");
			AddField("attack2Offset_", attack2Offset_)
				.Category("AttackEffect2")
				.Tooltip("2段目の攻撃エフェクト位置オフセット");
			AddField("attack2Rotation_", attack2Rotation_)
				.Category("AttackEffect2")
				.Tooltip("2段目の攻撃エフェクト回転（ラジアン）");
			AddField("attack3Offset_", attack3Offset_)
				.Category("AttackEffect3")
				.Tooltip("3段目の攻撃エフェクト位置オフセット");
			AddField("attack3Rotation_", attack3Rotation_)
				.Category("AttackEffect3")
				.Tooltip("3段目の攻撃エフェクト回転（ラジアン）");
		}

		CalyxEngine::ParamPath GetParamPath() const override {
			return {CalyxEngine::ParamDomain::Game, "PlayerAttack", "Actor/Player/AttackParam"};
		}

		int32_t comboCount = 3;

		float attackDuration1 = 0.40f;
		float attackDuration2 = 0.45f;
		float attackDuration3 = 0.60f;

		float comboAcceptStart = 0.18f;
		float comboAcceptEnd = 0.35f;

		float movementBlockStart = 0.0f;
		float movementBlockEnd = 0.35f;

		float hitboxForwardOffset = 1.35f;
		float hitboxHeightOffset = 1.0f;
		CalyxEngine::Vector3 hitboxSize = {0.8f, 0.7f, 0.9f};
		float hitboxActiveStart = 0.05f;
		float hitboxActiveEnd = 0.25f;
		bool drawHitbox = true;

		float damage = 10;

		CalyxEngine::Vector3 attack1Offset_ = {2.0f, 2.5f, 1.0f};
		CalyxEngine::Vector3 attack1Rotation_ = {-2.49f, -0.70f, 2.596f};
		CalyxEngine::Vector3 attack2Offset_ = {};
		CalyxEngine::Vector3 attack2Rotation_ = {};
		CalyxEngine::Vector3 attack3Offset_ = {};
		CalyxEngine::Vector3 attack3Rotation_ = {};
	};

	AttackParam param_;

	bool isAttacking_ = false;
	bool nextAttackReserved_ = false;

	int32_t comboIndex_ = 0;
	float attackTimer_ = 0.0f;
	std::shared_ptr<Sword> attackHitbox_;

	static constexpr int32_t kAttackCount = 3;
	bool repeatPreview_ = false;
	int32_t previewAttackIndex_ = 0;
};
