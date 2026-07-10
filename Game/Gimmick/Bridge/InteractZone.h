#pragma once

#include <Engine/Objects/3D/Actor/BaseGameObject.h>

class Player;

/*-----------------------------------------------------------------------------------------
 * InteractZone
 * - 橋起動の検出専用トリガー
 *---------------------------------------------------------------------------------------*/
class InteractZone : public BaseGameObject {
public:
	InteractZone();
	~InteractZone() override = default;

	// size=箱の大きさ, draw=コライダーを可視化するか
	void Configure(const CalyxEngine::Vector3& size, bool draw = false);

	void OnCollisionEnter(Collider* other) override;
	void OnCollisionExit(Collider* other) override;

	bool    IsPlayerInRange() const { return player_ != nullptr; }
	Player* GetPlayer() const { return player_; }

private:
	Player* player_ = nullptr;   // 範囲内のプレイヤー
};