#pragma once

#include "UIActor.h"

class PlayerEntity;

class PlayerStatusUI : public UIActor
{
public:
	PlayerStatusUI(
		Scene* scene,
		PlayerEntity* player,
		float x = 20.0f,
		float y = 160.0f
	);

	~PlayerStatusUI() override = default;

	bool Init() override;
	void Update(float deltaTime) override;
	void Draw() override;

private:
	PlayerEntity* m_player = nullptr;

	float m_x = 20.0f;
	float m_y = 160.0f;
};