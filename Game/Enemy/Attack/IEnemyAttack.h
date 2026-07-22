#pragma once
#include <Engine/Objects/Transform/Transform.h>

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
};