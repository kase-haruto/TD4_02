#pragma once

#include <Engine/Objects/3D/Actor/BaseGameObject.h>
#include <Player/UI/UiSprite.h>
#include <UI/MenuUI.h>

#include <memory>

CALYX_OBJECT(Category = GameObject, DisplayName = "ClearUI", Icon = "UI/Tool/cube.dds")
class ClearUI : public BaseGameObject {
public:
	ClearUI();
	~ClearUI() override = default;

	void Initialize() override;
	void Update(float dt) override;

private:
	void OnTitle();
	void OnExit();

	std::shared_ptr<UiSprite> logo_;
	MenuUI                    menu_;
};