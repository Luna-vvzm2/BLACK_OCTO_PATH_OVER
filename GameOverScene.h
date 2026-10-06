#pragma once
#include "Scene.h"

class GameOverScene : public Scene
{
public:
	enum class NextScene
	{
		Respawn = 0,
		Title,
		Exit,

		Count
	};

	GameOverScene(Game* game);
	~GameOverScene() override = default;

	bool Init() override;
	void Update(float deltaTime) override;
	void Draw() override;

	NextScene GetNextScene() const { return m_nextScene; }

private:
	NextScene m_nextScene;
};