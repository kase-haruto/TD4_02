#pragma once

#include <Engine/Foundation/Utility/Guid/Guid.h>
#include <unordered_map>

/*-----------------------------------------------------------------------------------------
 * WorldState
 * - セッション中だけ生きるゲーム状態（シーン遷移では消えない）
 * - タイトルでClear()
 *---------------------------------------------------------------------------------------*/
class WorldState {
public:
	static WorldState& Get() {
		static WorldState instance;
		return instance;
	}

	bool IsActivated(const Guid& id) const {
		auto it = activated_.find(id);
		return it != activated_.end() && it->second;
	}
	void SetActivated(const Guid& id, bool value = true) {
		activated_[id] = value;
	}

	void Clear() { activated_.clear(); }

private:
	WorldState() = default;
	WorldState(const WorldState&) = delete;
	WorldState& operator=(const WorldState&) = delete;

	std::unordered_map<Guid, bool> activated_;
};