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

	//===================================================================*/
	//						クリアタイムの計測
	//===================================================================*/
	/// ゲームプレイ中だけ進める。タイトルからの開始時に Clear() で 0 に戻る
	void AddPlayTime(float dt) {
		if (!playTimeStopped_) {
			playTime_ += dt;
		}
	}
	float PlayTime() const { return playTime_; }
	void  StopPlayTime() { playTimeStopped_ = true; }

	/// クリア処理を1回だけ走らせるための取得。初回のみ true
	bool TryMarkCleared() {
		if (cleared_) {
			return false;
		}
		cleared_ = true;
		return true;
	}

	void Clear() {
		activated_.clear();
		playerHp_ = -1;
		playTime_ = 0.0f;
		playTimeStopped_ = false;
		cleared_ = false;
	}

private:
	WorldState() = default;
	WorldState(const WorldState&) = delete;
	WorldState& operator=(const WorldState&) = delete;

	std::unordered_map<Guid, bool> activated_;
	int playerHp_ = -1; //!< -1 = 未保存

	float playTime_ = 0.0f;        //!< タイトルで開始してからのプレイ時間(秒)
	bool  playTimeStopped_ = false;//!< クリア到達で止める
	bool  cleared_ = false;        //!< クリア処理済みか(記録の二重登録よけ)
};