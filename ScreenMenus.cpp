#include "ScreenMenus.h"

#include "Game.h"
#include "SoundComponent.h"
#include "Input.h"
#include "Renderer.h"
#include "Color.h"

#include <DxLib.h>
#include <algorithm>
#include <string>

namespace
{
    void DrawBackdrop(const Game* game, bool overGame, int alpha = 255)
    {
        if (alpha <= 0) return;
        const unsigned int paper = GetColor(210, 210, 208);
        if (!overGame)
        {
            int oldBlend, oldAlpha;
            GetDrawBlendMode(&oldBlend, &oldAlpha);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
            DrawBox(0, 0, game->GetWidth(), game->GetHeight(), paper, TRUE);
            SetDrawBlendMode(oldBlend, oldAlpha);
            return;
        }

        int oldBlend, oldAlpha;
        GetDrawBlendMode(&oldBlend, &oldAlpha);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 135 * alpha / 255);
        DrawBox(0, 0, game->GetWidth(), game->GetHeight(), paper, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 205 * alpha / 255);
        DrawBox(0, 0, game->GetWidth() * 3 / 5, game->GetHeight(), paper, TRUE);
        SetDrawBlendMode(oldBlend, oldAlpha);
    }
}

namespace MenuVisual
{
    void DrawOption(Game* game, const std::string& label, float x, float y,
        int fontSize, bool selected, bool enabled)
    {
        Renderer* renderer = game->GetRenderer();
        const bool active = selected && enabled;
        renderer->DrawTextL(Vector2d(x, y), label,
            active ? Color(200, 35, 35) : (enabled ? Color(8, 8, 8) : Color(125, 125, 125)),
            game->GatDebugFont(), fontSize, false);
        if (!active) return;

        const float scale = static_cast<float>(fontSize) / 53.0f;
        const int markerX = static_cast<int>(x - 85.0f * scale);
        const int markerY = static_cast<int>(y);
        const unsigned int markerColor = GetColor(38, 29, 26);
        DrawCircle(markerX + static_cast<int>(5 * scale), markerY + static_cast<int>(scale),
            static_cast<int>(7 * scale), markerColor, TRUE);
        DrawLine(markerX + static_cast<int>(10 * scale), markerY + static_cast<int>(6 * scale),
            markerX + static_cast<int>(35 * scale), markerY + static_cast<int>(34 * scale),
            markerColor, static_cast<int>(7 * scale));
        DrawTriangle(markerX + static_cast<int>(31 * scale), markerY + static_cast<int>(27 * scale),
            markerX + static_cast<int>(64 * scale), markerY + static_cast<int>(60 * scale),
            markerX + static_cast<int>(37 * scale), markerY + static_cast<int>(48 * scale),
            markerColor, TRUE);
    }
}
SettingsMenu::SettingsMenu(Game* game, bool overGame) : m_game(game), m_overGame(overGame) {}

void SettingsMenu::Open()
{
    m_open = true;
    m_page = Page::Root;
    m_cursor = 0;
}

void SettingsMenu::Update()
{
    if (!m_open) return;
    const Input& input = m_game->GetInput();

    if (input.IsTrigger(Action::ESCAPE))
    {
        if (m_page == Page::Root) m_open = false;
        else { m_page = Page::Root; m_cursor = 0; }
        return;
    }

    const int count = m_page == Page::Volume ? 5 : (m_page == Page::Root ? 3 : 1);
    if (input.IsTrigger(Action::UP)) m_cursor = (m_cursor + count - 1) % count;
    if (input.IsTrigger(Action::DOWN)) m_cursor = (m_cursor + 1) % count;

    if (m_page == Page::Volume && m_cursor < 4)
    {
        const int step = input.IsTrigger(Action::RIGHT) ? 5 : (input.IsTrigger(Action::LEFT) ? -5 : 0);
        if (step != 0)
        {
            if (m_cursor == 3) SoundComponent::AdjustVoiceVolume(step);
            else m_game->AdjustVolume(m_cursor, step);
        }
    }

    if (!input.IsTrigger(Action::ENTER)) return;

    if (m_page == Page::Root)
    {
        if (m_cursor == 0) { m_page = Page::Volume; m_cursor = 0; }
        else if (m_cursor == 1) { m_page = Page::Controls; m_cursor = 0; }
        else m_open = false;
    }
    else if (m_page == Page::Volume && m_cursor == 4)
    {
        m_page = Page::Root;
        m_cursor = 0;
    }
    else if (m_page == Page::Controls)
    {
        m_page = Page::Root;
        m_cursor = 0;
    }
}

void SettingsMenu::Draw() const
{
    if (!m_open) return;
    DrawBackdrop(m_game, m_overGame);
    Renderer* renderer = m_game->GetRenderer();
    const float width = static_cast<float>(m_game->GetWidth());
    const float height = static_cast<float>(m_game->GetHeight());
    const float x = width * 0.18f;
    const auto& font = m_game->GatDebugFont();

    const char* title = m_page == Page::Root ? "設定変更" :
        (m_page == Page::Volume ? "音量設定" : "操作方法");
    renderer->DrawTextL(Vector2d(width * 0.13f, height * 0.10f),
        title, Color(8, 8, 8), font, 56, false);

    if (m_page == Page::Root)
    {
        const char* labels[] = { "音量設定", "操作方法", "戻る" };
        for (int i = 0; i < 3; ++i)
            MenuVisual::DrawOption(m_game, labels[i], x, height * (0.34f + i * 0.145f),
                43, i == m_cursor);
    }
    else if (m_page == Page::Volume)
    {
        const char* labels[] = { "全体音量", "SE", "BGM", "ボイス" };
        for (int i = 0; i < 4; ++i)
        {
            const int volume = i == 3 ? SoundComponent::GetVoiceVolume() : m_game->GetVolume(i);
            MenuVisual::DrawOption(m_game,
                std::string(labels[i]) + "  " + std::to_string(volume) + "%",
                x, height * (0.25f + i * 0.125f), 39, i == m_cursor);
        }
        MenuVisual::DrawOption(m_game, "戻る", x, height * 0.75f, 39, m_cursor == 4);
        renderer->DrawTextL(Vector2d(x, height * 0.89f),
            "左右キー／スティックで5%ずつ変更", Color(35, 35, 35), font, 23, false);
    }
    else
    {
        renderer->DrawTextL(Vector2d(x, height * 0.27f),
            "移動: WASD／方向キー    決定: Enter／B", Color(8, 8, 8), font, 26, false);
        renderer->DrawTextL(Vector2d(x, height * 0.37f),
            "ジャンプ: Space／A    攻撃: I／X", Color(8, 8, 8), font, 26, false);
        renderer->DrawTextL(Vector2d(x, height * 0.47f),
            "ポーズ: Esc／START    メニュー: M／BACK", Color(8, 8, 8), font, 26, false);
        renderer->DrawTextL(Vector2d(x, height * 0.58f),
            "キーの割り当て変更は未対応", Color(70, 70, 70), font, 23, false);
        MenuVisual::DrawOption(m_game, "戻る", x, height * 0.75f, 39, true);
    }
}
PauseMenu::PauseMenu(Game* game, bool fromStageSelect)
    : m_game(game), m_fromStageSelect(fromStageSelect), m_settings(game, true) {}

void PauseMenu::Open()
{
    m_open = true;
    m_cursor = 0;
}

void PauseMenu::Toggle()
{
    m_open = !m_open;
}

PauseMenu::Result PauseMenu::Update()
{
    if (!m_open) return Result::None;

    if (m_settings.IsOpen())
    {
        m_settings.Update();
        return Result::None;
    }

    const Input& input = m_game->GetInput();
    if (input.IsTrigger(Action::ESCAPE))
    {
        m_open = false;
        return Result::Resume;
    }

    const int count = m_fromStageSelect ? 3 : 4;
    if (input.IsTrigger(Action::UP)) m_cursor = (m_cursor + count - 1) % count;
    if (input.IsTrigger(Action::DOWN)) m_cursor = (m_cursor + 1) % count;
    if (!input.IsTrigger(Action::ENTER)) return Result::None;

    if (m_cursor == 0) { m_open = false; return Result::Resume; }
    if (m_cursor == 1) { m_settings.Open(); return Result::None; }
    if (!m_fromStageSelect && m_cursor == 2) return Result::StageSelect;
    return Result::Quit;
}

void PauseMenu::Draw(int alpha) const
{
    if (!m_open || alpha <= 0) return;
    if (m_settings.IsOpen())
    {
        m_settings.Draw();
        return;
    }
    if (alpha > 255) alpha = 255;
    DrawBackdrop(m_game, true, alpha);

    int oldBlend, oldAlpha;
    GetDrawBlendMode(&oldBlend, &oldAlpha);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    Renderer* renderer = m_game->GetRenderer();
    const float width = static_cast<float>(m_game->GetWidth());
    const float height = static_cast<float>(m_game->GetHeight());
    renderer->DrawTextL(Vector2d(width * 0.13f, height * 0.10f),
        "ポーズ", Color(8, 8, 8), m_game->GatDebugFont(), 56, false);

    const char* labels[] = {
        m_fromStageSelect ? "ステージ選択に戻る" : "ステージに戻る",
        "設定変更", "ステージ選択画面へ", "ゲーム終了"
    };
    const int count = m_fromStageSelect ? 3 : 4;
    for (int i = 0; i < count; ++i)
    {
        const int labelIndex = m_fromStageSelect && i == 2 ? 3 : i;
        MenuVisual::DrawOption(m_game, labels[labelIndex], width * 0.18f,
            height * (0.30f + i * 0.14f), 43, i == m_cursor);
    }
    SetDrawBlendMode(oldBlend, oldAlpha);
}
