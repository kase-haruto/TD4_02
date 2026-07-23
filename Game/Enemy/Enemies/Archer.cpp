#include "Archer.h"

#include <Enemy/Movement/KiteMove.h>
#include <Enemy/Attack/RangedAttack.h>
#include <Game/Audio/GameAudio.h>

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor
/////////////////////////////////////////////////////////////////////////////////////////
Archer::Archer()
	: BaseEnemy({ "Enemy_06_idle.gltf", "", "", "", "", "", "Enemy_06_aim.gltf" }, "Archer", statsImpl_) {
}

/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void Archer::Initialize() {
	statsImpl_.LoadParams();
	characterMovement_.SetMaxWalkSpeed(statsImpl_.moveSpeed);

	SetMovement(std::make_unique<KiteMove>());

	auto attack = std::make_unique<RangedAttack>();
	attack->SetAttackSe(GameAudio::kSeArrowShot);
	SetAttack(std::move(attack));
	BaseEnemy::Initialize();

}
