#include "ScreenMenus.h"

#include "Game.h"
#include "SoundComponent.h"
#include "Renderer.h"
#include "Color.h"
#include "SaveManager.h"
#include "SaveData.h"

#include <DxLib.h>
#include <algorithm>
#include <cstdlib>
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
    struct VolumeSliderGeometry
    {
        int barLeft;
        int barWidth;
        int centerY;
        int trackLeft;
        int trackRight;
        int knobX;
    };

    VolumeSliderGeometry GetVolumeSliderGeometry(const Game* game, int volume, float rowY)
    {
        const int barLeft = static_cast<int>(game->GetWidth() * 0.43f);
        const int barWidth = static_cast<int>(game->GetWidth() * 0.38f);
        const int trackLeft = barLeft + static_cast<int>(barWidth * 58.0f / 980.0f + 0.5f);
        const int trackRight = barLeft + static_cast<int>(barWidth * 899.0f / 980.0f + 0.5f);
        const int knobX = trackLeft + static_cast<int>(
            (trackRight - trackLeft) * std::clamp(volume, 0, 100) / 100.0f + 0.5f);
        return { barLeft, barWidth, static_cast<int>(rowY) + 22,
            trackLeft, trackRight, knobX };
    }

    void DrawVolumeSlider(const Game* game, int volume, float rowY)
    {
        static const int barHandle = LoadGraph("assets/images/uies/SoundTyousei/volume.png");
        static const int knobHandle = LoadGraph("assets/images/uies/SoundTyousei/volume_scrollhandle.png");
        const VolumeSliderGeometry slider = GetVolumeSliderGeometry(game, volume, rowY);

        if (barHandle >= 0)
        {
            DrawExtendGraph(slider.barLeft, slider.centerY - 17,
                slider.barLeft + slider.barWidth, slider.centerY + 17, barHandle, TRUE);
        }
        if (knobHandle >= 0)
        {
            DrawExtendGraph(slider.knobX - 11, slider.centerY - 13,
                slider.knobX + 11, slider.centerY + 13, knobHandle, TRUE);
        }
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

        static const int markerHandle = LoadGraph("assets/images/icon/kunai.png");
        if (markerHandle < 0) return;
        const float scale = static_cast<float>(fontSize) / 53.0f;
        const int size = static_cast<int>(100.0f * scale);
        const int left = static_cast<int>(x - 100.0f * scale);
        const int top = static_cast<int>(y - 15.0f * scale);
        DrawExtendGraph(left, top, left + size, top + size, markerHandle, TRUE);
    }
}
SettingsMenu::SettingsMenu(Game* game, bool overGame) : m_game(game), m_overGame(overGame) {}

void SettingsMenu::Open()
{
    m_open = true;
    m_page = Page::Root;
    m_cursor = 0;
    m_keyConfigCursor = 0;
    m_waitingForKey = false;
    m_dragVolumeRow = -1;
    m_mouseWasDown = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

    LoadSave(m_game);
}

void SettingsMenu::Update()
{
    if (!m_open) return;
    Input& input = m_game->GetInput();
    const bool mouseDown = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
    const bool mousePressed = mouseDown && !m_mouseWasDown;
    m_mouseWasDown = mouseDown;
    if (!mouseDown || m_page != Page::Volume) m_dragVolumeRow = -1;

    if (m_page == Page::Controls)
    {
        const auto& rows = GetRows();
        Input& input = m_game->GetInput();

        const int bindingCount = static_cast<int>(rows.size());
        const int saveRow = bindingCount;
        const int restoreRow = bindingCount + 1;
        const int defaultsRow = bindingCount + 2;
        const int rowCount = bindingCount + 3;

        if (m_waitingForKey)
        {
            if (input.IsMenuTrigger(Action::ESCAPE))
            {
                m_waitingForKey = false;
                return;
            }
            const int keyIndex = input.GetKey().GetFirstTriggeredKey();
            if (keyIndex >= 0 && input.SetKeyBinding(rows[m_keyConfigCursor].action, keyIndex))
                m_waitingForKey = false;
            return;
        }
        if (input.IsMenuTrigger(Action::ESCAPE))
        {
            RestoreBindings();
            m_game->GetSoundManager().Play("select_default");
            m_page = Page::Root;
            m_cursor = 1;
            return;
        }
        if (input.IsMenuTrigger(Action::UP))
        {
            m_keyConfigCursor = (m_keyConfigCursor + rowCount - 1) % rowCount;
            m_game->GetSoundManager().Play("game_select_mode");
        }
        if (input.IsMenuTrigger(Action::DOWN))
        {
            m_keyConfigCursor = (m_keyConfigCursor + 1) % rowCount;
            m_game->GetSoundManager().Play("game_select_mode");
        }
        if (input.IsMenuTrigger(Action::ENTER))
        {
            m_game->GetSoundManager().Play("select_default");
            if (m_keyConfigCursor == saveRow)
            {
                SaveSettings();
                m_page = Page::Root;
                m_cursor = 1;
            }
            else if (m_keyConfigCursor == restoreRow)
                RestoreBindings();
            else if (m_keyConfigCursor == defaultsRow)
                input.ResetKeyBindings();
            else
                m_waitingForKey = true;
        }
        return;
    }

    if (input.IsTrigger(Action::ESCAPE))
    {
        m_game->GetSoundManager().Play("select_default");
        if (m_page == Page::Root) m_open = false;
        else { m_page = Page::Root; m_cursor = 0; }
        return;
    }

    const int count = m_page == Page::Volume ? 5 : (m_page == Page::Root ? 3 : 1);
    if (input.IsMenuTrigger(Action::UP)) {
        m_cursor = (m_cursor + count - 1) % count;
        m_game->GetSoundManager().Play("game_select_mode");
    }
    if (input.IsMenuTrigger(Action::DOWN)) {
        m_cursor = (m_cursor + 1) % count;
        m_game->GetSoundManager().Play("game_select_mode");
    }

    if (m_page == Page::Volume && m_cursor < 4)
    {
        const int step = input.IsMenuTrigger(Action::RIGHT) ? 5 : (input.IsMenuTrigger(Action::LEFT) ? -5 : 0);
        if (step != 0)
        {
            const int before = m_cursor == 3 ? SoundComponent::GetVoiceVolume() : m_game->GetVolume(m_cursor);
            const int next = std::clamp(step > 0 ? (before / 5 + 1) * 5 : ((before - 1) / 5) * 5, 0, 100);
            if (m_cursor == 3) SoundComponent::AdjustVoiceVolume(next - before);
            else m_game->AdjustVolume(m_cursor, next - before);
            const int after = m_cursor == 3 ? SoundComponent::GetVoiceVolume() : m_game->GetVolume(m_cursor);
            if (after != before) m_game->GetSoundManager().Play("select_default");
        }
    }

    if (m_page == Page::Volume)
    {
        int mouseX = 0;
        int mouseY = 0;
        GetMousePoint(&mouseX, &mouseY);
        if (mousePressed)
        {
            for (int row = 0; row < 4; ++row)
            {
                const int volume = row == 3 ? SoundComponent::GetVoiceVolume() : m_game->GetVolume(row);
                const float rowY = m_game->GetHeight() * (0.25f + row * 0.125f);
                const VolumeSliderGeometry slider = GetVolumeSliderGeometry(m_game, volume, rowY);
                if (std::abs(mouseX - slider.knobX) <= 16 &&
                    std::abs(mouseY - slider.centerY) <= 18)
                {
                    m_dragVolumeRow = row;
                    m_cursor = row;
                    break;
                }
            }
        }
        if (mouseDown && m_dragVolumeRow >= 0)
        {
            const int row = m_dragVolumeRow;
            const int before = row == 3 ? SoundComponent::GetVoiceVolume() : m_game->GetVolume(row);
            const float rowY = m_game->GetHeight() * (0.25f + row * 0.125f);
            const VolumeSliderGeometry slider = GetVolumeSliderGeometry(m_game, before, rowY);
            const float position = std::clamp(
                static_cast<float>(mouseX - slider.trackLeft) / (slider.trackRight - slider.trackLeft),
                0.0f, 1.0f);
            const int next = static_cast<int>(position * 20.0f + 0.5f) * 5;
            if (next != before)
            {
                if (row == 3) SoundComponent::AdjustVoiceVolume(next - before);
                else m_game->AdjustVolume(row, next - before);
            }
        }
    }
    if (!input.IsTrigger(Action::ENTER)) return;
    if (m_page == Page::Volume && m_cursor < 4) return;
    m_game->GetSoundManager().Play("select_default");

    if (m_page == Page::Root)
    {
        if (m_cursor == 0) { m_page = Page::Volume; m_cursor = 0; }
        else if (m_cursor == 1) { CaptureBindings(); m_page = Page::Controls; m_keyConfigCursor = 0; }
        else m_open = false;
    }
    else if (m_page == Page::Volume && m_cursor == 4)
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
            const float rowY = height * (0.25f + i * 0.125f);
            MenuVisual::DrawOption(m_game, labels[i], x, rowY, 39, i == m_cursor);
            DrawVolumeSlider(m_game, volume, rowY);
            renderer->DrawTextL(Vector2d(width * 0.86f, rowY + 4.0f),
                std::to_string(volume) + "%",
                i == m_cursor ? Color(200, 35, 35) : Color(8, 8, 8), font, 31, false);
        }
        MenuVisual::DrawOption(m_game, "戻る", x, height * 0.75f, 39, m_cursor == 4);
        renderer->DrawTextL(Vector2d(x, height * 0.89f),
            "つまみをドラッグ／左右キーで5%ずつ変更", Color(35, 35, 35), font, 23, false);
    }
    else if (m_page == Page::Controls)
    {
        // キーコンフィグ画面の描画 (KeyConfigMenu.cpp から移植・調整)

        Renderer* renderer = m_game->GetRenderer();
        const float width = static_cast<float>(m_game->GetWidth());
        const float height = static_cast<float>(m_game->GetHeight());
        const auto& font = m_game->GatDebugFont();

        const auto& rows = GetRows();
        Input& input = m_game->GetInput();

        for (int i = 0; i < static_cast<int>(rows.size()); ++i)
        {
            // 画面の縦幅に合わせて各行を配置
            float x = width * 0.15f;
            float y = height * (0.20f + i * 0.040f); // 項目数が多いので間隔を少し詰める
            bool selected = (i == m_keyConfigCursor);

            // 項目名
            MenuVisual::DrawOption(m_game, rows[i].label, x, y, 26, selected);

            // 割り当てキー名 or 入力待ち表示
            std::string keyName;
            if (selected && m_waitingForKey) {
                keyName = "キー入力待ち...";
            }
            else {
                int keyIndex = input.GetKeyBinding(rows[i].action);
                keyName = Key::GetKeyName(keyIndex);
            }
            renderer->DrawTextL(Vector2d(x + 350.0f, y), keyName, selected ? Color(200, 35, 35) : Color(8, 8, 8), font, 24, false);

        }

        const int bindingCount = static_cast<int>(rows.size());
        const char* buttons[] = { "保存して戻る", "変更前に戻す", "初期設定に戻す" };
        for (int i = 0; i < 3; ++i)
            MenuVisual::DrawOption(m_game, buttons[i], width * 0.15f,
                height * (0.72f + i * 0.055f), 25, m_keyConfigCursor == bindingCount + i);
        renderer->DrawTextL(Vector2d(width * 0.15f, height * 0.91f),
            "上下:選択  Enter:変更・決定",
            Color(35, 35, 35), font, 18, false);
        renderer->DrawTextL(Vector2d(width * 0.15f, height * 0.95f),
            "Esc:変更を取り消して戻る",
            Color(35, 35, 35), font, 18, false);
    }
    return;
}

const std::vector<SettingsMenu::BindingRow>& SettingsMenu::GetRows()
{
    static const std::vector<BindingRow> rows =
    {
        { Action::DOWN,        "しゃがみ／下移動" },
        { Action::LEFT,        "左移動" },
        { Action::RIGHT,       "右移動" },
        { Action::DASH,        "回避" },
        { Action::WEAK_ATTACK, "刀攻撃" },
        { Action::KUNAI,       "苦無投げ" },
        { Action::HEAL,        "回復アイテム" },
        { Action::SHADOW_HEAL, "影ゲージ回復の構え" },
        { Action::OCTO,        "忍術：蛸" },
        { Action::TIGER,       "忍術：虎" },
        { Action::ORCA,        "忍術：鯱" },
        { Action::JUMP,        "ジャンプ" }
    };
    return rows;
}

bool SettingsMenu::IsCapturingKey() const
{
    return m_open && m_page == Page::Controls && m_waitingForKey;
}

void SettingsMenu::CaptureBindings()
{
    const Input& input = m_game->GetInput();
    for (size_t i = 0; i < m_originalBindings.size(); ++i)
        m_originalBindings[i] = input.GetKeyBindings(static_cast<Action>(i));
}

void SettingsMenu::RestoreBindings()
{
    Input& input = m_game->GetInput();
    for (size_t i = 0; i < m_originalBindings.size(); ++i)
        input.SetKeyBindings(static_cast<Action>(i), m_originalBindings[i]);
}

void SettingsMenu::SaveSettings()
{
    SaveData saveData;
    // 既存のセーブデータがあれば読み込む（ステージ進行度やプレイ時間等を保持するため）
    SaveManager::Load(saveData);

    Input& input = m_game->GetInput();

    // 全アクションのキーバインドと入力モードをSaveDataにコピー
    for (std::size_t i = 0; i < SaveData::ActionCount; ++i)
    {
        Action action = static_cast<Action>(i);

        saveData.keyBindings[i] = input.GetKeyBindings(action);
    }

    // ファイルへ保存
    SaveManager::Save(saveData);
}

void SettingsMenu::LoadSave(Game* game)
{
    if (!game) return;

    SaveData saveData;
    if (SaveManager::Load(saveData))
    {
        Input& input = game->GetInput();
        input.ResetKeyBindings();

        for (std::size_t i = 0; i < SaveData::ActionCount; ++i)
        {
            Action action = static_cast<Action>(i);

            // キー割り当てを反映
            if (saveData.hasKeyBinding[i])
            {
                input.SetKeyBindings(action, saveData.keyBindings[i]);
            }

        }
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
    if (input.IsMenuTrigger(Action::UP)) {
        m_cursor = (m_cursor + count - 1) % count;
        m_game->GetSoundManager().Play("game_select_mode");
    }
    if (input.IsMenuTrigger(Action::DOWN)) {
        m_cursor = (m_cursor + 1) % count;
        m_game->GetSoundManager().Play("game_select_mode");
    }
    if (!input.IsTrigger(Action::ENTER)) return Result::None;
    m_game->GetSoundManager().Play("select_default");

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
