#include "GameCameraEditor.h"

#include <CalyxEngine/Project.h>
#include <Engine/Graphics/Camera/Base/BaseCamera.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <externals/imgui/imgui.h>

namespace TD4 {
namespace {

	class GameCameraEditor final : public CalyxEditor::IEditorTool {
	public:
		explicit GameCameraEditor(const CalyxEditor::EditorToolContext&) {}

		void OnOpen() override {
			open_ = true;
			SyncFromCamera();
		}

		void Draw() override {
			if(!open_) return;

			ImGui::Begin("Game Camera Editor###Game.CameraEditor", &open_);

			

			ImGui::End();
		}

	private:
		void SyncFromCamera() {
			if(auto* camera = CameraManager::GetMain3d()) {
				const auto& position = camera->GetTranslate();
				const auto& rotation = camera->GetRotate();
			}
		}

		bool open_ = true;
	};

	CalyxEditor::IEditorTool* CreateGameCameraEditor(const CalyxEditor::EditorToolContext& context) {
		return new GameCameraEditor(context);
	}

	void DestroyGameCameraEditor(CalyxEditor::IEditorTool* tool) {
		delete tool;
	}

} // namespace

CalyxEditor::EditorToolDescriptor MakeGameCameraEditorDescriptor() {
	CalyxEditor::EditorToolDescriptor descriptor;
	descriptor.id = "Game.CameraEditor";
	descriptor.displayName = "Game Camera Editor";
	descriptor.menuPath = "Game/Camera";
	descriptor.workspaceId = "Game.Camera";
	static const std::string layoutPath =
		Calyx::ResolveAssetPath("Configs/Editor/Layout/GameCameraEditor.ini").generic_string();
	descriptor.layoutPath = layoutPath.c_str();
	descriptor.create = &CreateGameCameraEditor;
	descriptor.destroy = &DestroyGameCameraEditor;
	return descriptor;
}

} // namespace TD4
