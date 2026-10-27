#pragma once

#include "UIActor.h"

class PlayerEntity;

class PlayerStatusUI : public UIActor
{
public:
	PlayerStatusUI(
		Scene* scene,
		PlayerEntity* player,
		float x = 18.0f,
		float y = 135.0f
	);

	~PlayerStatusUI() override;

	bool Init() override;
	void Update(float deltaTime) override;
	void Draw() override;

private:
	PlayerEntity* m_player = nullptr;

	float m_x = 18.0f;
	float m_y = 135.0f;

	int m_jutsuIconHandle = -1;
	int m_healIconHandle = -1;
};
