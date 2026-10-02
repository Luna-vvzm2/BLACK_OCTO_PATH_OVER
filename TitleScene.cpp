#include "TitleScene.h"
#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "Color.h"
#include "TitleUI.h"
#include "StageSelectScene.h"
#include <memory>



TitleScene::TitleScene(Game* game)
	: Scene(game),
	titlePos(0.0f, 0.0f), m_settings(game)
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

    if (m_settings.IsOpen()) {
        m_settings.Update();
        return;
    }

    const Input& input = m_game->GetInput();
    if (input.IsTrigger(Action::UP)) m_cursor = (m_cursor + 2) % 3;
    if (input.IsTrigger(Action::DOWN)) m_cursor = (m_cursor + 1) % 3;

    if (input.IsTrigger(Action::ENTER)) {
        if (m_cursor == 0) m_game->ChangeScene(std::make_unique<StageSelectScene>(m_game));
        else if (m_cursor == 1) m_settings.Open();
        else m_game->RequestQuit();
    }
}

void TitleScene::Draw() {
    Renderer* renderer = m_game->GetRenderer();
    if (!renderer) return;

    drawActors(m_UIactors);

    const char* items[] = { "ゲーム開始", "設定画面", "ゲーム終了" };
    const auto& font = m_game->GatDebugFont();
    for (int i = 0; i < 3; ++i) {
        const bool selected = i == m_cursor;
        renderer->DrawTextC(
            Vector2d(m_game->GetWidth() * 0.5f, m_game->GetHeight() * 0.60f + i * 58.0f),
            (selected ? "> " : "  ") + std::string(items[i]) + (selected ? " <" : "  "),
            selected ? Color(255, 220, 110) : Color(225, 225, 225), font, 32, false);
    }

    m_settings.Draw();
}