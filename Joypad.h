#pragma once
#include <DxLib.h>
#include <array>
#include <cmath>
#include "Vector2d.h"

class Joypad
{
public:
	enum ButtonIndex {

		UP = 0,
		DOWN,
		LEFT,
		RIGHT,
		A,
		B,
		X,
		Y,
		LB,
		RB,
		BACK,
		START,
		LT,
		RT,
		BUTTON_COUNT

	};


	void Init(int padNumber = 0);
	void Update();

	bool IsDown(int button) const { return m_nowButtons[button]; }
	bool IsTrigger(int button) const { return m_nowButtons[button] && !m_oldButtons[button]; }
	bool IsRelease(int button) const { return !m_nowButtons[button] && m_oldButtons[button]; }
	int GetPressFrame(int button) const { return m_pressFrames[button]; }

	const Vector2d& GetStickL() const { return m_stickL; }
	const Vector2d& GetStickR() const { return m_stickR; }
	const float& GetStickLT() const { return m_LT; }
	const float& GetStickRT() const { return m_RT; }

private:
	int m_padNumber = 0;  // 使用するパッド番号（通常0）
	std::array<bool, BUTTON_COUNT> m_nowButtons{};
	std::array<bool, BUTTON_COUNT> m_oldButtons{};
	std::array<int, BUTTON_COUNT> m_pressFrames{};
	std::array<int, BUTTON_COUNT> m_buttonMap{}; // DxLibのボタン対応表

	Vector2d m_stickL = Vector2d::Zero();
	Vector2d m_stickR = Vector2d::Zero();

	float m_LT = 0.0f;
	float m_RT = 0.0f;

	float m_deadZone = 0.2f;
};