#include "ShieldEnemyBase.h"

#include <Enemy/Attack/IEnemyAttack.h>

#include <Engine/Objects/3D/Actor/Actor.h>
#include <Engine/Objects/Collider/Collider.h>
#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Foundation/Math/MathUtil.h>
#include <Engine/Foundation/Utility/Random/Random.h>

#include <cmath>

ShieldEnemyBase::ShieldEnemyBase(EnemyAnimationSet animations, const std::string& objName, EnemyStats& stats)
	: BaseEnemy(std::move(animations), objName, stats) {}

void ShieldEnemyBase::Initialize() {
	param_.LoadParams();
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
	if (isDefending_) {
		EnsureDefenceAnimation();
	}
}

void ShieldEnemyBase::OnHitByPlayerAttack(Collider* attacker) {
	if (isDefending_ && IsBlockedDirection(attacker)) {
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
	PlayAnimation(EnemyAnimationID::Defence);
}

void ShieldEnemyBase::ExitDefense() {
	isDefending_ = false;
	RollThreshold();
	PlayAnimation(EnemyAnimationID::Idle);
}

void ShieldEnemyBase::RollThreshold() {
	const int32_t minCount = param_.blockCountMin;
	const int32_t maxCount = param_.blockCountMax;
	hitsUntilDefense_ = (maxCount <= minCount) ? minCount : Random::Generate<int32_t>(minCount, maxCount);
}

void ShieldEnemyBase::FaceTarget() {
	auto targetPlayer = stats_.target.Resolve().get();
	if (!targetPlayer) {
		return;
	}
	CalyxEngine::Vector3 toTarget = targetPlayer->GetWorldPosition() - GetWorldPosition();
	toTarget.y = 0.0f;
	if (toTarget.LengthSquared() < 1.0e-6f) {
		return;
	}
	GetWorldTransform().rotation = CalyxEngine::Quaternion::FromToQuaternion(
			CalyxEngine::Vector3::Forward(), toTarget.Normalize());
}

void ShieldEnemyBase::EnsureDefenceAnimation() {
	if (animations_.defence.empty()) {
		return;
	}

	// 被弾アニメなどで別モデルに変わっていたら防御へ戻す。
	// 同じモデルなら PlayAnimation 側の名前ガードで何もしないので、再生位置は維持される
	PlayAnimation(EnemyAnimationID::Defence);
}

bool ShieldEnemyBase::IsBlockedDirection(Collider* attacker) const {
	if (!attacker) {
		return false;
	}
	// プレイヤーではなく「当たったヒットボックス」の位置で見る。
	// 防御中は常にプレイヤーを向いているので、プレイヤー基準だと必ず前方になってしまう
	const BaseGameObject* source = attacker->GetOwner();
	if (!source) {
		return false;
	}

	CalyxEngine::Vector3 forward = CalyxEngine::Quaternion::RotateVector(
			CalyxEngine::Vector3::Forward(), GetWorldTransform().rotation);
	forward.y = 0.0f;
	if (forward.LengthSquared() < 1.0e-6f) {
		return false;
	}

	// ボーン追従の武器は render 側の transform でないと正しい位置が取れない
	CalyxEngine::Vector3 toAttacker = source->GetRenderWorldTransform().translation - GetWorldPosition();
	toAttacker.y = 0.0f;

	if (toAttacker.LengthSquared() < 1.0e-6f) {
		// ヒットボックスが自分の中心に重なっている時は、攻撃側の向きの逆＝発生源の方向とみなす
		CalyxEngine::Vector3 attackDir = CalyxEngine::Quaternion::RotateVector(
				CalyxEngine::Vector3::Forward(), source->GetRenderWorldTransform().rotation);
		attackDir.y = 0.0f;
		if (attackDir.LengthSquared() < 1.0e-6f) {
			return false;
		}
		toAttacker = -attackDir;
	}

	// 前方 blockAngleDeg 以内から来た攻撃だけブロック
	const float cosHalf = std::cos(CalyxEngine::ToRadians(param_.blockAngleDeg * 0.5f));
	return CalyxEngine::Vector3::Dot(forward.Normalize(), toAttacker.Normalize()) >= cosHalf;
}
