#include "Sword.h"

Sword::Sword() :BaseGameObject("PlayerSword.obj") {}
Sword::Sword(const std::string& modelName, std::optional<std::string> objectName) :BaseGameObject(modelName, objectName) {

}
Sword::~Sword() = default;
