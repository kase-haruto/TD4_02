#include <CalyxEngine/CalyxEngine.h>

#include "GameApplication.h"

namespace CalyxEngine {
	void RegisterGeneratedGameSceneObjects();
}

extern "C" __declspec(dllexport) Calyx::Application* CreateCalyxApplication() {
	CalyxEngine::RegisterGeneratedSceneObjects();
	CalyxEngine::RegisterGeneratedGameSceneObjects();

	return new GameApplication();
}

extern "C" __declspec(dllexport) void DestroyCalyxApplication(Calyx::Application* application) {
	delete application;
}
