#include "DoomEnemy.h"

#include <Enemy/Movement/HomingMove.h>
#include <Enemy/Attack/PunchAttack.h>
#include <Game/Audio/GameAudio.h>

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor
/////////////////////////////////////////////////////////////////////////////////////////
DoomEnemy::DoomEnemy()
	: BaseEnemy({ "Enemy_02_idle.gltf", "Enemy_02_move.gltf", "Enemy_02_attack.gltf", "", "", "", "" }, "DoomEnemy", statsImpl_) {
}

/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void DoomEnemy::Initialize() {
	statsImpl_.LoadParams();
	characterMovement_.SetMaxWalkSpeed(statsImpl_.moveSpeed);

	SetMovement(std::make_unique<HomingMove>());

	auto attack = std::make_unique<PunchAttack>();
	attack->SetAttackSe(GameAudio::kSeSlimeAttack);
	SetAttack(std::move(attack));
	walk_.Load("playerWalk");
	BaseEnemy::Initialize();

}
