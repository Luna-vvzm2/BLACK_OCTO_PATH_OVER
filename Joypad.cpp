#include "Joypad.h"

void Joypad::Init(int padNumber) {
	m_padNumber = padNumber;

	m_buttonMap[UP] = PAD_INPUT_UP;
	m_buttonMap[DOWN] = PAD_INPUT_DOWN;
	m_buttonMap[LEFT] = PAD_INPUT_LEFT;
	m_buttonMap[RIGHT] = PAD_INPUT_RIGHT;
	m_buttonMap[A] = PAD_INPUT_1;
	m_buttonMap[B] = PAD_INPUT_2;
	m_buttonMap[X] = PAD_INPUT_3;
	m_buttonMap[Y] = PAD_INPUT_4;
	m_buttonMap[LB] = PAD_INPUT_5;
	m_buttonMap[RB] = PAD_INPUT_6;
	m_buttonMap[BACK] = PAD_INPUT_7;
	m_buttonMap[START] = PAD_INPUT_8;

	// èâä˙èÛë‘
	int state = GetJoypadInputState(DX_INPUT_PAD1 + padNumber);
	for (int i = 0; i < BUTTON_COUNT; i++) {
		m_nowButtons[i] = (state & m_buttonMap[i]) != 0;
		m_oldButtons[i] = m_nowButtons[i];
		m_pressFrames[i] = 0;
	}
	m_stickL = { 0.0f, 0.0f };
}

void Joypad::Update() {
	int state = GetJoypadInputState(DX_INPUT_PAD1 + m_padNumber);
	for (int i = 0; i < BUTTON_COUNT; i++) {
		m_oldButtons[i] = m_nowButtons[i];
		m_nowButtons[i] = (state & m_buttonMap[i]) != 0;
		m_pressFrames[i] = m_nowButtons[i] ? m_pressFrames[i] + 1 : 0;
	}

	int LX = 0, LY = 0;
	int RX = 0, RY = 0;

	int inputType = DX_INPUT_PAD1 + m_padNumber; // DX_INPUT_PAD1, DX_INPUT_PAD2, ...

	XINPUT_STATE xi;
	if (GetJoypadXInputState(DX_INPUT_PAD1 + m_padNumber, &xi) == 0) {
		m_LT = xi.LeftTrigger / 255.0f; // 0.0f Å` 1.0f
		m_RT = xi.RightTrigger / 255.0f; // 0.0f Å` 1.0f

		// àÍíËà»è„âüÇµÇΩÇÁÅuâüÇµÇƒÇ¢ÇÈÅvÇ∆îªíË
		m_nowButtons[LT] = m_LT > 0.2f;
		m_nowButtons[RT] = m_RT > 0.2f;
	}
	else {
		m_LT = 0.0f;
		m_RT = 0.0f;

		m_nowButtons[LT] = false;
		m_nowButtons[RT] = false;
	}

	GetJoypadAnalogInput(&LX, &LY, inputType);
	GetJoypadAnalogInputRight(&RX, &RY, inputType);

	// ê≥ãKâª (-1.0Å`1.0)
	m_stickL = Vector2d(LX / 1000.0f, LY / 1000.0f);
	m_stickR = Vector2d(RX / 1000.0f, RY / 1000.0f);

	// ÉfÉbÉhÉ]Å[Éìèàóù
	if (m_stickL.length() < m_deadZone) {
		m_stickL = Vector2d::Zero();
	}
	else {
		m_stickL = m_stickL.normalize() * ((m_stickL.length() - m_deadZone) / (1.0f - m_deadZone));
	}
	if (m_stickR.length() < m_deadZone) {
		m_stickR = Vector2d::Zero();
	}
	else {
		m_stickR = m_stickR.normalize() * ((m_stickR.length() - m_deadZone) / (1.0f - m_deadZone));
	}
}