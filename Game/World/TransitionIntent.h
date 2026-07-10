#pragma once
#include <cstdint>

// エリア遷移で次のシーンのどの出現位置に出るかを保持
class TransitionIntent {
public:
	static TransitionIntent& Get() { static TransitionIntent s; return s; }

	void    Request(int32_t spawnId) { spawnId_ = spawnId; hasPending_ = true; }
	bool    HasPending() const { return hasPending_; }
	int32_t SpawnId() const { return spawnId_; }
	void    Clear() { hasPending_ = false; spawnId_ = -1; }

private:
	TransitionIntent() = default;
	TransitionIntent(const TransitionIntent&) = delete;
	TransitionIntent& operator=(const TransitionIntent&) = delete;

	bool    hasPending_ = false;
	int32_t spawnId_ = -1;
};