#pragma once
#include <string>
#include <Engine/Foundation/Math/Vector3.h>
#include <Engine/Foundation/Utility/Guid/Guid.h>

// 最新のアクティブチェックポイント
class RespawnState {
public:
	static RespawnState& Get() { static RespawnState s; return s; }

	void SetCheckpoint(const std::string& scenePath, const CalyxEngine::Vector3& pos, const Guid& id) {
		scenePath_ = scenePath; pos_ = pos; activated_ = id; has_ = true;
	}
	bool Has() const { return has_; }
	const std::string& ScenePath() const { return scenePath_; }
	const CalyxEngine::Vector3& Position() const { return pos_; }

	void MarkPendingApply() { pendingApply_ = true; }
	bool ConsumePendingApply() { bool p = pendingApply_; pendingApply_ = false; return p; }

	void MarkJustRespawned() { justRespawned_ = true; }
	bool ConsumeJustRespawned() { bool p = justRespawned_; justRespawned_ = false; return p; }

	void Clear() { has_ = false; pendingApply_ = false; justRespawned_ = false; activated_ = {}; }

	const Guid& ActivateGuid() const { return activated_; }

private:
	RespawnState() = default;
	bool has_ = false;
	bool pendingApply_ = false;
	bool justRespawned_ = false;
	std::string scenePath_;
	CalyxEngine::Vector3 pos_{};
	Guid activated_;
};