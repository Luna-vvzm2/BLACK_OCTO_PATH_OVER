#pragma once
#include "Input.h"
#include <DxLib.h>

class Mouse {
public:
    Mouse();
    void Init();
    void Update();

    // マウス座標取得
    int GetX() const { return m_x; }
    int GetY() const { return m_y; }

    // マウスボタン状態
    bool IsLeftDown() const { return m_leftButtonDown; }
    bool IsLeftTrigger() const { return m_leftButtonDown && !m_leftButtonOld; }
    bool IsRightDown() const { return m_rightButtonDown; }
    bool IsRightTrigger() const { return m_rightButtonDown && !m_rightButtonOld; }

private:
    int m_x;
    int m_y;
    bool m_leftButtonDown;
    bool m_leftButtonOld;
    bool m_rightButtonDown;
    bool m_rightButtonOld;
};
