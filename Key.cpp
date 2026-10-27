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
	m_keyMap[E] = KEY_INPUT_E;
	m_keyMap[F] = KEY_INPUT_F;
	m_keyMap[G] = KEY_INPUT_G;
	m_keyMap[H] = KEY_INPUT_H;
	m_keyMap[M] = KEY_INPUT_M;
	m_keyMap[ESCAPE] = KEY_INPUT_ESCAPE;
	m_keyMap[SPACE] = KEY_INPUT_SPACE;
	m_keyMap[ENTER] = KEY_INPUT_RETURN;
	m_keyMap[PAGE_UP] = KEY_INPUT_PGUP;
	m_keyMap[PAGE_DOWN] = KEY_INPUT_PGDN;
	m_keyMap[NUM_4] = KEY_INPUT_4;  // デバッグ用の4キー

    // 追加：英字キー
    m_keyMap[B] = KEY_INPUT_B;
    m_keyMap[C] = KEY_INPUT_C;
    m_keyMap[J] = KEY_INPUT_J;
    m_keyMap[K] = KEY_INPUT_K;
    m_keyMap[L] = KEY_INPUT_L;
    m_keyMap[N] = KEY_INPUT_N;
    m_keyMap[R] = KEY_INPUT_R;
    m_keyMap[T] = KEY_INPUT_T;
    m_keyMap[V] = KEY_INPUT_V;
    m_keyMap[X] = KEY_INPUT_X;
    m_keyMap[Y] = KEY_INPUT_Y;
    m_keyMap[Z] = KEY_INPUT_Z;

    // 追加：数字キー
    m_keyMap[DIGIT_0] = KEY_INPUT_0;
    m_keyMap[DIGIT_1] = KEY_INPUT_1;
    m_keyMap[DIGIT_2] = KEY_INPUT_2;
    m_keyMap[DIGIT_3] = KEY_INPUT_3;
    m_keyMap[DIGIT_5] = KEY_INPUT_5;
    m_keyMap[DIGIT_6] = KEY_INPUT_6;
    m_keyMap[DIGIT_7] = KEY_INPUT_7;
    m_keyMap[DIGIT_8] = KEY_INPUT_8;
    m_keyMap[DIGIT_9] = KEY_INPUT_9;

    // 追加：ファンクションキー
    m_keyMap[F1] = KEY_INPUT_F1;
    m_keyMap[F2] = KEY_INPUT_F2;
    m_keyMap[F3] = KEY_INPUT_F3;
    m_keyMap[F4] = KEY_INPUT_F4;
    m_keyMap[F5] = KEY_INPUT_F5;
    m_keyMap[F6] = KEY_INPUT_F6;
    m_keyMap[F7] = KEY_INPUT_F7;
    m_keyMap[F8] = KEY_INPUT_F8;
    m_keyMap[F9] = KEY_INPUT_F9;
    m_keyMap[F10] = KEY_INPUT_F10;
    m_keyMap[F11] = KEY_INPUT_F11;
    m_keyMap[F12] = KEY_INPUT_F12;

    // 追加：その他
    m_keyMap[TAB] = KEY_INPUT_TAB;
    m_keyMap[BACKSPACE] = KEY_INPUT_BACK;

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

const char* Key::GetKeyName(int key)
{
    switch (key)
    {
    case UP:        return "UP";
    case DOWN:      return "DOWN";
    case LEFT:      return "LEFT";
    case RIGHT:     return "RIGHT";

    case W:         return "W";
    case A:         return "A";
    case S:         return "S";
    case D:         return "D";
    case U:         return "U";
    case I:         return "I";
    case O:         return "O";
    case P:         return "P";
    case Q:         return "Q";
    case E:         return "E";
    case F:         return "F";
    case G:         return "G";
    case H:         return "H";
    case M:         return "M";

    case B:         return "B";
    case C:         return "C";
    case J:         return "J";
    case K:         return "K";
    case L:         return "L";
    case N:         return "N";
    case R:         return "R";
    case T:         return "T";
    case V:         return "V";
    case X:         return "X";
    case Y:         return "Y";
    case Z:         return "Z";

    case ESCAPE:    return "ESC";
    case SPACE:     return "SPACE";
    case ENTER:     return "ENTER";
    case PAGE_UP:   return "PAGE UP";
    case PAGE_DOWN: return "PAGE DOWN";
    case NUM_4:     return "4";

    case DIGIT_0:   return "0";
    case DIGIT_1:   return "1";
    case DIGIT_2:   return "2";
    case DIGIT_3:   return "3";
    case DIGIT_5:   return "5";
    case DIGIT_6:   return "6";
    case DIGIT_7:   return "7";
    case DIGIT_8:   return "8";
    case DIGIT_9:   return "9";

    case F1:        return "F1";
    case F2:        return "F2";
    case F3:        return "F3";
    case F4:        return "F4";
    case F5:        return "F5";
    case F6:        return "F6";
    case F7:        return "F7";
    case F8:        return "F8";
    case F9:        return "F9";
    case F10:       return "F10";
    case F11:       return "F11";
    case F12:       return "F12";

    case TAB:       return "TAB";
    case BACKSPACE: return "BACKSPACE";

    default:        return "UNKNOWN";
    }
}