#pragma once 
#include <DxLib.h> 
#include <array> 
class Key {
public:
	enum KeyIndex {
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
		G,
		H,
		M,
		ESCAPE,
		SPACE,
		ENTER,
		NUM_4,
		KEY_COUNT
	};

	void Init();
	void Update();
	// ƒL[‚ª‰Ÿ‚³‚ê‚Ä‚¢‚éŠÔ(‰Ÿ‚³‚ê‚Ä‚¢‚½‚çture) 
	bool IsDown(int key) const { return m_nowKeys[key]; }
	bool IsTrigger(int key) const { return m_nowKeys[key] && !m_oldKeys[key]; }
	bool IsRelease(int key) const { return !m_nowKeys[key] && m_oldKeys[key]; }
	int GetPressFrame(int key) const { return m_pressFrames[key]; }
private:
	std::array<UINT8, KEY_COUNT> m_nowKeys{};
	std::array<UINT8, KEY_COUNT> m_oldKeys{};
	std::array<int, KEY_COUNT> m_pressFrames{};
	std::array<int, KEY_COUNT> m_keyMap{};
};