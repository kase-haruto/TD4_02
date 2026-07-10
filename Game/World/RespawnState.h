#include <string>
#include <Engine/Foundation/Math/Vector3.h>

// 最新のアクティブチェックポイント
class RespawnState {
public:
	static RespawnState& Get() { static RespawnState s; return s; }

	void SetCheckpoint(const std::string& scenePath, const CalyxEngine::Vector3& pos) {
		scenePath_ = scenePath; pos_ = pos; has_ = true;
	}
	bool Has() const { return has_; }
	const std::string& ScenePath() const { return scenePath_; }
	const CalyxEngine::Vector3& Position() const { return pos_; }

	void MarkPendingApply() { pendingApply_ = true; }
	bool ConsumePendingApply() { bool p = pendingApply_; pendingApply_ = false; return p; }

	void Clear() { has_ = false; pendingApply_ = false; }

private:
	RespawnState() = default;
	bool has_ = false;
	bool pendingApply_ = false;
	std::string scenePath_;
	CalyxEngine::Vector3 pos_{};
};