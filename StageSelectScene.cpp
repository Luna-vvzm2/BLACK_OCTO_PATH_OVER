#include "StageSelectScene.h"

#include "Game.h"
#include "PlayScene.h"
#include "TitleScene.h"
#include "Input.h"
#include "Renderer.h"
#include "Color.h"
#include "ScreenMenus.h"

#include <DxLib.h>
#include <memory>

StageSelectScene::StageSelectScene(Game* game, PlayScene* previousPlayScene)
    : Scene(game), m_previousPlayScene(previousPlayScene) {}

bool StageSelectScene::Init()
{
    m_type = Type::StageSelect;
    m_isRunning = true;
    return true;
}

void StageSelectScene::Update(float)
{
    const Input& input = m_game->GetInput();
    if (input.IsTrigger(Action::ESCAPE))
    {
        if (m_game->CanPopScene())
        {
            if (m_previousPlayScene)
                m_previousPlayScene->ReturnToPauseMenu();
            m_game->PopScene();
        }
        else
            m_game->ChangeScene(std::make_unique<TitleScene>(m_game));
        return;
    }

    // The current map set represents one playable stage. Later stages do not
    // yet have separate content, so their cards stay locked.
    if (input.IsTrigger(Action::ENTER)) 
    {
        if (m_game->CanPopScene())
        {

            m_game->PopScene();
        }
        else
        {
            m_game->ChangeScene(std::make_unique<PlayScene>(m_game));

        }
    }
}

void StageSelectScene::Draw()
{
    Renderer* renderer = m_game->GetRenderer();
    const auto& font = m_game->GatDebugFont();
    const float width = static_cast<float>(m_game->GetWidth());
    const float height = static_cast<float>(m_game->GetHeight());

    DrawBox(0, 0, m_game->GetWidth(), m_game->GetHeight(), GetColor(210, 210, 208), TRUE);
    renderer->DrawTextL(Vector2d(width * 0.13f, height * 0.10f),
        "ステージ選択", Color(8, 8, 8), font, 56, false);

    const char* labels[] = { "ステージ1  開始", "ステージ2  未開放", "ステージ3  未開放" };
    for (int i = 0; i < 3; ++i)
        MenuVisual::DrawOption(m_game, labels[i], width * 0.18f,
            height * (0.34f + i * 0.16f), 46, i == 0, i == 0);

    renderer->DrawTextL(Vector2d(width * 0.18f, height * 0.86f),
        "Enter／B: ステージ1開始", Color(35, 35, 35), font, 23, false);
    renderer->DrawTextL(Vector2d(width * 0.18f, height * 0.92f),
        m_game->CanPopScene() ? "Esc／START: ポーズへ戻る" : "Esc／START: タイトルへ戻る",
        Color(35, 35, 35), font, 23, false);
}