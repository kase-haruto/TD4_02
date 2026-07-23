#include "HeavySlime.h"

#include <Enemy/Movement/HomingMove.h>
#include <Enemy/Attack/MeleeAttack.h>

/////////////////////////////////////////////////////////////////////////////////////////
//			ctor
/////////////////////////////////////////////////////////////////////////////////////////
HeavySlime::HeavySlime()
	: BaseEnemy({ "Enemy_01_idle.gltf", "Enemy_01_move.gltf", "Enemy_01_attack.gltf", "", "Enemy_01_damage.gltf", "", "" }, "HeavySlime", statsImpl_) {}

/////////////////////////////////////////////////////////////////////////////////////////
//			初期化
/////////////////////////////////////////////////////////////////////////////////////////
void HeavySlime::Initialize() {
	statsImpl_.LoadParams();
	characterMovement_.SetMaxWalkSpeed(statsImpl_.moveSpeed);

	SetMovement(std::make_unique<HomingMove>());
	SetAttack(std::make_unique<MeleeAttack>());
	walk_.Load("playerWalk");
	BaseEnemy::Initialize();
}
