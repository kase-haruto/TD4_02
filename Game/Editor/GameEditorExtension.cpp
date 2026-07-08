#include <CalyxEngine/EditorExtension.h>
#include <Engine/Graphics/Camera/Base/BaseCamera.h>
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <externals/imgui/imgui.h>

namespace {
    class GameCameraEditor final : public CalyxEditor::IEditorTool {
    public:
        explicit GameCameraEditor(const CalyxEditor::EditorToolContext&) {}

        void OnOpen() override {
            open_ = true;
            SyncFromCamera();
        }

        void Draw() override {
            if (!open_) return;
            ImGui::Begin("Game Camera Editor###Game.CameraEditor", &open_);

            auto* camera = CameraManager::GetMain3d();
            if (!camera) {
                ImGui::TextDisabled("No main camera is available.");
                ImGui::End();
                return;
            }

            bool changed = ImGui::DragFloat3("Position", position_, 0.1f);
            changed |= ImGui::DragFloat3("Rotation", rotation_, 0.01f);
            if (changed) {
                camera->SetCamera(
                    {position_[0], position_[1], position_[2]},
                    {rotation_[0], rotation_[1], rotation_[2]});
                camera->UpdateMatrix();
            }

            ImGui::End();
        }

    private:
        void SyncFromCamera() {
            if (auto* camera = CameraManager::GetMain3d()) {
                const auto& position = camera->GetTranslate();
                const auto& rotation = camera->GetRotate();
                position_[0] = position.x;
                position_[1] = position.y;
                position_[2] = position.z;
                rotation_[0] = rotation.x;
                rotation_[1] = rotation.y;
                rotation_[2] = rotation.z;
            }
        }

        bool open_ = true;
        float position_[3]{};
        float rotation_[3]{};
    };

    CalyxEditor::IEditorTool* CreateGameCameraEditor(
        const CalyxEditor::EditorToolContext& context) {
        return new GameCameraEditor(context);
    }

    void DestroyGameCameraEditor(CalyxEditor::IEditorTool* tool) {
        delete tool;
    }
}

extern "C" __declspec(dllexport) bool RegisterCalyxEditorTools(
    std::uint32_t apiVersion,
    CalyxEditor::IEditorHost* host) {
    if (!host || apiVersion != CalyxEditor::kEditorToolApiVersion) return false;

    CalyxEditor::EditorToolDescriptor descriptor;
    descriptor.id = "Game.CameraEditor";
    descriptor.displayName = "Game Camera Editor";
    descriptor.menuPath = "Game/Camera";
    descriptor.workspaceId = "Game.Camera";
    descriptor.create = &CreateGameCameraEditor;
    descriptor.destroy = &DestroyGameCameraEditor;
    return host->RegisterTool(descriptor);
}