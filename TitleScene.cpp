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
	titlePos(0.0f, 0.0f),
	m_settings(game)
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
    if (input.IsTrigger(Action::UP)) {
        m_cursor = (m_cursor + 2) % 3;
        m_game->GetSoundManager().Play("game_select_mode");
    }
    if (input.IsTrigger(Action::DOWN)) {
        m_cursor = (m_cursor + 1) % 3;
        m_game->GetSoundManager().Play("game_select_mode");
    }

    if (input.IsTrigger(Action::ENTER)) {
        if (m_cursor == 0) m_game->ChangeScene(std::make_unique<StageSelectScene>(m_game));
        else if (m_cursor == 1) {
            m_game->GetSoundManager().Play("select_default");
            m_settings.Open();
        }
        else m_game->RequestQuit();
    }
}

void TitleScene::Draw() {
    Renderer* renderer = m_game->GetRenderer();
    if (!renderer) return;
    if (m_settings.IsOpen()) {
        m_settings.Draw();
        return;
    }

    drawActors(m_UIactors);

	const char* items[] = { "ゲーム開始", "設定変更", "ゲーム終了" };
	const float width = static_cast<float>(m_game->GetWidth());
	const float height = static_cast<float>(m_game->GetHeight());
	for (int i = 0; i < 3; ++i)
		MenuVisual::DrawOption(m_game, items[i], width * 0.132f,
			height * (0.612f + i * 0.119f), static_cast<int>(height * 0.074f),
			i == m_cursor);


#ifdef _DEBUG
	const std::string& debugFont = m_game->GatDebugFont();
	renderer->DrawTextL(Vector2d(m_game->GetWidth() - 150.0f, 0), "TitleScene", Color(255, 64, 0), debugFont, 24, false);
#endif

}