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

    const Key& GetKey() const { return m_key; }
    const Joypad& GetPad() const { return m_pad; }

private:
    Key m_key;
    Joypad m_pad;


    // アクションとキーの対応
    std::array<std::vector<Key::KeyIndex>, static_cast<size_t>(Action::ACTION_COUNT)> m_actionKeyMap;
    std::array<std::vector<Joypad::ButtonIndex>, static_cast<size_t>(Action::ACTION_COUNT)> m_actionPadMap;

};
