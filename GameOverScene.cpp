#include "GameOverScene.h"
#include "TitleScene.h"
#include "PlayScene.h"
#include "GameOverUI.h"
#include "Game.h"

GameOverScene::GameOverScene(Game* game)
	: Scene(game)
	, m_nextScene(NextScene::Respawn)
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

	Game* game = this->GetGame();
	const Input& input = game->GetInput();
	if (input.IsTrigger(Action::ENTER))
	{
		Game* game = this->GetGame();
		if (m_nextScene == NextScene::Respawn)
		{
			game->PopScene();

		}
		else if (m_nextScene == NextScene::Title)
		{
			game->ChangeScene(std::make_unique<TitleScene>(game));
		}
		else if (m_nextScene == NextScene::Exit)
		{
			game->Quit();
		}
	}

	else if (input.IsTrigger(Action::UP))
	{
		int current = static_cast<int>(m_nextScene);
		if (current > 0)
		{
			m_nextScene = static_cast<NextScene>(current - 1);
		}
	}

	else if (input.IsTrigger(Action::DOWN))
	{
		int current = static_cast<int>(m_nextScene);
		if (current < static_cast<int>(NextScene::Count) - 1)
		{
			m_nextScene = static_cast<NextScene>(current + 1);
		}
	}
}


void GameOverScene::Draw()
{
	Renderer* renderer = m_game->GetRenderer();
	if (!renderer) return;

	drawActors(m_UIactors);

	const std::string& debugFont = m_game->GatDebugFont();

	renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.15f), "GameOver", Color(255, 50, 50), debugFont, 96, false);
	renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.6f), "RePlay", Color(192, 192, 192), debugFont, 32, false);
	renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.7f), "Title", Color(192, 192, 192), debugFont, 32, false);
	renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.8f), "Exit", Color(192, 192, 192), debugFont, 32, false);

	if (m_nextScene == NextScene::Respawn)
	{
		renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.6f), "RePlay", Color(100, 100, 100), debugFont, 32, false);
	}
	else if (m_nextScene == NextScene::Title)
	{
		renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.7f), "Title", Color(100, 100, 100), debugFont, 32, false);
	}
	else if (m_nextScene == NextScene::Exit)
	{
		renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.8f), "Exit", Color(100, 100, 100), debugFont, 32, false);
	}

#ifdef _DEBUG
	renderer->DrawTextL(Vector2d(m_game->GetWidth() - 150.0f, 0), "GameOverScene", Color(255, 64, 0), debugFont, 24, false);

#endif // _DEBUG
}