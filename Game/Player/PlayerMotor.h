#pragma once

#include <Demo/Input/PlayerInput.h>
#include <Engine/Foundation/Math/Vector3.h>

class Player;

/**
 * \brief Playerの入力をキャラクター移動へ変換するクラス
 * - PlayerInputは入力状態の生成だけを担当する
 * - CharacterMovementComponentは物理移動と床判定だけを担当する
 * - このクラスは入力状態をワールド移動方向とジャンプ命令へ変換する責務を持つ
 */
class PlayerMotor {
public:
	/// 入力状態に基づいて Player へ移動命令を送る
	void Update(Player& player, const PlayerInputState& input, float dt);

private:
	/// 2D入力(x=右, y=前)をカメラ基準のXZ平面移動方向へ変換
	CalyxEngine::Vector3 BuildWorldMoveDirection(const CalyxEngine::Vector2& move) const;

	/// 移動方向へ見た目の向きを合わせる
	void FaceMoveDirection(Player& player, const CalyxEngine::Vector3& worldDirection) const;
};