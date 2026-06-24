#include "BaseEnemy.h"

BaseEnemy::BaseEnemy(const std::string& modelName, const std::string& objectName)
	: Actor(modelName, objectName) {}


/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void BaseEnemy::Initialize() {
	Actor::Initialize();

	currentHp_ = stats_.maxHp;
}

/////////////////////////////////////////////////////////////////////////////////////////
//			更新
/////////////////////////////////////////////////////////////////////////////////////////
void BaseEnemy::Update(float dt) {
	if (movement_ && target_) {
		movement_->Update(*this, target_->GetWorldPosition(), dt);
	}

	// 攻撃は後でここに追加予定

	Actor::Update(dt);
}

void BaseEnemy::SetMovement(std::unique_ptr<IEnemyMovement> movement) {
	movement_ = std::move(movement);
}

void BaseEnemy::TakeDamage(int amount) {
	if (amount <= 0) {
		return;
	}
	currentHp_ -= amount;
	if (currentHp_ < 0) {
		currentHp_ = 0;
	}
	// 死亡時の処理は後で IsDead() を見て呼び出し側で行う
}

