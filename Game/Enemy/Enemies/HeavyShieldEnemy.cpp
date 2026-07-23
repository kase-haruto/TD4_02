#include "HeavyShieldEnemy.h"

#include <Enemy/Movement/HomingMove.h>
#include <Enemy/Attack/MeleeAttack.h>
#include <Game/Audio/GameAudio.h>

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor
/////////////////////////////////////////////////////////////////////////////////////////
HeavyShieldEnemy::HeavyShieldEnemy()
	: ShieldEnemyBase({ "Enemy_03_idle.gltf", "", "Enemy_03_attack1.gltf", "Enemy_03_attack2.gltf", "", "Enemy_03_defence.gltf", "" }, "HeavyShieldEnemy", statsImpl_) {
}

/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void HeavyShieldEnemy::Initialize() {
	statsImpl_.LoadParams();
	characterMovement_.SetMaxWalkSpeed(statsImpl_.moveSpeed);

	SetMovement(std::make_unique<HomingMove>());

	auto attack = std::make_unique<MeleeAttack>();
	attack->SetAttackSe(GameAudio::kSeTurtleAttack);
	SetAttack(std::move(attack));
	walk_.Load("playerWalk");
	ShieldEnemyBase::Initialize();
}
