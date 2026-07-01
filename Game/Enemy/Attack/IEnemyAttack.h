#pragma once

class BaseEnemy;
class Actor;
namespace CalyxEngine { class SerializableObject; }

class IEnemyAttack {
public:
	virtual ~IEnemyAttack() = default;

	virtual void Update(BaseEnemy& self, const Actor* target, float dt) = 0;
	virtual void ShowGui() {};
	virtual bool IsAttacking() const { return false; }
	virtual CalyxEngine::SerializableObject& SerializableParam() = 0;
};