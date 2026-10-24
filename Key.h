#pragma once

#include <DxLib.h>
#include <array>

class Key {
public:
    enum KeyIndex {
        // 既存のキー番号は順番を変えない
        UP = 0,
        DOWN,
        LEFT,
        RIGHT,
        W,
        A,
        S,
        D,
        U,
        I,
        O,
        P,
        Q,
        E,
        F,
        G,
        H,
        M,
        ESCAPE,
        SPACE,
        ENTER,
        PAGE_UP,
        PAGE_DOWN,
        NUM_4,

        // 追加：英字キー
        B,
        C,
        J,
        K,
        L,
        N,
        R,
        T,
        V,
        X,
        Y,
        Z,

        // 追加：数字キー（4は既存のNUM_4を使用）
        DIGIT_0,
        DIGIT_1,
        DIGIT_2,
        DIGIT_3,
        DIGIT_5,
        DIGIT_6,
        DIGIT_7,
        DIGIT_8,
        DIGIT_9,

        // 追加：ファンクションキー
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        // 追加：その他
        TAB,
        BACKSPACE,

        KEY_COUNT
    };

    void Init();
    void Update();

    // キーが押されている間
    bool IsDown(int key) const {
        return m_nowKeys[key];
    }

    // キーを押した瞬間
    bool IsTrigger(int key) const {
        return m_nowKeys[key] && !m_oldKeys[key];
    }

    // キーを離した瞬間
    bool IsRelease(int key) const {
        return !m_nowKeys[key] && m_oldKeys[key];
    }

    int GetPressFrame(int key) const {
        return m_pressFrames[key];
    }

    // 最初に押されたキーの番号を取得する
    // 該当するキーがなければ -1
    int GetFirstTriggeredKey() const {
        for (int i = 0; i < KEY_COUNT; ++i) {
            if (IsTrigger(i)) {
                return i;
            }
        }
        return -1;
    }

    // キー番号を画面表示用の名前に変換する
    static const char* GetKeyName(int key);

private:
    std::array<UINT8, KEY_COUNT> m_nowKeys{};
    std::array<UINT8, KEY_COUNT> m_oldKeys{};
    std::array<int, KEY_COUNT> m_pressFrames{};
    std::array<int, KEY_COUNT> m_keyMap{};
};