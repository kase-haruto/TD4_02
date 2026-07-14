#pragma once

#include <Data/Engine/Configs/Scene/Objects/SceneObject/SceneObjectConfig.h>
#include <Engine/Foundation/Reflection/CalyxReflection.h>
#include <Engine/Foundation/Serialization/SerializableObject.h>
#include <Engine/Objects/3D/Actor/SceneObject.h>
#include <Engine/Objects/ConfigurableObject/IConfigurable.h>
#include <Engine/Scene/Reference/SceneObjectReference.h>
#include <Engine/Scene/Reference/TransformReference.h>

#include <Game/Player/Player.h>

#include <cstdint>
#include <memory>
#include <vector>

class ILockOnStateReader;

/*-----------------------------------------------------------------------------------------
 * CameraPivot
 * - ゲーム固有のメインカメラ追従、注視点計算、ロックオン演出を管理する。
 * - 敵の所有、生存管理、候補判定、生成破棄、投影設定は管理しない。
 * - Engine の Camera3d は子オブジェクトとして扱い、ゲーム側では Pivot の位置と回転だけを更新する。
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "CameraPivot")
class CameraPivot final :
	public SceneObject,
	public IConfigurable {
public:
	CameraPivot();
	~CameraPivot() override = default;

	void Initialize() override;

	/**
	 * @brief プレイヤー、ロックオン対象、視野内敵の情報からカメラ位置と注視点を更新する。
	 * @param dt デルタタイム
	 */
	void AlwaysUpdate(float dt) override;

	/**
	 * @brief デバッグ UI に Transform とゲームカメラ調整値を表示する。
	 */
	void ShowGui() override;

	/**
	 * @brief シーン保存データから Pivot の基本 Transform を復元する。
	 * @param j SceneObjectConfig JSON
	 */
	void ApplyConfigFromJson(const nlohmann::json& j) override;

	/**
	 * @brief Pivot の基本 Transform をシーン保存データへ抽出する。
	 * @param j 出力 JSON
	 */
	void ExtractConfigToJson(nlohmann::json& j) const override;

private:
	struct GameCameraViewSettings {
		CalyxEngine::Vector3 baseOffset = { 0.0f, 0.0f, 0.0f }; //< 角度計算後に加える微調整オフセット
		float baseDistance = 11.0f;                             //< 通常時の水平距離
		float baseHeight = 9.0f;                                //< 通常時の高さ
		float horizontalAngleDegrees = -150.0f;                 //< 水平方向の角度
		float lookAtHeight = 1.4f;                              //< 注視点の高さ補正
	};

	struct GameCameraFocusSettings {
		float playerWeight = 1.0f;           //< プレイヤーの注視点ウェイト
		float lockedTargetWeight = 0.45f;    //< ロックオン対象の注視点ウェイト
		float visibleEnemiesWeight = 0.25f;  //< 視野内敵グループの注視点ウェイト
		float maxHorizontalOffset = 3.0f;    //< 水平方向の最大注視点移動量
		float maxVerticalOffset = 1.5f;      //< 垂直方向の最大注視点移動量
		size_t maxEnemyCount = 4;            //< 使用する敵の最大数
	};

	struct PivotParam : CalyxEngine::SerializableObject {
		PivotParam();

		CalyxEngine::ParamPath GetParamPath() const override;

		CalyxEngine::SceneObjectRef<Player> playerRef_; //< 追従対象プレイヤー
		GameCameraViewSettings view;
		GameCameraFocusSettings focus;
		float positionFollowSpeed = 8.0f;       //< カメラ位置の1秒あたりの追従速度
		float focusFollowSpeed = 10.0f;         //< 注視点の1秒あたりの追従速度
		float lockOnZoomOffset = 2.0f;          //< ロックオン中に縮める水平距離
		float lockOnHeightOffset = 1.6f;        //< ロックオン中に追加する高さ
		float lockOnTransitionSpeed = 7.0f;     //< ロックオン状態の1秒あたりの遷移速度
		float interpolationDeltaTimeMax = 0.1f; //< 補間計算用 deltaTime 上限
	} param_;

	void ApplyConfig(const SceneObjectConfig& config);
	SceneObjectConfig ExtractConfig() const;
	std::shared_ptr<Player> ResolveFollowPlayer() const;

	/**
	 * @brief プレイヤーを最優先に、ロックオン対象と視野内敵グループを重み付きで混ぜた注視点を計算する。
	 */
	CalyxEngine::Vector3 ComputeTargetFocus(
		const Player& player,
		const ILockOnStateReader& lockOnState,
		const std::vector<CalyxEngine::TransformRef>& visibleTargets) const;

	/**
	 * @brief 通常/ロックオン状態を滑らかに混ぜた目標カメラ位置を計算する。
	 */
	CalyxEngine::Vector3 ComputeTargetCameraPosition(
		const CalyxEngine::Vector3& focus,
		float lockBlend) const;

	/**
	 * @brief フレームレート非依存の指数補間で現在位置と注視点を更新する。
	 */
	void UpdateSmoothedTransform(
		const CalyxEngine::Vector3& targetPosition,
		const CalyxEngine::Vector3& targetFocus,
		float dt);

	/**
	 * @brief ロックオン状態を距離、高さ、注視点の遷移に反映する。
	 */
	void UpdateLockOnBlend(const ILockOnStateReader& lockOnState, float dt);

	CalyxEngine::Vector3 ClampFocusOffset(
		const CalyxEngine::Vector3& playerPosition,
		const CalyxEngine::Vector3& focus) const;
	float ExpBlend(float speed, float dt) const;

	CalyxEngine::Vector3 smoothedPosition_{};
	CalyxEngine::Vector3 smoothedFocus_{};
	float lockOnBlend_ = 0.0f;
	bool smoothingInitialized_ = false;
};
