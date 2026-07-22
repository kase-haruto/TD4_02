#pragma once

#include <Player/UI/UiSprite.h>

#include <Engine/Scene/Utility/SceneUtility.h>
#include <Engine/Scene/Context/SceneContext.h>

#include <memory>
#include <string>
#include <vector>

/*-----------------------------------------------------------------------------------------
 * TimeDisplay
 * - 数字テクスチャ(Textures/Game/Number/0.png 〜 9.png)を並べて時間や数値を表示する
 *---------------------------------------------------------------------------------------*/
class TimeDisplay {
public:
	/// 見た目の設定。1桁のサイズと隙間
	struct Layout {
		float digitW = 28.0f;    //!< 数字1桁の幅
		float digitH = 50.0f;    //!< 数字1桁の高さ
		float digitGap = 4.0f;   //!< 数字どうしの隙間
		float groupGap = 20.0f;  //!< 分/秒/(1/100)秒の区切り幅(記号の代わり)
	};

	TimeDisplay() = default;
	~TimeDisplay() { Clear(); }

	TimeDisplay(const TimeDisplay&) = delete;
	TimeDisplay& operator=(const TimeDisplay&) = delete;

	/// 秒を MM SS CC の並びで作る。leftX は左端、centerY は中心Y
	void BuildTime(float seconds, float leftX, float centerY, const Layout& layout = {}) {
		Clear();
		layout_ = layout;

		int minutes = 0;
		int secs = 0;
		int centis = 0;
		SplitTime(seconds, minutes, secs, centis);

		float x = leftX;
		x = AppendNumber(minutes, 2, x, centerY);
		x += layout_.groupGap;
		x = AppendNumber(secs, 2, x, centerY);
		x += layout_.groupGap;
		x = AppendNumber(centis, 2, x, centerY);

		width_ = x - leftX;
	}

	/// 数値をそのまま作る(順位の表示などに使う)
	void BuildNumber(int value, int minDigits, float leftX, float centerY, const Layout& layout = {}) {
		Clear();
		layout_ = layout;
		width_ = AppendNumber(value, minDigits, leftX, centerY) - leftX;
	}

	void SetColorRGBA(float r, float g, float b, float a = 1.0f) {
		for (auto& digit : digits_) {
			digit->SetColorRGBA(r, g, b, a);
		}
	}

	void SetVisible(bool visible) {
		for (auto& digit : digits_) {
			digit->SetVisible(visible);
		}
	}

	/// 作ったスプライトをシーンから消す
	void Clear() {
		if (auto* context = SceneContext::Current()) {
			for (auto& digit : digits_) {
				context->RemoveObject(std::static_pointer_cast<SceneObject>(digit));
			}
		}
		digits_.clear();
		width_ = 0.0f;
	}

	/// 全体の横幅(px)
	float Width() const { return width_; }

	/// 秒を 分/秒/(1/100)秒 に分解する
	static void SplitTime(float seconds, int& outMinutes, int& outSeconds, int& outCentis) {
		const float clamped = seconds > 0.0f ? seconds : 0.0f;
		const int   total = static_cast<int>(clamped * 100.0f + 0.5f);
		outCentis = total % 100;
		outSeconds = (total / 100) % 60;
		outMinutes = total / 6000;
	}

private:
	/// value を minDigits 桁(ゼロ埋め)で並べ、次に置くべきX座標を返す
	float AppendNumber(int value, int minDigits, float leftX, float centerY) {
		std::string text = std::to_string(value < 0 ? 0 : value);
		while (static_cast<int>(text.size()) < minDigits) {
			text.insert(text.begin(), '0');
		}

		float x = leftX;
		for (size_t i = 0; i < text.size(); ++i) {
			if (i > 0) {
				x += layout_.digitGap;
			}
			AddDigit(text[i] - '0', x, centerY);
			x += layout_.digitW;
		}
		return x;
	}

	void AddDigit(int digit, float leftX, float centerY) {
		const std::string path = "Textures/Numbers/" + std::to_string(digit) + ".png";

		auto sprite = SceneAPI::Instantiate<UiSprite>(path);
		if (!sprite) {
			return;
		}
		sprite->SetAnchor({ 0.0f, 0.5f });
		sprite->SetPositionPx(leftX, centerY);
		sprite->SetSizePx(layout_.digitW, layout_.digitH);
		digits_.push_back(std::move(sprite));
	}

	std::vector<std::shared_ptr<UiSprite>> digits_;
	Layout layout_{};
	float  width_ = 0.0f;
};
