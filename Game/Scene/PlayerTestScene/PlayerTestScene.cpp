#include "PlayerTestScene.h"

// engine
#include <Engine/Foundation/Utility/Func/MyFunc.h>
#include <Engine/Scene/Serializer/SceneSerializer.h>

/////////////////////////////////////////////////////////////////////////////////////////
//	コンストラクタ/デストラクタ
/////////////////////////////////////////////////////////////////////////////////////////
PlayerTestScene::PlayerTestScene() {
	SetSceneName("PlayerTestScene");
}

/////////////////////////////////////////////////////////////////////////////////////////
//	アセットのロード
/////////////////////////////////////////////////////////////////////////////////////////
void PlayerTestScene::LoadAssets() {}

/////////////////////////////////////////////////////////////////////////////////////////
//	初期化処理
/////////////////////////////////////////////////////////////////////////////////////////
void PlayerTestScene::Initialize() {
	sceneContext_->Initialize();
	sceneContext_->SetSceneName("PlayerTestScene");

	BaseScene::Initialize();

	std::string scenePath = "Resources/Assets/Scenes/PlayerTestScene.scene";
	SceneSerializer::Load(*sceneContext_, scenePath);
	sceneContext_->SetScenePath(scenePath);

	LoadAssets();

	player_ = SceneAPI::Instantiate<Player>();
	player_->Initialize();
}

/////////////////////////////////////////////////////////////////////////////////////////
//	更新処理
/////////////////////////////////////////////////////////////////////////////////////////
void PlayerTestScene::Update([[maybe_unused]] float dt) {}

void PlayerTestScene::Draw(ID3D12GraphicsCommandList* cmdList, PipelineService* psoService, IRenderTarget* rt) {

	//========================================================//
	//	spriteの登録
	//========================================================//

	//シーン上のオブジェクトの描画
	BaseScene::Draw(cmdList, psoService, rt);

}

void PlayerTestScene::CleanUp() {
	sceneContext_->GetObjectLibrary()->Clear();
}