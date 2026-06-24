#pragma once

#include <Enemy/BaseEnemy.h>

/*-----------------------------------------------------------------------------------------
 * Slime
 * - 基本の敵(仮)
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Slime", Icon = "UI/Tool/cube.dds")
class Slime
	: public BaseEnemy {
public:
	Slime();
	~Slime() override = default;

	void Initialize() override;
};