#include "TitleScene.h"
#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "Color.h"
#include "TitleUI.h"



TitleScene::TitleScene(Game* game)
	: Scene(game),
	titlePos(0.0f, 0.0f)
{

}



bool TitleScene::Init() {
	titlePos = { m_game->GetWidth() / 2.0f, m_game->GetHeight() / 10.0f };
	m_type = Type::Title;

	TitleUI* title = new TitleUI(this);
	AddUIActor(title);

	m_isRunning = true;
	return true;
}

void TitleScene::Update(float deltaTime) {

	updateActors(m_UIactors, deltaTime);

}

void TitleScene::Draw() {
	Renderer* renderer = m_game->GetRenderer();
	if (!renderer) return;

	drawActors(m_UIactors);


	const std::string& debugFont = m_game->GatDebugFont();


	renderer->DrawTextC(Vector2d(m_game->GetWidth() / 2.0f, m_game->GetHeight() * 0.8f), "Press [ENTER] or (B) to Start", Color(192, 192, 192), debugFont, 24, false);


#ifdef _DEBUG
	renderer->DrawTextL(Vector2d(m_game->GetWidth() - 150.0f, 0), "TitleScene", Color(255, 64, 0), debugFont, 24, false);

#endif // _DEBUG
}