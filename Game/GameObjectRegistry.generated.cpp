#include <Engine/Objects/3D/Actor/Registry/SceneObjectRegistry.h>

#include <Demo/3D/Actor/DemoCamera/DemoCameraPivot.h>
#include <Demo/3D/Actor/DemoPlayer/DemoPlayer.h>
#include <Demo/3D/Actor/DemoPlayer/Weapon/Weapon.h>

namespace CalyxEngine {
	void RegisterGeneratedGameSceneObjects() {
		SceneObjectRegistry::Get().Register("DemoCameraPivot", "Demo Camera Pivot", ObjectType::GameObject, "UI/Tool/AssetPanel/generic.png", true, false, false, &CreateSceneObject<DemoCameraPivot>);
		SceneObjectRegistry::Get().Register("DemoPlayer", "Demo Player", ObjectType::GameObject, "UI/Tool/AssetPanel/generic.png", true, false, false, &CreateSceneObject<DemoPlayer>);
		SceneObjectRegistry::Get().Register("Weapon", "Weapon", ObjectType::GameObject, "UI/Tool/AssetPanel/generic.png", true, false, false, &CreateSceneObject<Weapon>);
	}
}
