#pragma once
#include <Engine/Objects/3D/Actor/Actor.h>
#include <Engine/Application/Effects/EffectAsset.h>
#include <Engine/Scene/Utility/SceneUtility.h>
#include <Engine/Scene/Reference/TransformReference.h>
#include <Engine/Scene/Reference/SceneObjectReference.h>
#include <Engine/Application/Effects/EffectAsset.h>
#include <Engine/Application/Effects/EffectPlayer.h>

#include <memory>
#include <string>

#include "EnemyStats.h"
#include "IEnemyMovement.h"
#include "Attack/IEnemyAttack.h"
#include <Game/Battle/LockOn/LockOnTargetComponent.h>

enum class EnemyAnimationID {
	Idle,
	Move,
	Attack,
	Attack2,
	Damage,
	Defence,
	Aim,
};

struct EnemyAnimationSet {
	std::string idle;
	std::string move;
	std::string attack;
	std::string attack2;
	std::string damage;
	std::string defence;
	std::string aim;
};

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
	BaseEnemy(EnemyAnimationSet animations, const std::string& objName, EnemyStats& stats);
	~BaseEnemy() override = default;

	void Initialize() override;
	void Update(float dt) override;

	void OnCollisionEnter([[maybe_unused]] Collider* other)override;

	void SetMovement(std::unique_ptr<IEnemyMovement> movement);
	void SetAttack(std::unique_ptr<IEnemyAttack> attack);

	void TakeDamage(int amount);
	bool IsDead() const { return currentHp_ <= 0; }
	void PlayAnimation(EnemyAnimationID animationId);
	void PlayNextAttackAnimation();

	const EnemyStats& GetStats() const { return stats_; }

	void DerivativeGui() override;
	void Destroy() override;

protected:
	// 敵ごとの反応
	virtual void OnHitByPlayerAttack(Collider* attacker);
	// プレイヤーが向いている方向へ吹き飛ぶ
	void ApplyKnockbackFrom(Collider* attacker);

	void UpdateKnockback(float dt);

	virtual bool AllowMovement() const { return true; }
	virtual bool AllowAttack()   const { return true; }

	//===================================================================*/
	//						protected variables
	//===================================================================*/
	bool pendingRemove_ = false;
	EnemyStats& stats_;        // 個体値
	int        currentHp_ = 0;  // 現在HP
	CalyxEngine::Vector3 knockbackVelocity_{};

	std::unique_ptr<IEnemyMovement> movement_;       // 動き
	std::unique_ptr<IEnemyAttack> attack_;           // 攻撃
	LockOnTargetComponent lockOnTarget_;

	EnemyAnimationSet animations_;
	std::string currentAnimationModel_;
	float damageAnimationTimer_ = 0.0f;
	bool useSecondAttackAnimation_ = false;

	CalyxEngine::EffectAsset hit_;
	CalyxEngine::EffectAsset hitLight_;
	CalyxEngine::EffectAsset walk_;
};
