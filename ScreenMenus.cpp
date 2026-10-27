#include "ScreenMenus.h"

#include "Game.h"
#include "SoundComponent.h"
#include "Renderer.h"
#include "Color.h"
#include "SaveManager.h"
#include "SaveData.h"

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

    LoadSave(m_game);
}

void SettingsMenu::Update()
{
    if (!m_open) return;
    Input& input = m_game->GetInput();

    if (m_page == Page::Controls)
    {
        const auto& rows = GetRows();
        Input& input = m_game->GetInput();

        // 1. キー入力待ち（割り当て中）
        if (m_waitingForKey)
        {
            int keyIndex = input.GetKey().GetFirstTriggeredKey();
            if (keyIndex >= 0)
            {
                if (input.SetKeyBinding(rows[m_keyConfigCursor].action, keyIndex))
                {
                    m_waitingForKey = false;
                    // セーブ処理
                    SaveSettings();
                }
            }
            return;
        }

        // ESCキー：設定のルートメニューに戻る
        if (input.GetKey().IsTrigger(Key::ESCAPE) || input.IsMenuTrigger(Action::ESCAPE))
        {
            m_game->GetSoundManager().Play("select_default");
            m_page = Page::Root;
            m_cursor = 1;
            return;
        }

        // Rキー：初期設定に戻す
        if (input.GetKey().IsTrigger(Key::R))
        {
            input.Init();
            SaveSettings();
            m_keyConfigCursor = 0;
            m_waitingForKey = false;
            return;
        }

        // 上下キー：操作項目の選択（Actionではなく物理キーの矢印やW/Sで直接判定）
        const int rowCount = static_cast<int>(rows.size());
        if (input.GetKey().IsTrigger(Key::UP) || input.GetKey().IsTrigger(Key::W)) {
            m_keyConfigCursor = (m_keyConfigCursor + rowCount - 1) % rowCount;
            m_game->GetSoundManager().Play("game_select_mode");
        }
        if (input.GetKey().IsTrigger(Key::DOWN) || input.GetKey().IsTrigger(Key::S)) {
            m_keyConfigCursor = (m_keyConfigCursor + 1) % rowCount;
            m_game->GetSoundManager().Play("game_select_mode");
        }

        // 左右キー：入力方式の切り替え（物理キーの左右やA/Dで直接判定）
        if (input.GetKey().IsTrigger(Key::LEFT) || input.GetKey().IsTrigger(Key::A) ||
            input.GetKey().IsTrigger(Key::RIGHT) || input.GetKey().IsTrigger(Key::D))
        {
            Action action = rows[m_keyConfigCursor].action;
            InputMode current = input.GetInputMode(action);
            InputMode next = (current == InputMode::Hold) ? InputMode::Toggle : InputMode::Hold;
            input.SetInputMode(action, next);
            SaveSettings();
        }

        // Enterキー：キー割り当て変更の開始（物理キーの Enter で判定）
        if (input.GetKey().IsTrigger(Key::ENTER))
        {
            m_game->GetSoundManager().Play("select_default");
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
    if (input.IsTrigger(Action::UP)) {
        m_cursor = (m_cursor + count - 1) % count;
        m_game->GetSoundManager().Play("game_select_mode");
    }
    if (input.IsTrigger(Action::DOWN)) {
        m_cursor = (m_cursor + 1) % count;
        m_game->GetSoundManager().Play("game_select_mode");
    }

    if (m_page == Page::Volume && m_cursor < 4)
    {
        const int step = input.IsTrigger(Action::RIGHT) ? 5 : (input.IsTrigger(Action::LEFT) ? -5 : 0);
        if (step != 0)
        {
            const int before = m_cursor == 3 ? SoundComponent::GetVoiceVolume() : m_game->GetVolume(m_cursor);
            if (m_cursor == 3) SoundComponent::AdjustVoiceVolume(step);
            else m_game->AdjustVolume(m_cursor, step);
            const int after = m_cursor == 3 ? SoundComponent::GetVoiceVolume() : m_game->GetVolume(m_cursor);
            if (after != before) m_game->GetSoundManager().Play("select_default");
        }
    }

    if (!input.IsTrigger(Action::ENTER)) return;
    if (m_page == Page::Volume && m_cursor < 4) return;
    m_game->GetSoundManager().Play("select_default");

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
            float y = height * (0.20f + i * 0.042f); // 項目数が多いので間隔を少し詰める
            bool selected = (i == m_keyConfigCursor);

            // 項目名
            MenuVisual::DrawOption(m_game, rows[i].label, x, y, 26, selected);

            // 割り当てキー名 or 入力待ち表示
            std::string keyName;
            if (selected && m_waitingForKey) {
                keyName = "キーを押してください...";
            }
            else {
                int keyIndex = input.GetKeyBinding(rows[i].action);
                keyName = Key::GetKeyName(keyIndex);
            }
            renderer->DrawTextL(Vector2d(x + 350.0f, y), keyName, selected ? Color(200, 35, 35) : Color(8, 8, 8), font, 24, false);

            // 入力方式（長押し／トグル）
            const char* modeName = (input.GetInputMode(rows[i].action) == InputMode::Hold) ? "長押し" : "トグル";
            renderer->DrawTextL(Vector2d(x + 650.0f, y), modeName, Color(70, 70, 70), font, 24, false);
        }

        // 下部ヘルプ
        renderer->DrawTextL(Vector2d(width * 0.15f, height * 0.90f),
            "上下:選択  左右:入力方式  Enter:変更  R:初期化  Esc:戻る",
            Color(35, 35, 35), font, 20, false);
    }
    return;
}

const std::vector<SettingsMenu::BindingRow>& SettingsMenu::GetRows()
{
    static const std::vector<BindingRow> rows =
    {
        { Action::UP,          "上移動" },
        { Action::DOWN,        "しゃがみ／下移動" },
        { Action::LEFT,        "左移動" },
        { Action::RIGHT,       "右移動" },
        { Action::DASH,        "回避" },
        { Action::WEAK_ATTACK, "刀攻撃" },
        { Action::KUNAI,       "苦無投げ" },
        { Action::HEAL,        "回復アイテム" },
        { Action::KAMAE,       "構え" },
        { Action::G,           "特殊操作 G" },
        { Action::JUTSU_KAMAE, "忍術構え" },
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
        saveData.inputModes[i] = static_cast<int>(input.GetInputMode(action));
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

        for (std::size_t i = 0; i < SaveData::ActionCount; ++i)
        {
            Action action = static_cast<Action>(i);

            // キー割り当てを反映
            if (!saveData.keyBindings[i].empty())
            {
                input.SetKeyBindings(action, saveData.keyBindings[i]);
            }

            // 入力方式（Hold / Toggle）を反映
            if (saveData.inputModes[i] == 0 || saveData.inputModes[i] == 1)
            {
                input.SetInputMode(action, static_cast<InputMode>(saveData.inputModes[i]));
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
    if (input.IsTrigger(Action::UP)) {
        m_cursor = (m_cursor + count - 1) % count;
        m_game->GetSoundManager().Play("game_select_mode");
    }
    if (input.IsTrigger(Action::DOWN)) {
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

