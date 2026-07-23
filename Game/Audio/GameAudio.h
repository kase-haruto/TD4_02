#pragma once

#include <Engine/Scene/Utility/SceneUtility.h>

#include <string>

/*-----------------------------------------------------------------------------------------
 * GameAudio
 * - ゲームで鳴らす音の名前と、BGMの切り替えをまとめたもの
 *---------------------------------------------------------------------------------------*/
namespace GameAudio {

	// BGM
	inline constexpr const char* kBgmTitleClear = "TitleClearBGM.mp3"; //!< タイトルとクリアで共有
	inline constexpr const char* kBgmGame = "GameBGM.mp3";

	// SE
	inline constexpr const char* kSeAttackSwing = "Attack_normal.mp3"; //!< 剣を振った
	inline constexpr const char* kSeAttackHit = "Attack_hit.mp3";      //!< 敵に当たった
	inline constexpr const char* kSeDodge = "Dodge.mp3";
	inline constexpr const char* kSeSpirit = "Spirit.mp3";             //!< 魂(クローン)を出した
	inline constexpr const char* kSeCheckpoint = "CheckPoint.mp3";

	// 敵SE
	inline constexpr const char* kSeArrowShot = "Shot.mp3";
	inline constexpr const char* kSeBatAttack = "BatAttack.mp3";
	inline constexpr const char* kSePigAttack = "PigAttack.mp3";
	inline constexpr const char* kSeSlimeAttack = "SlimeAttack.mp3";
	inline constexpr const char* kSeTurtleAttack = "TurtleAttack.mp3";

	// 既定音量
	inline constexpr float kBgmVolume = 0.3f;
	inline constexpr float kSeVolume = 0.3f;

	namespace Detail {
		inline std::string currentBgm; //!< 今鳴らしているBGM。空なら無し
	}

	inline void PlaySe(const std::string& filename, float volume = kSeVolume) {
		AudioAPI::Play(filename, false, volume);
	}

	/// BGMを切り替える。同じ曲が鳴っているなら何もしない(エリア移動で鳴り直さない)
	inline void PlayBgm(const std::string& filename, float volume = kBgmVolume) {
		if (Detail::currentBgm == filename) {
			return;
		}
		// 一度も再生していない曲を止めようとすると engine 側で assert するので、鳴らした曲だけ止める
		if (!Detail::currentBgm.empty()) {
			AudioAPI::Stop(Detail::currentBgm);
		}
		AudioAPI::Play(filename, true, volume);
		Detail::currentBgm = filename;
	}

	inline void StopBgm() {
		if (Detail::currentBgm.empty()) {
			return;
		}
		AudioAPI::Stop(Detail::currentBgm);
		Detail::currentBgm.clear();
	}
}
