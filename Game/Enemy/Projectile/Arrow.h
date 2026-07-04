#pragma once

#include <Player/Sword/Sword.h>
#include <Engine/Foundation/Math/Vector3.h>

/*-----------------------------------------------------------------------------------------
 * Arrow
 * - Archer が撃つ矢
 *---------------------------------------------------------------------------------------*/
class Arrow
	: public Sword {
public:
	Arrow();
	Arrow(const std::string& modelName, std::optional<std::string> objectName = std::nullopt);
	~Arrow() override = default;

	// 発射
	void Launch(const CalyxEngine::Vector3& velocity, float lifeTime);

	void Update(float dt) override;
	void OnCollisionEnter(Collider* other) override;

private:
	void RemoveSelf();

private:
	CalyxEngine::Vector3 velocity_{};   // 進行方向
	float lifeTimer_ = 0.0f;            // 残り寿命
	bool  pendingRemove_ = false;
};