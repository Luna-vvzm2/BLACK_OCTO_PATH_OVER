#include <cstdio>
#include "ClearScene.h"
#include "TitleScene.h"
#include "ClearUI.h"
#include "Game.h"

ClearScene::ClearScene(Game* game, float clearTime)
	: Scene(game)
	, m_clearTime(clearTime)
{
}

bool ClearScene::Init()
{
	m_type = Type::Clear;

	ClearUI* clear = new ClearUI(this);
	AddUIActor(clear);

	m_isRunning = true;
	return true;
}

void ClearScene::Update(float deltaTime)
{
	updateActors(m_UIactors, deltaTime);

}


void ClearScene::Draw() {
	Renderer* renderer = m_game->GetRenderer();
	if (!renderer) return;

	drawActors(m_UIactors);


	const std::string& debugFont = m_game->GatDebugFont();

	int totalSeconds = static_cast<int>(m_clearTime);
	int minutes = totalSeconds / 60;
	int seconds = totalSeconds % 60;

	char buf[32];
	snprintf(buf, sizeof(buf), "Clear Time  %02d:%02d", minutes, seconds);
	std::string timeStr = buf; 

	renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.15f), "STAGE CLEAR", Color(192, 192, 192), debugFont, 96, false);
	renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.7f), timeStr, Color(192, 192, 192), debugFont, 32, false);
	renderer->DrawLine(Vector2d(m_game->GetWidth() * 0.25f, m_game->GetHeight() * 0.75f), Vector2d(m_game->GetWidth() * 0.75f, m_game->GetHeight() * 0.75f), Color(192, 192, 192), false);
	renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.8f), "Press [ENTER] or (B) to Continue", Color(192, 192, 192), debugFont, 24, false);


#ifdef _DEBUG
	renderer->DrawTextL(Vector2d(m_game->GetWidth() - 150.0f, 0), "ClearScene", Color(255, 64, 0), debugFont, 24, false);

#endif // _DEBUG
}