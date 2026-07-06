#include "CameraPivot.h"

// engine
#include <Engine/Graphics/Camera/Manager/CameraManager.h>
#include <UI/Panels/InspectorPanel.h>
#include <Engine/System/Command/EditorCommand/GuiCommand/ImGuiHelper/GuiCmd.h>



/////////////////////////////////////////////////////////////////////////////////////////
//		メインカメラのポインタを返す
/////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<SceneObject> CameraPivot::ResolveMainCamera() {
	auto camera = CameraManager::GetMain3dShared();
	if (camera && camera->GetParent().get() == this) {
		camera->SetParent(nullptr);
	}
	return camera;
}

CameraPivot::CameraPivot(){
	// オブジェクト名とタイプを設定
	SceneObject::SetName("CameraPivot", ObjectType::GameObject);
	
}

void CameraPivot::Initialize(){
	// パラメータをjsonファイルから読み込む
	param_.LoadParams();
}

/////////////////////////////////////////////////////////////////////////////////////////
//		デバッグ表示用のGUIを表示する
/////////////////////////////////////////////////////////////////////////////////////////
void CameraPivot::AlwaysUpdate(float dt){
	(void)dt;
	LookAtTarget();
}

/////////////////////////////////////////////////////////////////////////////////////////
//		targetを注視する
/////////////////////////////////////////////////////////////////////////////////////////
void CameraPivot::LookAtTarget() {
	// 追従対象が有効か
	if(auto target = param_.playerRef_.Resolve()){
		// 追従対象に向けて回転する
		CalyxEngine::Vector3 targetPos = target->GetWorldTransform().GetWorldPosition();
		worldTransform_.rotation = CalyxEngine::Quaternion::LookAt(worldTransform_.GetWorldPosition(), targetPos);
	}
}



/////////////////////////////////////////////////////////////////////////////////////////
//		デバッグ表示用のGUIを表示する
/////////////////////////////////////////////////////////////////////////////////////////
void CameraPivot::ShowGui() {
	// --- トランスフォーム ---
	if (GuiCmd::BeginSection(CalyxEngine::ParamFilterSection::Object)) {
		// worldTransform
		worldTransform_.ShowImGui("world");
		// playerRef
		GuiCmd::SceneObjectReferenceField("FollowTargetObject", param_.playerRef_);
		GuiCmd::EndSection();
	}

}

/////////////////////////////////////////////////////////////////////////////////////////
//		パラメータをjsonファイルから適用
/////////////////////////////////////////////////////////////////////////////////////////
void CameraPivot::ApplyConfigFromJson(const nlohmann::json& j) {
	ApplyConfig(j.get<SceneObjectConfig>());
}

void CameraPivot::ApplyConfig(const SceneObjectConfig& config) {
	id_ = config.guid;
	parentId_ = config.parentGuid;
	objectType_ = static_cast<ObjectType>(config.objectType);
	name_ = config.name.empty() ? "CameraPivot" : config.name;
	worldTransform_.ApplyConfig(config.transform);
}

/////////////////////////////////////////////////////////////////////////////////////////
//		パラメータからjsonファイルに抽出
/////////////////////////////////////////////////////////////////////////////////////////
void CameraPivot::ExtractConfigToJson(nlohmann::json& j) const {
	j = ExtractConfig();
}

SceneObjectConfig CameraPivot::ExtractConfig()const {
	SceneObjectConfig config;
	config.guid = id_;
	config.parentGuid = parentId_;
	config.objectType = static_cast<int>(objectType_);
	config.name = name_;
	config.transform = const_cast<WorldTransform&>(worldTransform_).ExtractConfig();
	return config;
}

/////////////////////////////////////////////////////////////////////////////////////////
//		パラメータ初期化
/////////////////////////////////////////////////////////////////////////////////////////
CameraPivot::PivotParam::PivotParam() {
	AddField("pivotLocalOffset", pivotLocalOffset_).Tooltip("ローカルオフセット");
	AddField("playerRef", playerRef_).Tooltip("追従対象プレイヤー").ReadOnly();
}

/////////////////////////////////////////////////////////////////////////////////////////
//		パラメータ保存パス設定
/////////////////////////////////////////////////////////////////////////////////////////
CalyxEngine::ParamPath CameraPivot::PivotParam::GetParamPath() const {
	return { CalyxEngine::ParamDomain::Game,"CameraPivot","Camera" };
}
