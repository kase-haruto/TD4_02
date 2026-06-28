#pragma once
#include <Engine/Objects/3D/Actor/Actor.h>
#include <Engine/Application/Effects/EffectAsset.h>
#include <Engine/Scene/Utility/SceneUtility.h>

#include <memory>
#include <string>

#include "EnemyStats.h"
#include "IEnemyMovement.h"

/*-----------------------------------------------------------------------------------------
 * BaseEnemy
 * - すべての敵の基礎
 *---------------------------------------------------------------------------------------*/
class BaseEnemy
	:public Actor {
public:
	//===================================================================*/
	//						public methods
	//===================================================================*/
	BaseEnemy(const std::string& modelName, const std::string& objName, EnemyStats& stats);
	~BaseEnemy() override = default;

	void Initialize() override;
	void Update(float dt) override;

	void OnCollisionEnter([[maybe_unused]] Collider* other)override;

	void SetMovement(std::unique_ptr<IEnemyMovement> movement);
	void SetTarget(const Actor* target) { target_ = target; }

	void TakeDamage(int amount);
	bool IsDead() const { return currentHp_ <= 0; }

	const EnemyStats& GetStats() const { return stats_; }

	void DerivativeGui() override;

protected:
	// 敵ごとの反応
	virtual void OnHitByPlayerAttack(Collider* attacker);
	// プレイヤーが向いている方向へ吹き飛ぶ
	void ApplyKnockbackFrom(Collider* attacker);

	void UpdateKnockback(float dt);

	//===================================================================*/
	//						protected variables
	//===================================================================*/
	EnemyStats& stats_;        // 個体値
	int        currentHp_ = 0;  // 現在HP
	CalyxEngine::Vector3 knockbackVelocity_{};

	std::unique_ptr<IEnemyMovement> movement_;       // 動き
	const Actor* target_ = nullptr;

	//CalyxEngine::EffectAsset hit_;
};
