#pragma once

#include <Enemy/BaseEnemy.h>

/*-----------------------------------------------------------------------------------------
 * Zako
 * - 基本の敵(仮)
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Zako", Icon = "UI/Tool/cube.dds")
class Zako
	: public BaseEnemy {
public:
	Zako();
	~Zako() override = default;

	void Initialize() override;
};