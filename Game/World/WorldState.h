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

	//===================================================================*/
	//						プレイヤーHPの持ち越し
	//===================================================================*/
	/// エリア移動をまたいでHPを引き継ぐ。死亡リスポーン時は引き継がない
	bool IsPlayerHpStored() const { return playerHp_ >= 0; }
	int  PlayerHp() const { return playerHp_; }
	void SetPlayerHp(int hp) { playerHp_ = hp; }

	void Clear() {
		activated_.clear();
		playerHp_ = -1;
	}

private:
	WorldState() = default;
	WorldState(const WorldState&) = delete;
	WorldState& operator=(const WorldState&) = delete;

	std::unordered_map<Guid, bool> activated_;
	int playerHp_ = -1; //!< -1 = 未保存
};