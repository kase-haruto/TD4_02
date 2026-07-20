#pragma once

#include <Engine/Objects/3D/Actor/BaseGameObject.h>
#include <Player/UI/UiSprite.h>
#include <UI/MenuUI.h>

#include <memory>

CALYX_OBJECT(Category = GameObject, DisplayName = "TitleUI", Icon = "UI/Tool/cube.dds")
class TitleUI : public BaseGameObject {
public:
	TitleUI();
	~TitleUI() override = default;

	void Initialize() override;
	void Update(float dt) override;

private:
	void OnStart();
	void OnExit();

	std::shared_ptr<UiSprite> logo_;
	MenuUI                    menu_;
};