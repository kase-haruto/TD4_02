#include <CalyxEngine/EditorExtension.h>

#include "Game/Camera/Editor/GameCameraEditor.h"

#include <filesystem>
#include <fstream>

namespace {

	void WriteEditorExtensionLog(const char* message, std::uint32_t apiVersion = 0) {
		std::filesystem::create_directories("Generated/Logs");
		std::ofstream log("Generated/Logs/EditorExtension.log", std::ios::app);
		if(!log) return;
		log << message;
		if(apiVersion != 0) {
			log << " apiVersion=" << apiVersion
				<< " compiledApiVersion=" << CalyxEditor::kEditorToolApiVersion;
		}
		log << '\n';
	}

}

extern "C" __declspec(dllexport) bool RegisterCalyxEditorTools(
    std::uint32_t apiVersion,
    CalyxEditor::IEditorHost* host) {
    WriteEditorExtensionLog("RegisterCalyxEditorTools called", apiVersion);
    if (!host || apiVersion == 0 || apiVersion > CalyxEditor::kEditorToolApiVersion) {
        WriteEditorExtensionLog("RegisterCalyxEditorTools rejected", apiVersion);
        return false;
    }

    bool ok = true;
    ok &= host->RegisterTool(TD4::MakeGameCameraEditorDescriptor());
    WriteEditorExtensionLog(ok ? "Game Camera Editor registered" : "Game Camera Editor registration failed");

    // Add more game editor descriptors here:
    // ok &= host->RegisterTool(TD4::MakeEnemyEditorDescriptor());
    // ok &= host->RegisterTool(TD4::MakeStageEditorDescriptor());
    return ok;
}
