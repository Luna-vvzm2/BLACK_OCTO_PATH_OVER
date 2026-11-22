#include "ResultScenes.h"
#include "Game.h"
#include "StageSelectScene.h"
#include <algorithm>
#include <cstdio>
#include <memory>

namespace
{
    void ReleaseGraph(int& handle)
    {
        if (handle >= 0) DeleteGraph(handle);
        handle = -1;
    }

    void DrawScreenArt(Game* game, int handle)
    {
        int width = 0, height = 0;
        if (handle < 0 || GetGraphSize(handle, &width, &height) < 0 || width <= 0 || height <= 0) return;
        // Both supplied images use the same 16:9 layout as the game window.
        game->GetRenderer()->DrawSpriteEx(Vector2d(0, 0),
            static_cast<float>(game->GetWidth()) / width,
            static_cast<float>(game->GetHeight()) / height,
            0.0f, handle, true, Vector2d::Zero(), 255, false, false, false);
    }

    int TextSize(Game* game, float fraction)
    {
        return (std::max)(16, static_cast<int>(game->GetHeight() * fraction));
    }
}

ClearResultScene::ClearResultScene(Game* game, float clearTime)
    : Scene(game), m_clearTime((std::max)(0.0f, clearTime))
{
}

ClearResultScene::~ClearResultScene()
{
    ReleaseGraph(m_art);
    ReleaseGraph(m_background);
}

bool ClearResultScene::Init()
{
    m_type = Type::Clear;
    m_art = LoadGraph("assets/images/results/CLEAR.png");
    if (m_art < 0) std::cerr << "Could not load result artwork: CLEAR.png\n";
    m_background = MakeScreen(m_game->GetWidth(), m_game->GetHeight(), FALSE);
    m_isRunning = true;
    return true;
}

void ClearResultScene::Update(float deltaTime)
{
    // Do not let the final dialogue confirmation also dismiss the result.
    if (m_inputDelay > 0.0f) { m_inputDelay -= deltaTime; return; }
    if (m_game->GetInput().IsTrigger(Action::ENTER))
        m_game->ChangeScene(std::make_unique<StageSelectScene>(m_game));
}

void ClearResultScene::Draw()
{
    auto* renderer = m_game->GetRenderer();
    if (!renderer) return;
    // Push this scene above PlayScene. Game draws the paused stage first.
    // Capture once before drawing artwork; never blur the artwork or menu text.
    if (!m_captured)
    {
        m_captured = true;
        if (m_background >= 0 && GetDrawScreenGraph(0, 0,
            m_game->GetWidth(), m_game->GetHeight(), m_background) >= 0)
        {
            if (GraphFilter(m_background, DX_GRAPH_FILTER_GAUSS, 8, 150) < 0)
                std::cerr << "Result background blur unavailable; using captured stage.\n";
        }
        else ReleaseGraph(m_background);
    }
    DrawScreenArt(m_game, m_background);
    renderer->DrawFullScreenFill(Color(255, 255, 255), 65);
    DrawScreenArt(m_game, m_art);
    const float w = static_cast<float>(m_game->GetWidth());
    const float h = static_cast<float>(m_game->GetHeight());
    const auto& font = m_game->GatGameFont();
    if (m_art < 0)
    {
        renderer->DrawFullScreenFill(Color(0, 0, 0), 180);
        renderer->DrawTextC(Vector2d(w * 0.35f, h * 0.4f), "CLEAR", Color(255, 255, 255), font, TextSize(m_game, 0.08f), false);
    }
    char time[64];
    const int seconds = static_cast<int>(m_clearTime);
    std::snprintf(time, sizeof(time), "CLEAR TIME  %02d:%02d", seconds / 60, seconds % 60);
    renderer->DrawTextC(Vector2d(w * 0.32f, h * 0.79f), time, Color(255, 255, 255), font, TextSize(m_game, 0.035f), false);
    renderer->DrawTextC(Vector2d(w * 0.32f, h * 0.88f), "Enter / B：ステージ選択へ", Color(255, 255, 255), font, TextSize(m_game, 0.032f), false);
}

GameOverResultScene::GameOverResultScene(Game* game) : GameOverScene(game) {}
GameOverResultScene::~GameOverResultScene() { ReleaseGraph(m_art); }

bool GameOverResultScene::Init()
{
    // No base Init: it creates the old background UI, which is not needed here.
    m_type = Type::GameOver;
    m_art = LoadGraph("assets/images/results/gameover.png");
    if (m_art < 0) std::cerr << "Could not load result artwork: gameover.png\n";
    m_isRunning = true;
    return true;
}

void GameOverResultScene::Update(float deltaTime)
{
    if (m_inputDelay > 0.0f) { m_inputDelay -= deltaTime; return; }
    // Reuse the existing RePlay / Title / Exit actions, including retry behavior.
    GameOverScene::Update(deltaTime);
}

void GameOverResultScene::Draw()
{
    auto* renderer = m_game->GetRenderer();
    if (!renderer) return;
    renderer->DrawFullScreenFill(Color(0, 0, 0), 255);
    DrawScreenArt(m_game, m_art);
    const float w = static_cast<float>(m_game->GetWidth());
    const float h = static_cast<float>(m_game->GetHeight());
    const auto& font = m_game->GatGameFont();
    if (m_art < 0)
        renderer->DrawTextC(Vector2d(w * 0.5f, h * 0.3f), "GAME OVER", Color(255, 255, 255), font, TextSize(m_game, 0.08f), false);
    const char* labels[] = { "リトライ", "タイトルへ戻る", "終了" };
    for (int i = 0; i < 3; ++i)
    {
        const bool selected = static_cast<int>(GetNextScene()) == i;
        const std::string label = selected ? ">  " + std::string(labels[i]) + "  <" : labels[i];
        renderer->DrawTextC(Vector2d(w * 0.5f, h * (0.71f + 0.075f * i)), label,
            Color(255, 255, 255), font, TextSize(m_game, 0.04f), false);
    }
    renderer->DrawTextC(Vector2d(w * 0.5f, h * 0.95f), "上下：選択   Enter / B：決定",
        Color(255, 255, 255), font, TextSize(m_game, 0.025f), false);
}
