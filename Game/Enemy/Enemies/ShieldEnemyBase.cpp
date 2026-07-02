#include "ShieldEnemyBase.h"

#include <Enemy/Attack/IEnemyAttack.h>

#include <Engine/Objects/3D/Actor/Actor.h>
#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Foundation/Math/MathUtil.h>
#include <Engine/Foundation/Utility/Random/Random.h>

#include <cmath>

ShieldEnemyBase::ShieldEnemyBase(const std::string& modelName, const std::string& objName, EnemyStats& stats)
	: BaseEnemy(modelName, objName, stats) {}

void ShieldEnemyBase::Initialize() {
	param_.LoadParams();
	SerializableParamObjectsMutable().push_back(&param_);
	RollThreshold();
	BaseEnemy::Initialize();
}

void ShieldEnemyBase::Update(float dt) {
	// 防御フェーズ管理（停止中も常にプレイヤーを向く）
	if (isDefending_) {
		FaceTarget();
		defenseTimer_ -= dt;
		if (defenseTimer_ <= 0.0f) {
			ExitDefense();
		}
	}

	BaseEnemy::Update(dt);
}

void ShieldEnemyBase::OnHitByPlayerAttack(Collider* attacker) {
	if (isDefending_ && IsBlockedDirection()) {
		return;   // ガード成功（ダメージもノックバックも無し）
	}
	BaseEnemy::OnHitByPlayerAttack(attacker);

	if (!isDefending_ && !IsDead()) {
		hitCount_++;
		if (hitCount_ >= hitsUntilDefense_) {
			EnterDefense();
		}
	}
}

void ShieldEnemyBase::DerivativeGui() {
	BaseEnemy::DerivativeGui();
	param_.ShowGui();
}

void ShieldEnemyBase::EnterDefense() {
	isDefending_ = true;
	defenseTimer_ = param_.defenseDuration;
	hitCount_ = 0;
}

void ShieldEnemyBase::ExitDefense() {
	isDefending_ = false;
	RollThreshold();
}

void ShieldEnemyBase::RollThreshold() {
	const int32_t minCount = param_.blockCountMin;
	const int32_t maxCount = param_.blockCountMax;
	hitsUntilDefense_ = (maxCount <= minCount) ? minCount : Random::Generate<int32_t>(minCount, maxCount);
}

void ShieldEnemyBase::FaceTarget() {
	if (!target_) {
		return;
	}
	CalyxEngine::Vector3 toTarget = target_->GetWorldPosition() - GetWorldPosition();
	toTarget.y = 0.0f;
	if (toTarget.LengthSquared() < 1.0e-6f) {
		return;
	}
	GetWorldTransform().rotation = CalyxEngine::Quaternion::FromToQuaternion(
			CalyxEngine::Vector3::Forward(), toTarget.Normalize());
}

bool ShieldEnemyBase::IsBlockedDirection() const {
	if (!target_) {
		return false;
	}
	CalyxEngine::Vector3 forward = CalyxEngine::Quaternion::RotateVector(
			CalyxEngine::Vector3::Forward(), GetWorldTransform().rotation);
	forward.y = 0.0f;

	CalyxEngine::Vector3 toTarget = target_->GetWorldPosition() - GetWorldPosition();
	toTarget.y = 0.0f;

	if (forward.LengthSquared() < 1.0e-6f || toTarget.LengthSquared() < 1.0e-6f) {
		return false;
	}

	// 前方180°ならブロック
	const float cosHalf = std::cos(CalyxEngine::ToRadians(param_.blockAngleDeg * 0.5f));
	return CalyxEngine::Vector3::Dot(forward.Normalize(), toTarget.Normalize()) >= cosHalf;
}
