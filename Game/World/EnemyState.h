#pragma once

#include <Engine/Foundation/Utility/Guid/Guid.h>
#include <unordered_map>

/*-----------------------------------------------------------------------------------------
 * EnemyState
 * - セッション中の倒した敵の記録
 * - タイトル/プレイヤー死亡でClear()
 *---------------------------------------------------------------------------------------*/
class EnemyState {
public:
	static EnemyState& Get() {
		static EnemyState instance;
		return instance;
	}

	void MarkDefeated(const Guid& id) { defeated_.insert(id); }
	bool IsDefeated(const Guid& id) const { return defeated_.contains(id); }

	void Clear() { defeated_.clear(); }

private:
	EnemyState() = default;
	std::unordered_set<Guid> defeated_;
};