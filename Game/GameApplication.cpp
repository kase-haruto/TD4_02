#include "GameApplication.h"

#include <CalyxEngine/SceneRegistry.h>
#include <Demo/Scene/DemoScene/DemoScene.h>
#include <Scene/PlayerTestScene/PlayerTestScene.h>

void GameApplication::RegisterScenes(Calyx::SceneRegistry& registry) {
	registry.AddScene<DemoScene>(0);
	//registry.AddScene<PlayerTestScene>(1);
	registry.SetStartupScene(0);
}

void GameApplication::OnInitialize() {}

void GameApplication::OnUpdate() {}

void GameApplication::OnRender() {}

void GameApplication::OnFinalize() {}
