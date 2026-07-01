#pragma once

#include <Enemy/BaseEnemy.h>
#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <cstdint>

/*-----------------------------------------------------------------------------------------
 * ShieldEnemyBase
 * - 盾持ちの基礎
 *---------------------------------------------------------------------------------------*/
class ShieldEnemyBase : public BaseEnemy {
public:
	ShieldEnemyBase(const std::string& modelName, const std::string& objName, EnemyStats& stats);
	~ShieldEnemyBase() override = default;

	void Initialize() override;
	void Update(float dt) override;

protected:
	// 防御中は移動も攻撃も止める
	bool AllowMovement() const override { return !isDefending_; }
	bool AllowAttack()   const override { return !isDefending_; }

	// 前方180°ガード
	void OnHitByPlayerAttack(Collider* attacker) override;

	void DerivativeGui() override;

private:
	void EnterDefense();
	void ExitDefense();
	void RollThreshold();          // attacksUntilDefense_ を抽選
	void FaceTarget();             // 常にプレイヤーを向く
	bool IsBlockedDirection() const;


	struct ShieldParam : CalyxEngine::SerializableObject {
		ShieldParam() {
			AddField("blockCountMin", blockCountMin).Category("Shield").Tooltip("防御に入るまでに食らう攻撃回数(最小)");
			AddField("blockCountMax", blockCountMax).Category("Shield").Tooltip("最大。minと同値なら固定回数");
			AddField("defenseDuration", defenseDuration).Category("Shield").Tooltip("防御を続ける秒数");
			AddField("blockAngleDeg", blockAngleDeg).Category("Shield").Tooltip("ガード可能な角度。180で真横まで");
		}
		CalyxEngine::ParamPath GetParamPath() const override {
			return { CalyxEngine::ParamDomain::Game, "ShieldEnemy", "Actor/Enemy/ShieldParam" };
		}
		int32_t blockCountMin = 3;
		int32_t blockCountMax = 3;
		float   defenseDuration = 2.0f;
		float   blockAngleDeg = 180.0f;
	};

	ShieldParam param_;

	int32_t hitCount_ = 0;           // 食らった回数
	int32_t hitsUntilDefense_ = 3;   // 何回食らったら防御に入るか
	bool    isDefending_ = false;
	float   defenseTimer_ = 0.0f;
};
