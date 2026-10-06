#include "GameOverScene.h"
#include "TitleScene.h"
#include "GameOverUI.h"
#include "Game.h"

GameOverScene::GameOverScene(Game* game)
	: Scene(game)
{
}

bool GameOverScene::Init()
{
	m_type = Type::GameOver;

	GameOverUI* gameOver = new GameOverUI(this);
	AddUIActor(gameOver);

	m_isRunning = true;
	return true;
}

void GameOverScene::Update(float deltaTime)
{
	updateActors(m_UIactors, deltaTime);

}


void GameOverScene::Draw()
{
	Renderer* renderer = m_game->GetRenderer();
	if (!renderer) return;

	drawActors(m_UIactors);

	const std::string& debugFont = m_game->GatDebugFont();

#ifdef _DEBUG
	renderer->DrawTextL(Vector2d(m_game->GetWidth() - 150.0f, 0), "GameOverScene", Color(255, 64, 0), debugFont, 24, false);

#endif // _DEBUG
}