#include "KeyConfigMenu.h"

#include "Menu.h"
#include "Game.h"

#include <DxLib.h>

KeyConfigMenu::KeyConfigMenu(Menu* owner)
    : MenuPage(owner)
{
}

const std::vector<KeyConfigMenu::BindingRow>&
KeyConfigMenu::GetRows()
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

void KeyConfigMenu::Initialize()
{
    m_selectedIndex = 0;
    m_waitingForKey = false;
}

void KeyConfigMenu::Update(float deltaTime)
{
    (void)deltaTime;

    if (m_owner->GetCursorArea() !=
        Menu::MenuCursorArea::KeyConfigList)
    {
        return;
    }

    Input& input = m_owner->GetGame()->GetInput();

    const auto& rows = GetRows();

    // キー入力待ち
    if (m_waitingForKey)
    {
        int keyIndex =
            input.GetKey().GetFirstTriggeredKey();

        if (keyIndex >= 0)
        {
            if (input.SetKeyBinding(
                rows[m_selectedIndex].action,
                keyIndex))
            {
                m_waitingForKey = false;

                // 変更を即時保存
                m_owner->SaveSettings();
            }
        }

        return;
    }

    // Rキー：初期設定に戻す
    if (input.GetKey().IsTrigger(Key::R))
    {
        input.Init();
        m_owner->SaveSettings();

        m_selectedIndex = 0;
        m_waitingForKey = false;
        return;
    }

    // ESCキー：タブ選択に戻る（固定操作）
    if (input.IsMenuTrigger(Action::ESCAPE))
    {
        m_owner->SetCursorArea(
            Menu::MenuCursorArea::Tab
        );

        return;
    }

    // 上下キー：操作項目を選択（固定操作）
    if (input.IsMenuTrigger(Action::UP))
    {
        --m_selectedIndex;
    }

    if (input.IsMenuTrigger(Action::DOWN))
    {
        ++m_selectedIndex;
    }

    ClampCursor();

    // 左右キー：長押し／トグルの切り替え（固定操作）
    if (input.IsMenuTrigger(Action::LEFT) ||
        input.IsMenuTrigger(Action::RIGHT))
    {
        Action action = rows[m_selectedIndex].action;

        InputMode current = input.GetInputMode(action);

        InputMode next =
            current == InputMode::Hold
            ? InputMode::Toggle
            : InputMode::Hold;

        input.SetInputMode(action, next);

        // 変更を即時保存
        m_owner->SaveSettings();
    }

    // Enter：キー割り当て変更（固定操作）
    if (input.IsMenuTrigger(Action::ENTER))
    {
        m_waitingForKey = true;
    }
}

void KeyConfigMenu::Draw()
{
    Input& input = m_owner->GetGame()->GetInput();

    const auto& rows = GetRows();

    const int startX = 45;
    const int startY = 100;
    const int rowHeight = 27;

    DrawString(
        startX,
        76,
        "操作設定",
        GetColor(255, 255, 255)
    );

    DrawString(
        80,
        78,
        "操作",
        GetColor(180, 180, 180)
    );

    DrawString(
        520,
        78,
        "割り当てキー",
        GetColor(180, 180, 180)
    );

    DrawString(
        850,
        78,
        "入力方式",
        GetColor(180, 180, 180)
    );

    for (int i = 0;
        i < static_cast<int>(rows.size());
        ++i)
    {
        int y = startY + i * rowHeight;

        bool selected = (i == m_selectedIndex);

        DrawBox(
            startX,
            y,
            1225,
            y + rowHeight - 2,
            selected
            ? GetColor(70, 70, 95)
            : GetColor(35, 35, 35),
            TRUE
        );

        if (selected)
        {
            DrawBox(
                startX,
                y,
                1225,
                y + rowHeight - 2,
                GetColor(255, 80, 80),
                FALSE
            );
        }

        DrawString(
            80,
            y + 4,
            rows[i].label,
            GetColor(255, 255, 255)
        );

        if (selected && m_waitingForKey)
        {
            DrawString(
                520,
                y + 4,
                "キーを押してください...",
                GetColor(255, 220, 80)
            );
        }
        else
        {
            int keyIndex =
                input.GetKeyBinding(rows[i].action);

            DrawString(
                520,
                y + 4,
                Key::GetKeyName(keyIndex),
                GetColor(255, 255, 255)
            );
        }

        const char* modeName =
            input.GetInputMode(rows[i].action)
            == InputMode::Hold
            ? "長押し"
            : "トグル";

        DrawString(
            850,
            y + 4,
            modeName,
            GetColor(255, 255, 255)
        );
    }

    if (m_waitingForKey)
    {
        DrawString(
            45,
            660,
            "割り当てたいキーを押してください",
            GetColor(255, 220, 80)
        );
    }
    else
    {
        DrawString(
            45,
            660,
            "上下：操作選択  左右：入力方式  Enter：キー変更  R：初期設定  Esc：戻る",
            GetColor(255, 255, 255)
        );
    }
}

void KeyConfigMenu::ClampCursor()
{
    const auto& rows = GetRows();

    if (m_selectedIndex < 0)
        m_selectedIndex = 0;

    if (m_selectedIndex >= static_cast<int>(rows.size()))
        m_selectedIndex = static_cast<int>(rows.size()) - 1;
}