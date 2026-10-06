#pragma once
#include "Scene.h"

class GameOverScene : public Scene
{
public:
	GameOverScene(Game* game);
	~GameOverScene() override = default;
	bool Init() override;
	void Update(float deltaTime) override;
	void Draw() override;

private:
	
};