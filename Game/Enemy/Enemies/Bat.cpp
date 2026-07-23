#include "Bat.h"

#include <Enemy/Movement/HomingMove.h>
#include <Enemy/Attack/ChargeAttack.h>
#include <Game/Audio/GameAudio.h>

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor
/////////////////////////////////////////////////////////////////////////////////////////
Bat::Bat()
	: BaseEnemy({ "Enemy_05_idle.gltf", "", "Enemy_05_attack.gltf", "", "", "", "Enemy_05_aim.gltf" }, "Bat", statsImpl_) {
}

/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void Bat::Initialize() {
	statsImpl_.LoadParams();
	characterMovement_.SetMaxWalkSpeed(statsImpl_.moveSpeed);

	SetMovement(std::make_unique<HomingMove>());

	auto attack = std::make_unique<ChargeAttack>();
	attack->SetAttackSe(GameAudio::kSeBatAttack);
	SetAttack(std::move(attack));
	BaseEnemy::Initialize();

}
