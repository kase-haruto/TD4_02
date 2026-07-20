#pragma once

#include <Engine/Objects/3D/Actor/BaseGameObject.h>

/*-----------------------------------------------------------------------------------------
 * TitleUI
 * - TitleUI
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "TitleUI", Icon = "UI/Tool/cube.dds")
class TitleUI : public BaseGameObject {
public:
	TitleUI();
	~TitleUI() override = default;

	void Initialize() override;
	void Update(float dt) override;

private:

	bool isActiveVisual_ = false;
	bool visualApplied_ = false;
};