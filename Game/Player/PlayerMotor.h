#pragma once

#include <Demo/Input/PlayerInput.h>
#include <Engine/Foundation/Math/Vector3.h>
#include <Engine/Foundation/Serialization/SerializableObject.h>

class PlayerBase;

/**
 * \brief Playerの入力をキャラクター移動へ変換するクラス
 * - PlayerInputは入力状態の生成だけを担当する
 * - CharacterMovementComponentは物理移動と床判定だけを担当する
 * - このクラスは入力状態をワールド移動方向とジャンプ命令へ変換する責務を持つ
 */
class PlayerMotor {
public:
	/**
	 * \brief 初期化
	 * \param player プレイヤー
	 */	
	void Initialize(PlayerBase* player);
	/**
	 * \brief 更新
	 * \param player プレイヤー
	 * \param input 入力状態
	 * \param dt 時間差
	 */
	void Update(PlayerBase* player, const PlayerInputState& input, float dt);
	void ShowGui();

	CalyxEngine::Vector3 GetMoveDir() const { return lastMoveDir_; }

private:
	/// 2D入力(x=右, y=前)をカメラ基準のXZ平面移動方向へ変換
	CalyxEngine::Vector3 BuildWorldMoveDirection(const CalyxEngine::Vector2& move) const;

	/// 移動方向へ見た目の向きを合わせる
	void FaceMoveDirection(PlayerBase* player, const CalyxEngine::Vector3& worldDirection) const;

	struct PlayerMoveParam : CalyxEngine::SerializableObject{
		PlayerMoveParam() {
			AddField("moveSpeed", moveSpeed).Category("Movement").Tooltip("移動速度");
			AddField("jumpForce", jumpForce).Category("Movement").Tooltip("ジャンプ力");
		}

		float moveSpeed = 5.0f; //!< 移動速度
		float jumpForce = 5.0f; //!< ジャンプ力
	}param_;

	CalyxEngine::Vector3 lastMoveDir_{};
};