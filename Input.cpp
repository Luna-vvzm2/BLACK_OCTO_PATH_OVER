#include "Input.h"
#include <algorithm>

Input::Input() {
    Init();
}

void Input::Init() {
    // アクションとキーを対応付け
    m_actionKeyMap[static_cast<size_t>(Action::UP)] = { Key::UP, Key::W };
    m_actionKeyMap[static_cast<size_t>(Action::DOWN)] = { Key::DOWN, Key::S };
    m_actionKeyMap[static_cast<size_t>(Action::LEFT)] = { Key::LEFT, Key::A };
    m_actionKeyMap[static_cast<size_t>(Action::RIGHT)] = { Key::RIGHT, Key::D };
    m_actionKeyMap[static_cast<size_t>(Action::DASH)] = { Key::U };
    m_actionKeyMap[static_cast<size_t>(Action::WEAK_ATTACK)] = { Key::I };
    m_actionKeyMap[static_cast<size_t>(Action::STRONG_ATTACK)] = { Key::O };
    m_actionKeyMap[static_cast<size_t>(Action::KUNAI)] = { Key::P };
    m_actionKeyMap[static_cast<size_t>(Action::KAMAE)] = { Key::Q };
    m_actionKeyMap[static_cast<size_t>(Action::G)] = { Key::G };
    m_actionKeyMap[static_cast<size_t>(Action::JUTSU_KAMAE)] = { Key::H };
    m_actionKeyMap[static_cast<size_t>(Action::JUMP)] = { Key::SPACE };
    m_actionKeyMap[static_cast<size_t>(Action::ESCAPE)] = { Key::ESCAPE };
    m_actionKeyMap[static_cast<size_t>(Action::ENTER)] = { Key::ENTER };
    m_actionKeyMap[static_cast<size_t>(Action::MENU)] = { Key::M };

    // ジョイパッドも同様に
    m_actionPadMap[static_cast<size_t>(Action::UP)] = { Joypad::UP };
    m_actionPadMap[static_cast<size_t>(Action::DOWN)] = { Joypad::DOWN };
    m_actionPadMap[static_cast<size_t>(Action::LEFT)] = { Joypad::LEFT };
    m_actionPadMap[static_cast<size_t>(Action::RIGHT)] = { Joypad::RIGHT };
    m_actionPadMap[static_cast<size_t>(Action::DASH)] = { Joypad::RB };
    m_actionPadMap[static_cast<size_t>(Action::WEAK_ATTACK)] = { Joypad::X };
    m_actionPadMap[static_cast<size_t>(Action::STRONG_ATTACK)] = { Joypad::Y };
    m_actionPadMap[static_cast<size_t>(Action::KUNAI)] = { Joypad::B };
    m_actionPadMap[static_cast<size_t>(Action::KAMAE)] = { Joypad::LB };
    m_actionPadMap[static_cast<size_t>(Action::G)] = { Joypad::LT };
    m_actionPadMap[static_cast<size_t>(Action::JUTSU_KAMAE)] = { Joypad::RT };
    m_actionPadMap[static_cast<size_t>(Action::JUMP)] = { Joypad::A };
    m_actionPadMap[static_cast<size_t>(Action::ESCAPE)] = { Joypad::START };
    m_actionPadMap[static_cast<size_t>(Action::ENTER)] = { Joypad::B };
    m_actionPadMap[static_cast<size_t>(Action::MENU)] = { Joypad::BACK };
    m_key.Init();
    m_pad.Init();
}

void Input::Update() {
    m_key.Update();
    m_pad.Update();

}

bool Input::IsDown(Action action) const {
    auto idx = static_cast<size_t>(action);

    // キーボード側チェック
    for (auto key : m_actionKeyMap[idx]) {
        if (m_key.IsDown(key)) return true;
    }

    // パッド側チェック
    for (auto btn : m_actionPadMap[idx]) {
        if (m_pad.IsDown(btn)) return true;
    }

    return false;
}
bool Input::IsTrigger(Action action) const {
    auto idx = static_cast<size_t>(action);

    // キーボード側チェック
    for (auto key : m_actionKeyMap[idx]) {
        if (m_key.IsTrigger(key)) return true;
    }

    // パッド側チェック
    for (auto btn : m_actionPadMap[idx]) {
        if (m_pad.IsTrigger(btn)) return true;
    }

    return false;
}

bool Input::IsRelease(Action action) const {
    auto idx = static_cast<size_t>(action);

    // キーボード側チェック
    for (auto key : m_actionKeyMap[idx]) {
        if (m_key.IsRelease(key)) return true;
    }

    // パッド側チェック
    for (auto btn : m_actionPadMap[idx]) {
        if (m_pad.IsRelease(btn)) return true;
    }

    return false;
}

int Input::GetPressFrame(Action action) const {
    auto idx = static_cast<size_t>(action);
    int frame = 0;

    // キーボード側チェック（最大値をとる）
    for (auto key : m_actionKeyMap[idx]) {
        if (m_key.GetPressFrame(key) > frame) {
            frame = m_key.GetPressFrame(key);
        }
    }

    // パッド側チェック（最大値をとる）
    for (auto btn : m_actionPadMap[idx]) {
        if (m_pad.GetPressFrame(btn) > frame) {
            frame = m_pad.GetPressFrame(btn);
        }
    }

    return frame;
}