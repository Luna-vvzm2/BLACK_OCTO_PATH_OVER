#include "StageSelectScene.h"

#include "Game.h"
#include "PlayScene.h"
#include "Input.h"
#include "Renderer.h"
#include "Color.h"

#include <DxLib.h>
#include <memory>

StageSelectScene::StageSelectScene(Game* game)
    : Scene(game), m_pauseMenu(game, true) {}

bool StageSelectScene::Init()
{
    m_type = Type::StageSelect;
    m_isRunning = true;
    return true;
}

void StageSelectScene::Update(float)
{
    if (m_pauseMenu.IsOpen())
    {
        const PauseMenu::Result result = m_pauseMenu.Update();
        if (result == PauseMenu::Result::Quit) m_game->RequestQuit();
        return;
    }

    const Input& input = m_game->GetInput();
    if (input.IsTrigger(Action::ESCAPE))
    {
        m_pauseMenu.Open();
        return;
    }

    // The current map set represents one playable stage. Later stages do not
    // yet have separate content, so their cards stay locked.
    if (input.IsTrigger(Action::ENTER))
        m_game->ChangeScene(std::make_unique<PlayScene>(m_game));
}

void StageSelectScene::Draw()
{
    Renderer* renderer = m_game->GetRenderer();
    const auto& font = m_game->GatDebugFont();
    const float centerX = m_game->GetWidth() * 0.5f;

    DrawBox(0, 0, m_game->GetWidth(), m_game->GetHeight(), GetColor(20, 26, 36), TRUE);
    renderer->DrawTextC(Vector2d(centerX, 95), "ステージ選択", Color(255, 255, 255), font, 48, false);

    const char* labels[] = { "ステージ 1", "ステージ 2", "ステージ 3" };
    for (int i = 0; i < 3; ++i)
    {
        const float x = 135.0f + i * 345.0f;
        const bool available = i == 0;
        renderer->DrawRect(Vector2d(x, 265), 320, 205,
            available ? Color(55, 92, 85) : Color(55, 55, 60), true, false);
        renderer->DrawTextC(Vector2d(x + 160, 330), labels[i],
            available ? Color(255, 220, 110) : Color(135, 135, 135), font, 32, false);
        renderer->DrawTextC(Vector2d(x + 160, 395), available ? "開始可能" : "未開放",
            available ? Color(255, 255, 255) : Color(135, 135, 135), font, 24, false);
    }

    renderer->DrawTextC(Vector2d(centerX, 560), "Enter／B: ステージ1開始", Color(235, 235, 235), font, 24, false);
    renderer->DrawTextC(Vector2d(centerX, 605), "Esc／START: ポーズ", Color(190, 190, 190), font, 22, false);
    m_pauseMenu.Draw();
}
