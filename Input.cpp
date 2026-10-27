#include "Input.h"
#include <algorithm>

Input::Input() {
    Init();
}

void Input::Init() {
    m_actionInputModes.fill(InputMode::Hold);
    m_toggleStates.fill(false);

    // アクションとキーを対応付け
    m_actionKeyMap[static_cast<size_t>(Action::UP)] =
    { Key::W, Key::UP };

    m_actionKeyMap[static_cast<size_t>(Action::DOWN)] =
    { Key::S, Key::DOWN };

    m_actionKeyMap[static_cast<size_t>(Action::LEFT)] =
    { Key::A, Key::LEFT };

    m_actionKeyMap[static_cast<size_t>(Action::RIGHT)] =
    { Key::D, Key::RIGHT };
    m_actionKeyMap[static_cast<size_t>(Action::DASH)] = { Key::U };
    m_actionKeyMap[static_cast<size_t>(Action::WEAK_ATTACK)] = { Key::I };
    m_actionKeyMap[static_cast<size_t>(Action::KUNAI)] = { Key::O };
    m_actionKeyMap[static_cast<size_t>(Action::HEAL)] = { Key::P };
    m_actionKeyMap[static_cast<size_t>(Action::KAMAE)] = { Key::Q };
    m_actionKeyMap[static_cast<size_t>(Action::G)] = { Key::G };
    m_actionKeyMap[static_cast<size_t>(Action::JUTSU_KAMAE)] = { Key::H };
    m_actionKeyMap[static_cast<size_t>(Action::OCTO)] = { Key::Q };
    m_actionKeyMap[static_cast<size_t>(Action::TIGER)] = { Key::E };
    m_actionKeyMap[static_cast<size_t>(Action::ORCA)] = { Key::F };
    m_actionKeyMap[static_cast<size_t>(Action::JUMP)] = { Key::SPACE };
    m_actionKeyMap[static_cast<size_t>(Action::ESCAPE)] = { Key::ESCAPE };
    m_actionKeyMap[static_cast<size_t>(Action::ENTER)] = { Key::ENTER };
    m_actionKeyMap[static_cast<size_t>(Action::MENU)] = { Key::M };
    m_actionKeyMap[static_cast<size_t>(Action::PAGE_UP)] = { Key::PAGE_UP };
    m_actionKeyMap[static_cast<size_t>(Action::PAGE_DOWN)] = { Key::PAGE_DOWN };
    m_actionKeyMap[static_cast<size_t>(Action::ZOOM_IN)] = { Key::Q };
    m_actionKeyMap[static_cast<size_t>(Action::ZOOM_OUT)] = { Key::E };

    // ジョイパッドも同様に
    m_actionPadMap[static_cast<size_t>(Action::UP)] = { Joypad::UP };
    m_actionPadMap[static_cast<size_t>(Action::DOWN)] = { Joypad::DOWN };
    m_actionPadMap[static_cast<size_t>(Action::LEFT)] = { Joypad::LEFT };
    m_actionPadMap[static_cast<size_t>(Action::RIGHT)] = { Joypad::RIGHT };
    m_actionPadMap[static_cast<size_t>(Action::DASH)] = { Joypad::RB };
    m_actionPadMap[static_cast<size_t>(Action::WEAK_ATTACK)] = { Joypad::X };
    m_actionPadMap[static_cast<size_t>(Action::KUNAI)] = { Joypad::B };
    m_actionPadMap[static_cast<size_t>(Action::HEAL)] = { Joypad::X };
    m_actionPadMap[static_cast<size_t>(Action::KAMAE)] = { Joypad::LB };
    m_actionPadMap[static_cast<size_t>(Action::G)] = { Joypad::LT };
    m_actionPadMap[static_cast<size_t>(Action::JUTSU_KAMAE)] = { Joypad::RT };
    m_actionPadMap[static_cast<size_t>(Action::OCTO)] = {};
    m_actionPadMap[static_cast<size_t>(Action::TIGER)] = {};
    m_actionPadMap[static_cast<size_t>(Action::ORCA)] = {};
    m_actionPadMap[static_cast<size_t>(Action::JUMP)] = { Joypad::A };
    m_actionPadMap[static_cast<size_t>(Action::ESCAPE)] = { Joypad::START };
    m_actionPadMap[static_cast<size_t>(Action::ENTER)] = { Joypad::B };
    m_actionPadMap[static_cast<size_t>(Action::MENU)] = { Joypad::BACK };
    m_actionPadMap[static_cast<size_t>(Action::PAGE_UP)] = { Joypad::LB };
    m_actionPadMap[static_cast<size_t>(Action::PAGE_DOWN)] = { Joypad::RB };
    m_actionPadMap[static_cast<size_t>(Action::ZOOM_IN)] = { Joypad::RS_UP };
    m_actionPadMap[static_cast<size_t>(Action::ZOOM_OUT)] = { Joypad::RS_DOWN };
    m_key.Init();
    m_pad.Init();
}

void Input::Update() {
    m_key.Update();
    m_pad.Update();

    // トグル方式の操作は、押すたびにON/OFFを切り替える
    for (size_t i = 0;
        i < static_cast<size_t>(Action::ACTION_COUNT);
        ++i)
    {
        if (m_actionInputModes[i] == InputMode::Toggle &&
            IsTrigger(static_cast<Action>(i)))
        {
            m_toggleStates[i] = !m_toggleStates[i];
        }
    }
}

bool Input::IsDown(Action action) const {
    const size_t index = static_cast<size_t>(action);

    // キーボード側チェック
    if (index >= static_cast<size_t>(Action::ACTION_COUNT))
        return false;

    // トグル方式では、押した状態を保持する
    if (m_actionInputModes[index] == InputMode::Toggle)
        return m_toggleStates[index];

    // 長押し方式では、従来どおり実際の入力を確認する
    for (auto key : m_actionKeyMap[index])
    {
        if (m_key.IsDown(key))
            return true;
    }

    // パッド側チェック
    for (auto button : m_actionPadMap[index])
    {
        if (m_pad.IsDown(button))
            return true;
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

    // LT + 各ボタン
    if (action == Action::OCTO) {
        return m_pad.IsDown(Joypad::LT) &&
            m_pad.IsTrigger(Joypad::X);
    }

    if (action == Action::TIGER) {
        return m_pad.IsDown(Joypad::LT) &&
            m_pad.IsTrigger(Joypad::Y);
    }

    if (action == Action::ORCA) {
        return m_pad.IsDown(Joypad::LT) &&
            m_pad.IsTrigger(Joypad::B);
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

int Input::GetKeyBinding(Action action) const
{
    const size_t index = static_cast<size_t>(action);

    if (index >= static_cast<size_t>(Action::ACTION_COUNT))
        return -1;

    const auto& keys = m_actionKeyMap[index];

    if (keys.empty())
        return -1;

    return static_cast<int>(keys[0]);
}

bool Input::SetKeyBinding(Action action, int keyIndex)
{
    const size_t index = static_cast<size_t>(action);

    if (index >= static_cast<size_t>(Action::ACTION_COUNT))
        return false;

    if (keyIndex < 0 || keyIndex >= Key::KEY_COUNT)
        return false;

    auto& keys = m_actionKeyMap[index];

    keys.clear();
    keys.push_back(static_cast<Key::KeyIndex>(keyIndex));

    return true;
}

InputMode Input::GetInputMode(Action action) const
{
    const size_t index = static_cast<size_t>(action);

    if (index >= static_cast<size_t>(Action::ACTION_COUNT))
        return InputMode::Hold;

    return m_actionInputModes[index];
}

void Input::SetInputMode(Action action, InputMode mode)
{
    const size_t index = static_cast<size_t>(action);

    if (index >= static_cast<size_t>(Action::ACTION_COUNT))
        return;

    m_actionInputModes[index] = mode;

    // 方式を変更したときに、ON状態が残らないようにする
    m_toggleStates[index] = false;
}

std::vector<int> Input::GetKeyBindings(Action action) const
{
    const size_t index = static_cast<size_t>(action);

    if (index >= static_cast<size_t>(Action::ACTION_COUNT))
        return {};

    std::vector<int> result;

    for (auto key : m_actionKeyMap[index])
    {
        result.push_back(static_cast<int>(key));
    }

    return result;
}

bool Input::SetKeyBindings(
    Action action,
    const std::vector<int>& keyIndices)
{
    const size_t index = static_cast<size_t>(action);

    if (index >= static_cast<size_t>(Action::ACTION_COUNT))
        return false;

    // 空の割り当ては受け付けない
    if (keyIndices.empty())
        return false;

    std::vector<Key::KeyIndex> newKeys;

    for (int keyIndex : keyIndices)
    {
        if (keyIndex < 0 || keyIndex >= Key::KEY_COUNT)
            return false;

        auto key = static_cast<Key::KeyIndex>(keyIndex);

        // 同じキーが重複しないようにする
        if (std::find(newKeys.begin(), newKeys.end(), key)
            == newKeys.end())
        {
            newKeys.push_back(key);
        }
    }

    m_actionKeyMap[index] = std::move(newKeys);

    return true;
}

bool Input::IsMenuDown(Action action) const
{
    switch (action)
    {
    case Action::UP:
        return m_key.IsDown(Key::W) ||
            m_key.IsDown(Key::UP);

    case Action::DOWN:
        return m_key.IsDown(Key::S) ||
            m_key.IsDown(Key::DOWN);

    case Action::LEFT:
        return m_key.IsDown(Key::A) ||
            m_key.IsDown(Key::LEFT);

    case Action::RIGHT:
        return m_key.IsDown(Key::D) ||
            m_key.IsDown(Key::RIGHT);

    case Action::ENTER:
        return m_key.IsDown(Key::ENTER);

    case Action::ESCAPE:
        return m_key.IsDown(Key::ESCAPE);

    case Action::MENU:
        return m_key.IsDown(Key::M);

    default:
        return false;
    }
}

bool Input::IsMenuTrigger(Action action) const
{
    switch (action)
    {
    case Action::UP:
        return m_key.IsTrigger(Key::W) ||
            m_key.IsTrigger(Key::UP);

    case Action::DOWN:
        return m_key.IsTrigger(Key::S) ||
            m_key.IsTrigger(Key::DOWN);

    case Action::LEFT:
        return m_key.IsTrigger(Key::A) ||
            m_key.IsTrigger(Key::LEFT);

    case Action::RIGHT:
        return m_key.IsTrigger(Key::D) ||
            m_key.IsTrigger(Key::RIGHT);

    case Action::ENTER:
        return m_key.IsTrigger(Key::ENTER);

    case Action::ESCAPE:
        return m_key.IsTrigger(Key::ESCAPE);

    case Action::MENU:
        return m_key.IsTrigger(Key::M);

    default:
        return false;
    }
}