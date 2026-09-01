#pragma once
#include <DxLib.h>
#include <iostream>
#include "Renderer.h"
#include "Input.h"
#include "Scene.h"
#include <memory>

class Game
{
public:
	Game();		//	コンストラクタ
	~Game();	//	デストラクタ

	bool  Init(const std::string& title, UINT width, UINT height, UINT color, const std::string& gamefont, const std::string& debugfont);
	bool Run();
	void End();

	void InitConsole();

	//bool tick(float& deltaTime, int targetFPS = 60, float maxDeltaTime = 0.1f);

	bool IsRunning() const { return m_running; }
	int GetWidth() const { return m_winWidth; }
	int GetHeight() const { return m_winHeight; }

	const Renderer* GetRenderer() const { return m_renderer.get(); }
	Renderer* GetRenderer() { return m_renderer.get(); }
	const Input& GetInput() const { return m_input; }
	const std::string& GatGameFont() const { return m_gameFont; }
	const std::string& GatDebugFont() const { return m_debugFont; }

	void ChangeScene(std::unique_ptr<Scene>nextScene); //クリアシーンのために追加 シーンを外部から切り替えるための関数

private:

	void Update(float deltaTime);
	void Draw();

	HWND m_window;
	std::unique_ptr<Renderer> m_renderer;
	Input m_input;

	std::unique_ptr<Scene> m_scene;
	std::unique_ptr<Scene> m_nextScene{ nullptr };//クリアシーンのために追加

	bool m_running;
	bool m_ended;

	int m_winWidth;
	int m_winHeight;
	int m_winColor;
	std::string m_gameFont;
	std::string m_debugFont;


	//	FPS計算用
	float m_fps = 0.0f;
	LARGE_INTEGER m_prevTime;
	LARGE_INTEGER m_freq;
	float m_accumulator;

};