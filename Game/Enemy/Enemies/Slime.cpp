#include "Slime.h"

#include <Enemy/Movement/HomingMove.h>
#include <Enemy/Attack/MeleeAttack.h>
#include <Game/Audio/GameAudio.h>

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor
/////////////////////////////////////////////////////////////////////////////////////////
Slime::Slime()
	: BaseEnemy({ "Enemy_01_idle.gltf", "Enemy_01_move.gltf", "Enemy_01_attack.gltf", "", "Enemy_01_damage.gltf", "", "" }, "Slime", statsImpl_) {
}

/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void Slime::Initialize() {
	statsImpl_.LoadParams();
	characterMovement_.SetMaxWalkSpeed(statsImpl_.moveSpeed);

	SetMovement(std::make_unique<HomingMove>());

	auto attack = std::make_unique<MeleeAttack>();
	attack->SetAttackSe(GameAudio::kSePigAttack);
	SetAttack(std::move(attack));
	walk_.Load("playerWalk");
	BaseEnemy::Initialize();
}
