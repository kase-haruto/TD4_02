#pragma once

#include <Engine/Objects/3D/Actor/BaseGameObject.h>

/*-----------------------------------------------------------------------------------------
 * ClearUI
 * - ClearUI
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "ClearUI", Icon = "UI/Tool/cube.dds")
class ClearUI : public BaseGameObject {
public:
	ClearUI();
	~ClearUI() override = default;

	void Initialize() override;
	void Update(float dt) override;

private:

};