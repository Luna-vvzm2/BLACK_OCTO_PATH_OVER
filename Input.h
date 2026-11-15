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
    SHADOW_HEAL,
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
    ZOOM_IN,
    ZOOM_OUT,
    ACTION_COUNT
};


class Input {
public:
    Input();
    void Init();
    void ResetKeyBindings();
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


    const Key& GetKey() const { return m_key; }
    const Joypad& GetPad() const { return m_pad; }

    // メニュー専用の固定入力
    bool IsMenuDown(Action action) const;
    bool IsMenuTrigger(Action action) const;

private:
    Key m_key;
    Joypad m_pad;

    std::array<
        std::vector<Key::KeyIndex>,
        static_cast<size_t>(Action::ACTION_COUNT)
    > m_actionKeyMap;

    std::array<
        std::vector<Joypad::ButtonIndex>,
        static_cast<size_t>(Action::ACTION_COUNT)
    > m_actionPadMap;

};
