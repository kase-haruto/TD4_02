#pragma once

//engine
#include "Engine/Foundation/Reflection/CalyxReflection.h"
#include <Data/Engine/Configs/Scene/Objects/SceneObject/SceneObjectConfig.h>
#include <Engine/Foundation/Math/MathUtil.h>
#include <Engine/Objects/3D/Actor/SceneObject.h>
#include <Engine/Objects/ConfigurableObject/IConfigurable.h>
#include <Engine/Scene/Reference/SceneObjectReference.h>
#include <Engine/Graphics/Camera/3d/Camera3d.h>

// game
#include <Game/Player/Player.h>

/*-----------------------------------------------------------------------------------------
 * CameraPivot
 * - カメラのピボットクラス
 * - メインカメラの親となり、追従機能を持つ
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "CameraPivot")
class CameraPivot final:
	public SceneObject,
	public IConfigurable{
public:
	//==================================================================*//
	//		  public functions
	//==================================================================*//
	CameraPivot();
	~CameraPivot() = default;

	void Initialize()override;
	
	/**
	 * @brief 常時更新
	 * @param dt 
	 */
	void AlwaysUpdate(float dt);
	/**
	 * @brief デバッグGUI表示
	 */
	void ShowGui()override;
	/**
	 * @brief jsonFileからパラメータを適用
	 * @param j 
	 */
	void ApplyConfigFromJson(const nlohmann::json& j)override;
	/**
	 * @brief パラメータをjsonに排出
	 * @param j 
	 */
	void ExtractConfigToJson(nlohmann::json& j)const override;

private:
	//==================================================================*//
	//		  private functions
	//==================================================================*//
	void ApplyConfig(const SceneObjectConfig& config);
	SceneObjectConfig ExtractConfig() const;
	void UpdateRotationInput(float dt);
	std::shared_ptr<SceneObject> ResolveMainCamera();
	void ApplyPivotTransform(float dt);
	void ApplyCameraTransform();
	/**
	 * @brief targetを注視
	 */
	void LookAtTarget();

	/**
	 * @brief targetを追従する
	 */
	void FollowTarget();

private:
	//==================================================================*//
	//		  private variable
	//==================================================================*//
	std::weak_ptr<Camera3d> mainCamera_;         //< 操作対象メインカメラ

	struct PivotParam :CalyxEngine::SerializableObject {
		PivotParam();

		CalyxEngine::ParamPath GetParamPath() const override;

		// variables
		CalyxEngine::Vector3 pivotLocalOffset_ = { 0.0f, 1.5f, 0.0f }; //< ピボットの
		CalyxEngine::SceneObjectRef<Player> playerRef_; //< 追従対象プレイヤー

	}param_;


};

