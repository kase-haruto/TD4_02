#pragma once

#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <cstdint>

/*-----------------------------------------------------------------------------------------
 * EnemyStats
 * - 敵のパラメータだけをまとめた構造体
 *---------------------------------------------------------------------------------------*/
struct EnemyStats : CalyxEngine::SerializableObject {
	EnemyStats() {
		AddField("maxHp", maxHp).Category("Status").Tooltip("最大HP");
		AddField("moveSpeed", moveSpeed).Category("Status").Tooltip("移動速度");

		AddField("contactDamage", contactDamage).Category("Damage").Tooltip("接触ダメージ");
		AddField("attackDamage", attackDamage).Category("Damage").Tooltip("攻撃ダメージ");
		AddField("attackRange", attackRange).Category("Damage").Tooltip("攻撃に移る距離");
		AddField("attackInterval", attackInterval).Category("Damage").Tooltip("攻撃間隔");

		AddField("detectionRange", detectionRange).Category("Detection").Tooltip("索敵距離");

		AddField("knockbackInitialSpeed", knockbackInitialSpeed).Category("Knockback").Tooltip("ノックバックの初速。0でノックバックしない");
		AddField("knockbackDamping", knockbackDamping).Category("Knockback").Tooltip("減衰の強さ。大きいほど早く止まる");
		AddField("knockbackStopSpeed", knockbackStopSpeed).Category("Knockback").Tooltip("この速度以下で停止");
	}

	CalyxEngine::ParamPath GetParamPath() const override {
		return { CalyxEngine::ParamDomain::Game, "Enemy", "Actor/Enemy" };
	}

	int32_t   maxHp = 10;    // 最大HP
	float moveSpeed = 2.0f;  // 移動速度

	// --- ダメージ関連 ---
	int32_t   contactDamage = 1;     // 体に触れている間に与えるダメージ
	int32_t   attackDamage = 2;     // 攻撃アクションが当たったときのダメージ
	float attackRange = 1.5f;  // この距離まで近づくと攻撃に移る
	float attackInterval = 2.0f;  // 攻撃の間隔

	// --- 索敵 ---
	float detectionRange = 8.0f;  // プレイヤーを感知して動き出す距離

	// --- ノックバック ---
	float knockbackInitialSpeed = 16.0f;
	float knockbackDamping = 8.0f;
	float knockbackStopSpeed = 0.2f;
};