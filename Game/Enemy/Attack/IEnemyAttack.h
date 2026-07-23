#pragma once
#include <Engine/Objects/Transform/Transform.h>
#include <Game/Audio/GameAudio.h>

#include <string>
#include <utility>

class BaseEnemy;
class Actor;
namespace CalyxEngine { class SerializableObject; }

class IEnemyAttack {
public:
	virtual ~IEnemyAttack() = default;

	virtual void Update(BaseEnemy& self, const Actor* target, float dt) = 0;
	// 攻撃を中断し、出している攻撃判定を破棄する
	virtual void Cancel() {};
	virtual void ShowGui() {};
	virtual bool IsAttacking() const { return false; }
	virtual CalyxEngine::SerializableObject& SerializableParam() = 0;

	// 攻撃の出だしに鳴らすSE。攻撃クラスは敵ごとに使い回すので、敵側のInitializeで設定する
	void SetAttackSe(std::string filename, float volume = 0.3f) {
		attackSe_ = std::move(filename);
		attackSeVolume_ = volume;
	}

protected:
	// 未設定の敵では何も鳴らない
	void PlayAttackSe() const {
		if (attackSe_.empty()) {
			return;
		}
		GameAudio::PlaySe(attackSe_, attackSeVolume_);
	}

private:
	std::string attackSe_;
	float       attackSeVolume_ = 0.4f;
};