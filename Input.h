#pragma once

#include "Key.h"
#include "Joypad.h"
#include <array>
#include <vector>

// ゲーム内で扱うアクション
enum class Action {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    DASH,
    WEAK_ATTACK,
    KUNAI,
    HEAL,
    KAMAE,
    G,
    JUTSU_KAMAE,
    OCTO,
    TIGER,
    ORCA,
    JUMP,
    ESCAPE,
    ENTER,
    MENU,
    PAGE_UP,
    PAGE_DOWN,
    ACTION_COUNT
};

// 操作方式
enum class InputMode {
    Hold = 0,   // 長押し
    Toggle = 1  // 押すたびにON/OFF切り替え
};

class Input {
public:
    Input();

    void Init();
    void Update();

    // アクション単位で入力取得
    bool IsDown(Action action) const;
    bool IsTrigger(Action action) const;
    bool IsRelease(Action action) const;
    int GetPressFrame(Action action) const;

    // キー割り当ての取得・変更
    int GetKeyBinding(Action action) const;
    bool SetKeyBinding(Action action, int keyIndex);

    // 操作に割り当てられているすべてのキーを取得
    std::vector<int> GetKeyBindings(Action action) const;

    // 複数のキー割り当てを復元
    bool SetKeyBindings(
        Action action,
        const std::vector<int>& keyIndices
    );

    // 操作方式の取得・変更
    InputMode GetInputMode(Action action) const;
    void SetInputMode(Action action, InputMode mode);

    const Key& GetKey() const { return m_key; }
    const Joypad& GetPad() const { return m_pad; }

    // メニュー専用の固定入力
    bool IsMenuDown(Action action) const;
    bool IsMenuTrigger(Action action) const;

private:
    Key m_key;
    Joypad m_pad;

    // アクションとキーの対応
    std::array<
        std::vector<Key::KeyIndex>,
        static_cast<size_t>(Action::ACTION_COUNT)
    > m_actionKeyMap;

    std::array<
        std::vector<Joypad::ButtonIndex>,
        static_cast<size_t>(Action::ACTION_COUNT)
    > m_actionPadMap;

    // 操作方式
    std::array<
        InputMode,
        static_cast<size_t>(Action::ACTION_COUNT)
    > m_actionInputModes{};

    // トグル方式で現在ONになっているか
    std::array<
        bool,
        static_cast<size_t>(Action::ACTION_COUNT)
    > m_toggleStates{};
};