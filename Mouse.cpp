#include "Mouse.h"

Mouse::Mouse()
    : m_x(0)
    , m_y(0)
    , m_leftButtonDown(false)
    , m_leftButtonOld(false)
    , m_rightButtonDown(false)
    , m_rightButtonOld(false)
{
}

void Mouse::Init() {
    m_x = 0;
    m_y = 0;
    m_leftButtonDown = false;
    m_leftButtonOld = false;
    m_rightButtonDown = false;
    m_rightButtonOld = false;
}

void Mouse::Update() {
    // 前フレームのボタン状態を保存
    m_leftButtonOld = m_leftButtonDown;
    m_rightButtonOld = m_rightButtonDown;

    // マウス座標取得
    GetMousePoint(&m_x, &m_y);

    // マウスボタン状態取得
    int button = GetMouseInput();
    m_leftButtonDown = (button & MOUSE_INPUT_LEFT) != 0;
    m_rightButtonDown = (button & MOUSE_INPUT_RIGHT) != 0;
}
