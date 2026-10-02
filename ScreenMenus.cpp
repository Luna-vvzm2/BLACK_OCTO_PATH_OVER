#include "ScreenMenus.h"

#include "Game.h"
#include "Input.h"
#include "Renderer.h"
#include "Color.h"

#include <DxLib.h>
#include <algorithm>
#include <string>

namespace
{
    void DrawBackdrop(const Game* game)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 205);
        DrawBox(0, 0, game->GetWidth(), game->GetHeight(), GetColor(10, 14, 22), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    void DrawItem(Game* game, const std::string& label, int index, int selected)
    {
        Renderer* renderer = game->GetRenderer();
        const float centerX = game->GetWidth() * 0.5f;
        const float y = game->GetHeight() * 0.38f + index * 72.0f;
        const bool active = index == selected;
        renderer->DrawTextC(
            Vector2d(centerX, y),
            (active ? "> " : "  ") + label + (active ? " <" : "  "),
            active ? Color(255, 220, 110) : Color(225, 225, 225),
            game->GatDebugFont(), 32, false);
    }
}

SettingsMenu::SettingsMenu(Game* game) : m_game(game) {}

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

    const int count = m_page == Page::Volume ? 4 : (m_page == Page::Root ? 3 : 1);
    if (input.IsTrigger(Action::UP)) m_cursor = (m_cursor + count - 1) % count;
    if (input.IsTrigger(Action::DOWN)) m_cursor = (m_cursor + 1) % count;

    if (m_page == Page::Volume && m_cursor < 3)
    {
        const int step = input.IsTrigger(Action::RIGHT) ? 10 : (input.IsTrigger(Action::LEFT) ? -10 : 0);
        if (step != 0) m_game->AdjustVolume(m_cursor, step);
    }

    if (!input.IsTrigger(Action::ENTER)) return;

    if (m_page == Page::Root)
    {
        if (m_cursor == 0) { m_page = Page::Volume; m_cursor = 0; }
        else if (m_cursor == 1) { m_page = Page::Controls; m_cursor = 0; }
        else m_open = false;
    }
    else if (m_page == Page::Volume && m_cursor == 3)
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
    DrawBackdrop(m_game);
    Renderer* renderer = m_game->GetRenderer();
    const float centerX = m_game->GetWidth() * 0.5f;
    const auto& font = m_game->GatDebugFont();

    const char* title = m_page == Page::Root ? "設定画面" :
        (m_page == Page::Volume ? "音量設定" : "操作方法");
    renderer->DrawTextC(Vector2d(centerX, 100), title, Color(255, 255, 255), font, 46, false);

    if (m_page == Page::Root)
    {
        DrawItem(m_game, "音量設定", 0, m_cursor);
        DrawItem(m_game, "操作方法", 1, m_cursor);
        DrawItem(m_game, "戻る", 2, m_cursor);
    }
    else if (m_page == Page::Volume)
    {
        const char* labels[] = { "マスター", "効果音", "BGM" };
        for (int i = 0; i < 3; ++i)
            DrawItem(m_game, std::string(labels[i]) + "  " + std::to_string(m_game->GetVolume(i)) + "%", i, m_cursor);
        DrawItem(m_game, "戻る", 3, m_cursor);
        renderer->DrawTextC(Vector2d(centerX, 650), "左右キー／スティックで10%ずつ変更", Color(185, 185, 185), font, 22, false);
    }
    else
    {
        renderer->DrawTextC(Vector2d(centerX, 260), "移動: WASD／方向キー    決定: Enter／B", Color(225, 225, 225), font, 24, false);
        renderer->DrawTextC(Vector2d(centerX, 310), "ジャンプ: Space／A    攻撃: I／X", Color(225, 225, 225), font, 24, false);
        renderer->DrawTextC(Vector2d(centerX, 360), "ポーズ: Esc／START    メニュー: M／BACK", Color(225, 225, 225), font, 24, false);
        renderer->DrawTextC(Vector2d(centerX, 430), "キーの割り当て変更は未実装", Color(190, 190, 190), font, 22, false);
        DrawItem(m_game, "戻る", 3, m_cursor + 3);
    }
}

PauseMenu::PauseMenu(Game* game, bool fromStageSelect)
    : m_game(game), m_fromStageSelect(fromStageSelect), m_settings(game) {}

void PauseMenu::Open()
{
    m_open = true;
    m_cursor = 0;
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

void PauseMenu::Draw() const
{
    if (!m_open) return;
    DrawBackdrop(m_game);
    Renderer* renderer = m_game->GetRenderer();
    renderer->DrawTextC(Vector2d(m_game->GetWidth() * 0.5f, 100), "ポーズ", Color(255, 255, 255), m_game->GatDebugFont(), 46, false);

    DrawItem(m_game, m_fromStageSelect ? "ステージ選択に戻る" : "ステージに戻る", 0, m_cursor);
    DrawItem(m_game, "設定画面", 1, m_cursor);
    if (!m_fromStageSelect) DrawItem(m_game, "ステージ選択画面へ", 2, m_cursor);
    DrawItem(m_game, "ゲーム終了", m_fromStageSelect ? 2 : 3, m_cursor);
    m_settings.Draw();
}
