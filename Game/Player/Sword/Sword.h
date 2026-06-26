#pragma once
#include <Engine\Objects\3D\Actor\BaseGameObject.h>

/*-----------------------------------------------------------------------------------------
 * Sword
 * - playerの所有する武器
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Sword")
class Sword
	: public BaseGameObject {
public:
	//====================================================================*/
	//                    public methods
	//====================================================================*/
	Sword();
	Sword(const std::string& modelName, std::optional<std::string> objectName = std::nullopt);
	~Sword()override;
	void ConfigureAsAttackHitbox(
		const CalyxEngine::Vector3& size,
		bool drawCollider);

private:
	//====================================================================*/
	//                    private methods
	//====================================================================*/
};
