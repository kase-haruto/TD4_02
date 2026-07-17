#include "CameraPivot.h"

#include <Game/Battle/LockOn/ILockOnStateReader.h>
#include <Game/Player/Player.h>

#include <Engine/Foundation/Math/Quaternion.h>
#include <Engine/Foundation/Math/MathUtil.h>
#include <Engine/System/Command/EditorCommand/GuiCommand/ImGuiHelper/GuiCmd.h>
#include <UI/Panels/InspectorPanel.h>

#include <algorithm>
#include <cmath>

namespace {
	float Clamp(float value, float minValue, float maxValue) {
		if (value < minValue) return minValue;
		if (value > maxValue) return maxValue;
		return value;
	}

	CalyxEngine::Vector3 LerpVec3(
		const CalyxEngine::Vector3& from,
		const CalyxEngine::Vector3& to,
		float t) {
		return from + (to - from) * t;
	}
}

CameraPivot::CameraPivot() {
	SceneObject::SetName("CameraPivot", ObjectType::GameObject);
}

void CameraPivot::Initialize() {
	param_.LoadParams();

	// シーン上で Player の子になっている場合でも、Runtime では Pivot 自身の追従補間で位置を管理する。
	worldTransform_.inheritTranslate = false;
	worldTransform_.inheritRotate = false;
}

////////////////////////////
// カメラ更新
////////////////////////////
void CameraPivot::AlwaysUpdate(float dt) {
	const auto player = ResolveFollowPlayer();
	if (!player) {
		return;
	}

	const ILockOnStateReader& lockOnState = player->GetLockOnStateReader();
	const float clampedDt = Clamp(dt, 0.0f, param_.interpolationDeltaTimeMax);

	UpdateLockOnBlend(lockOnState, clampedDt);

	const auto visibleTargets = player->QueryVisibleLockOnTargets(param_.focus.maxEnemyCount);
	const CalyxEngine::Vector3 targetFocus = ComputeTargetFocus(*player, lockOnState, visibleTargets);
	const CalyxEngine::Vector3 targetPosition = ComputeTargetCameraPosition(targetFocus, lockOnBlend_);

	UpdateSmoothedTransform(targetPosition, targetFocus, clampedDt);
}

////////////////////////////
// デバッグ UI
////////////////////////////
void CameraPivot::ShowGui() {
	if (GuiCmd::BeginSection(CalyxEngine::ParamFilterSection::Object)) {
		worldTransform_.ShowImGui("world");
		GuiCmd::SceneObjectReferenceField("FollowTargetObject", param_.playerRef_);
		GuiCmd::EndSection();
	}
	param_.ShowGui();
}

////////////////////////////
// シーン設定
////////////////////////////
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

void CameraPivot::ExtractConfigToJson(nlohmann::json& j) const {
	j = ExtractConfig();
}

SceneObjectConfig CameraPivot::ExtractConfig() const {
	SceneObjectConfig config;
	config.guid = id_;
	config.parentGuid = parentId_;
	config.objectType = static_cast<int>(objectType_);
	config.name = name_;
	config.transform = const_cast<WorldTransform&>(worldTransform_).ExtractConfig();
	return config;
}

////////////////////////////
// 追従対象解決
////////////////////////////
std::shared_ptr<Player> CameraPivot::ResolveFollowPlayer() const {
	if (auto player = param_.playerRef_.Resolve()) {
		return player;
	}
	return std::dynamic_pointer_cast<Player>(GetParent());
}

////////////////////////////
// 注視点の重み付き計算
////////////////////////////
CalyxEngine::Vector3 CameraPivot::ComputeTargetFocus(
	const Player& player,
	const ILockOnStateReader& lockOnState,
	const std::vector<CalyxEngine::TransformRef>& visibleTargets) const {
	const CalyxEngine::Vector3 playerPosition = player.GetWorldPosition();
	CalyxEngine::Vector3 weightedPosition = playerPosition * param_.focus.playerWeight;
	float totalWeight = param_.focus.playerWeight;

	const CalyxEngine::TransformRef& lockedTargetRef = lockOnState.GetCurrentTarget();
	const WorldTransform* lockedTarget = lockOnState.ResolveCurrentTarget();
	if (lockedTarget) {
		weightedPosition += lockedTarget->GetWorldPosition() * param_.focus.lockedTargetWeight;
		totalWeight += param_.focus.lockedTargetWeight;
	}

	CalyxEngine::Vector3 enemyCenter{};
	uint32_t enemyCount = 0;
	for (const CalyxEngine::TransformRef& targetRef : visibleTargets) {
		if (lockedTarget && targetRef.GetGuid() == lockedTargetRef.GetGuid()) {
			continue;
		}

		const WorldTransform* target = targetRef.Resolve();
		if (!target) {
			continue;
		}

		enemyCenter += target->GetWorldPosition();
		++enemyCount;
		if (enemyCount >= param_.focus.maxEnemyCount) {
			break;
		}
	}

	if (enemyCount > 0 && param_.focus.visibleEnemiesWeight > 0.0f) {
		enemyCenter = enemyCenter / static_cast<float>(enemyCount);
		weightedPosition += enemyCenter * param_.focus.visibleEnemiesWeight;
		totalWeight += param_.focus.visibleEnemiesWeight;
	}

	if (totalWeight <= 0.0001f) {
		return playerPosition + CalyxEngine::Vector3{ 0.0f, param_.view.lookAtHeight, 0.0f };
	}

	CalyxEngine::Vector3 targetFocus = weightedPosition / totalWeight;
	targetFocus = ClampFocusOffset(playerPosition, targetFocus);
	targetFocus.y += param_.view.lookAtHeight;
	return targetFocus;
}

////////////////////////////
// ロックオン状態の反映
////////////////////////////
void CameraPivot::UpdateLockOnBlend(const ILockOnStateReader& lockOnState, float dt) {
	const float targetBlend = lockOnState.IsLockingOn() ? 1.0f : 0.0f;
	const float blend = ExpBlend(param_.lockOnTransitionSpeed, dt);
	lockOnBlend_ = Clamp(lockOnBlend_ + (targetBlend - lockOnBlend_) * blend, 0.0f, 1.0f);
}

CalyxEngine::Vector3 CameraPivot::ComputeTargetCameraPosition(
	const CalyxEngine::Vector3& focus,
	float lockBlend) const {
	const float rawDistance = param_.view.baseDistance - param_.lockOnZoomOffset * lockBlend;
	const float distance = rawDistance > 0.1f ? rawDistance : 0.1f;
	const float height = param_.view.baseHeight + param_.lockOnHeightOffset * lockBlend;
	const float yaw = CalyxEngine::ToRadians(param_.view.horizontalAngleDegrees);

	const CalyxEngine::Vector3 offset{
		std::sin(yaw) * distance,
		height,
		std::cos(yaw) * distance
	};
	return focus + offset + param_.view.baseOffset;
}

////////////////////////////
// カメラ状態の補間
////////////////////////////
void CameraPivot::UpdateSmoothedTransform(
	const CalyxEngine::Vector3& targetPosition,
	const CalyxEngine::Vector3& targetFocus,
	float dt) {
	if (!smoothingInitialized_) {
		smoothedPosition_ = targetPosition;
		smoothedFocus_ = targetFocus;
		smoothingInitialized_ = true;
	}

	smoothedPosition_ = LerpVec3(smoothedPosition_, targetPosition, ExpBlend(param_.positionFollowSpeed, dt));
	smoothedFocus_ = LerpVec3(smoothedFocus_, targetFocus, ExpBlend(param_.focusFollowSpeed, dt));

	worldTransform_.translation = smoothedPosition_;
	worldTransform_.rotation = CalyxEngine::Quaternion::LookAt(smoothedPosition_, smoothedFocus_);
}

CalyxEngine::Vector3 CameraPivot::ClampFocusOffset(
	const CalyxEngine::Vector3& playerPosition,
	const CalyxEngine::Vector3& focus) const {
	CalyxEngine::Vector3 offset = focus - playerPosition;
	CalyxEngine::Vector3 horizontal{ offset.x, 0.0f, offset.z };
	const float horizontalLength = horizontal.Length();
	if (horizontalLength > param_.focus.maxHorizontalOffset && horizontalLength > 0.0001f) {
		horizontal = horizontal.Normalize() * param_.focus.maxHorizontalOffset;
		offset.x = horizontal.x;
		offset.z = horizontal.z;
	}

	offset.y = Clamp(offset.y, -param_.focus.maxVerticalOffset, param_.focus.maxVerticalOffset);
	return playerPosition + offset;
}

float CameraPivot::ExpBlend(float speed, float dt) const {
	if (speed <= 0.0f || dt <= 0.0f) {
		return 0.0f;
	}
	return Clamp(1.0f - std::exp(-speed * dt), 0.0f, 1.0f);
}

////////////////////////////
// パラメータ
////////////////////////////
CameraPivot::PivotParam::PivotParam() {
	AddField("playerRef", playerRef_).Tooltip("追従対象プレイヤー").ReadOnly();

	AddField("baseOffset", view.baseOffset).Category("View").Tooltip("角度計算後に加える微調整オフセット");
	AddField("baseDistance", view.baseDistance).Category("View").Tooltip("通常時の水平距離");
	AddField("baseHeight", view.baseHeight).Category("View").Tooltip("通常時の高さ");
	AddField("horizontalAngleDegrees", view.horizontalAngleDegrees).Category("View").Tooltip("水平方向の角度");
	AddField("lookAtHeight", view.lookAtHeight).Category("View").Tooltip("注視点の高さ補正");

	AddField("positionFollowSpeed", positionFollowSpeed).Category("Follow").Tooltip("カメラ位置の1秒あたりの追従速度");
	AddField("focusFollowSpeed", focusFollowSpeed).Category("Follow").Tooltip("注視点の1秒あたりの追従速度");
	AddField("interpolationDeltaTimeMax", interpolationDeltaTimeMax).Category("Follow").Tooltip("補間計算用deltaTime上限");

	AddField("lockOnZoomOffset", lockOnZoomOffset).Category("LockOn").Tooltip("ロックオン中に縮める水平距離");
	AddField("lockOnHeightOffset", lockOnHeightOffset).Category("LockOn").Tooltip("ロックオン中に追加する高さ");
	AddField("lockOnTransitionSpeed", lockOnTransitionSpeed).Category("LockOn").Tooltip("ロックオン状態の1秒あたりの遷移速度");

	AddField("playerFocusWeight", focus.playerWeight).Category("Focus").Tooltip("プレイヤーの注視点ウェイト");
	AddField("lockedTargetFocusWeight", focus.lockedTargetWeight).Category("Focus").Tooltip("ロックオン対象の注視点ウェイト");
	AddField("visibleEnemyFocusWeight", focus.visibleEnemiesWeight).Category("Focus").Tooltip("視野内敵グループの注視点ウェイト");
	AddField("maxHorizontalFocusOffset", focus.maxHorizontalOffset).Category("Focus").Tooltip("水平方向の最大注視点移動量");
	AddField("maxVerticalFocusOffset", focus.maxVerticalOffset).Category("Focus").Tooltip("垂直方向の最大注視点移動量");
	AddField("maxFocusEnemyCount", focus.maxEnemyCount).Category("Focus").Tooltip("注視点計算に使用する敵の最大数");
}

CalyxEngine::ParamPath CameraPivot::PivotParam::GetParamPath() const {
	return { CalyxEngine::ParamDomain::Game, "CameraPivot", "Camera" };
}
