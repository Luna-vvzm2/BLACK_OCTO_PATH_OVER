#include "GameOverScene.h"
#include "TitleScene.h"
#include "PlayScene.h"
#include "GameOverUI.h"
#include "Game.h"

GameOverScene::GameOverScene(Game* game)
	: Scene(game)
	, m_nextScene(NextScene::Respawn)
	, m_inextScene(0)
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
			game->ChangeScene(std::make_unique<PlayScene>(game));
		}
		else if (m_nextScene == NextScene::Title)
		{
			game->ChangeScene(std::make_unique<TitleScene>(game));
		}
		else if (m_nextScene == NextScene::Exit)
		{
			
		}
	}

	else if (input.IsTrigger(Action::UP))
	{
		if (static_cast<int>(m_nextScene) > 0)
		{
			m_inextScene--;
			m_nextScene = static_cast<NextScene>(m_inextScene);
		}
	}

	else if (input.IsTrigger(Action::DOWN))
	{
		if (static_cast<int>(m_nextScene) < 2)
		{
			m_inextScene++;
			m_nextScene = static_cast<NextScene>(m_inextScene);
		}
	}
}


void GameOverScene::Draw()
{
	Renderer* renderer = m_game->GetRenderer();
	if (!renderer) return;

	drawActors(m_UIactors);

	const std::string& debugFont = m_game->GatDebugFont();

	renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.15f), "GameOver", Color(192, 192, 192), debugFont, 96, false);
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