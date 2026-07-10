#pragma once
#include <Engine\Objects\3D\Actor\BaseGameObject.h>

/*-----------------------------------------------------------------------------------------
 * Sword
 * - playerの所有する武器
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Sword", Placeable = false, SceneSerializable = false, PrefabSerializable = false)
class Sword
	: public BaseGameObject {
public:
	//====================================================================*/
	//                    public enums
	//====================================================================*/
	enum class HitboxOwner {
		Player, // プレイヤーの攻撃：敵に当たる
		Enemy,  // 敵の攻撃：プレイヤーに当たる
	};
	//====================================================================*/
	//                    public methods
	//====================================================================*/
	Sword();
	Sword(const std::string& modelName, std::optional<std::string> objectName = std::nullopt);
	~Sword()override;
	void ConfigureAsAttackHitbox(
		const CalyxEngine::Vector3& size,
		bool drawCollider,
		HitboxOwner owner = HitboxOwner::Player);

	void  SetKnockbackPower(float power) { knockbackPower_ = power; }
	float GetKnockbackPower() const { return knockbackPower_; }

	void  SetDamage(int damage) { damage_ = damage; }
	int   GetDamage() const { return damage_; }

private:
	//====================================================================*/
	//                    private methods
	//====================================================================*/
	float knockbackPower_ = 0.0f;
	int   damage_ = 0;
};
