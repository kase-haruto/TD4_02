#pragma once

#include <CalyxEngine/Project.h>

#include <externals/nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

/*-----------------------------------------------------------------------------------------
 * ClearTimeRecords
 * - クリアタイムの上位5件を保持し、ファイルへ永続化する
 * - セッション状態(WorldState)と違い、アプリを閉じても残る
 *---------------------------------------------------------------------------------------*/
class ClearTimeRecords {
public:
	static constexpr int kMaxRecords = 5;

	static ClearTimeRecords& Get() {
		static ClearTimeRecords s;
		return s;
	}

	/// 記録をファイルから読む(未読なら読み込む)。以降はメモリ上の値を使う
	void EnsureLoaded() {
		if (loaded_) {
			return;
		}
		loaded_ = true;

		const std::filesystem::path path = FilePath();
		std::ifstream file(path);
		if (!file) {
			return;   // まだ1度もクリアしていない
		}

		// 壊れたファイルや空ファイルで落ちないよう、例外を投げないパースを使う
		const nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
		if (json.is_discarded() || !json.contains("times") || !json["times"].is_array()) {
			return;
		}

		times_.clear();
		for (const auto& value : json["times"]) {
			if (value.is_number()) {
				times_.push_back(value.get<float>());
			}
		}
		SortAndTrim();
	}

	bool Save() const {
		const std::filesystem::path path = FilePath();
		std::error_code ec;
		std::filesystem::create_directories(path.parent_path(), ec);

		std::ofstream file(path);
		if (!file) {
			return false;
		}

		nlohmann::json json;
		json["times"] = times_;
		file << json.dump(2);
		return true;
	}

	/// 上位5件。速い順に並ぶ
	const std::vector<float>& Times() {
		EnsureLoaded();
		return times_;
	}

	/// クリアタイムを登録する。ランクインしたら順位(0始まり)、圏外なら -1
	int Submit(float seconds) {
		EnsureLoaded();

		lastTime_ = seconds;

		// 同着は先に記録済みの方を上位として扱う(upper_bound)
		const auto insertAt = std::upper_bound(times_.begin(), times_.end(), seconds);
		const int  rank = static_cast<int>(std::distance(times_.begin(), insertAt));

		if (rank >= kMaxRecords) {
			lastRank_ = -1;   // 圏外。記録は更新しない
			return lastRank_;
		}

		times_.insert(insertAt, seconds);
		SortAndTrim();
		lastRank_ = rank;
		Save();
		return lastRank_;
	}

	/// 直近のクリアタイム(未クリアなら負値)
	float LastTime() const { return lastTime_; }
	/// 直近のクリアの順位。-1 = 圏外 / 未クリア
	int   LastRank() const { return lastRank_; }
	bool  HasLastResult() const { return lastTime_ >= 0.0f; }

	/// 秒を 分/秒/1/100秒 に分解する(UI表示用)
	static void Split(float seconds, int& outMinutes, int& outSeconds, int& outCentis) {
		const float clamped = seconds > 0.0f ? seconds : 0.0f;
		const int   total = static_cast<int>(clamped * 100.0f + 0.5f);   // 1/100秒単位
		outCentis = total % 100;
		outSeconds = (total / 100) % 60;
		outMinutes = total / 6000;
	}

	/// 記録を全部消す(デバッグ用)
	void ClearAll() {
		EnsureLoaded();
		times_.clear();
		lastTime_ = -1.0f;
		lastRank_ = -1;
		Save();
	}

private:
	ClearTimeRecords() = default;
	ClearTimeRecords(const ClearTimeRecords&) = delete;
	ClearTimeRecords& operator=(const ClearTimeRecords&) = delete;

	static std::filesystem::path FilePath() {
		const std::filesystem::path root = Calyx::GetResourcesRoot();
		const std::filesystem::path relative = "Params/Game/ClearTime/records.json";
		return root.empty() ? relative : root / relative;
	}

	void SortAndTrim() {
		std::sort(times_.begin(), times_.end());
		if (times_.size() > static_cast<size_t>(kMaxRecords)) {
			times_.resize(kMaxRecords);
		}
	}

	bool  loaded_ = false;
	std::vector<float> times_;      //!< 速い順、最大5件
	float lastTime_ = -1.0f;        //!< 直近のクリアタイム
	int   lastRank_ = -1;           //!< 直近の順位(-1=圏外)
};
