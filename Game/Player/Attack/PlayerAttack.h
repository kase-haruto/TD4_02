#pragma once

#include <Demo/Input/PlayerInput.h>
#include <Engine/Foundation/Serialization/SerializableObject.h>

class PlayerBase;

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

	/**
	 * 更新処理
	 */
	void Update(PlayerBase& player, const PlayerInputState& input, float dt);

	/**
	 * 攻撃中か
	 */
	bool IsAttacking() const { return isAttacking_; }

	/**
	 * 移動を止めるか
	 */
	bool BlocksMovement() const { return isAttacking_; }

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
		}

		int32_t comboCount = 3;

		float attackDuration1 = 0.40f;
		float attackDuration2 = 0.45f;
		float attackDuration3 = 0.60f;

		float comboAcceptStart = 0.18f;
		float comboAcceptEnd = 0.35f;
	};

	AttackParam param_;

	bool isAttacking_ = false;
	bool nextAttackReserved_ = false;

	int32_t comboIndex_ = 0;
	float attackTimer_ = 0.0f;
};