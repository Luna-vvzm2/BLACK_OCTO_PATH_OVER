#pragma once
#include "Scene.h"

class ClearScene : public Scene
{
public:
	ClearScene(Game* game, float clearTime);
	~ClearScene() override = default;

	bool Init() override;
	void Update(float deltaTime) override;
	void Draw() override;

private:
	float m_clearTime{ 0.0f };
};