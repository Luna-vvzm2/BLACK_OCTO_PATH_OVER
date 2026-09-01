#include "Key.h"

void Key::Init() {
	// m_keyMapに各キーのDxLib定数を設定 
	m_keyMap[UP] = KEY_INPUT_UP;
	m_keyMap[DOWN] = KEY_INPUT_DOWN;
	m_keyMap[LEFT] = KEY_INPUT_LEFT;
	m_keyMap[RIGHT] = KEY_INPUT_RIGHT;
	m_keyMap[W] = KEY_INPUT_W;
	m_keyMap[A] = KEY_INPUT_A;
	m_keyMap[S] = KEY_INPUT_S;
	m_keyMap[D] = KEY_INPUT_D;
	m_keyMap[U] = KEY_INPUT_U;
	m_keyMap[I] = KEY_INPUT_I;
	m_keyMap[O] = KEY_INPUT_O;
	m_keyMap[P] = KEY_INPUT_P;
	m_keyMap[Q] = KEY_INPUT_Q;
	m_keyMap[G] = KEY_INPUT_G;
	m_keyMap[H] = KEY_INPUT_H;
	m_keyMap[M] = KEY_INPUT_M;
	m_keyMap[ESCAPE] = KEY_INPUT_ESCAPE;
	m_keyMap[SPACE] = KEY_INPUT_SPACE;
	m_keyMap[ENTER] = KEY_INPUT_RETURN;
	m_keyMap[NUM_4] = KEY_INPUT_4;  // デバッグ用の4キー


	for (int i = 0; i < KEY_COUNT; i++) {
		m_nowKeys[i] = CheckHitKey(m_keyMap[i]) != 0;
		m_oldKeys[i] = m_nowKeys[i]; m_pressFrames[i] = 0;
	}
}

void Key::Update() {
	for (int i = 0; i < KEY_COUNT; i++) {
		m_oldKeys[i] = m_nowKeys[i];
		m_nowKeys[i] = CheckHitKey(m_keyMap[i]) != 0;
		m_pressFrames[i] = m_nowKeys[i] ? m_pressFrames[i] + 1 : 0;
	}
}
