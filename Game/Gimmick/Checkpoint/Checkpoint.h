#pragma once

#include <Engine/Objects/3D/Actor/BaseGameObject.h>

/*-----------------------------------------------------------------------------------------
 * Checkpoint
 * - プレイヤーが近づくとチェックポイントとして登録される設置物
 *---------------------------------------------------------------------------------------*/
CALYX_OBJECT(Category = GameObject, DisplayName = "Checkpoint", Icon = "UI/Tool/cube.dds")
class Checkpoint : public BaseGameObject {
public:
	Checkpoint();
	~Checkpoint() override = default;

	void Initialize() override;
	void Update(float dt) override;
	void OnCollisionEnter(Collider* other) override;

private:
	void RefreshVisual();

	bool isActiveVisual_ = false;
	bool visualApplied_ = false;
};