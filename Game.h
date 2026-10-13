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
	void RequestQuit() { m_running = false; }
	int GetVolume(int channel) const;
	void AdjustVolume(int channel, int amount);
	int GetWidth() const { return m_winWidth; }
	int GetHeight() const { return m_winHeight; }

	const Renderer* GetRenderer() const { return m_renderer.get(); }
	Renderer* GetRenderer() { return m_renderer.get(); }
	const Input& GetInput() const { return m_input; }
	const std::string& GatGameFont() const { return m_gameFont; }
	const std::string& GatDebugFont() const { return m_debugFont; }

	void PushScene(std::unique_ptr<Scene> nextScene);
	void PopScene();
	bool CanPopScene() const { return m_sceneStack.size() > 1; }
	void ChangeScene(std::unique_ptr<Scene> nextScene); //クリアシーンのために追加 シーンを外部から切り替えるための関数
	void Quit() { m_running = false; }

	Scene* GetCurrentScene() const;

private:

	void Update(float deltaTime);
	void Draw();

	HWND m_window;
	std::unique_ptr<Renderer> m_renderer;
	Input m_input;

	std::vector<std::unique_ptr<Scene>> m_sceneStack;
	enum class PendingAction
	{
		None,
		Push,
		Pop,
		Change
	};
	PendingAction m_pendingAction;
	std::unique_ptr<Scene> m_pendingScene;

	void ProccessPendingActions();

	bool m_running;
	bool m_ended;

	int m_winWidth;
	int m_winHeight;
	int m_winColor;
	std::string m_gameFont;
	std::string m_debugFont;
	int m_volumes[3] = { 100, 100, 100 }; // master, effects, BGM

	//	FPS計算用
	float m_fps = 0.0f;
	LARGE_INTEGER m_prevTime;
	LARGE_INTEGER m_freq;
	float m_accumulator;

};