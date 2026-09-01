#include "Game.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "TitleScene.h"
#include "PlayScene.h"
#include "SpriteComponent.h"

Game::Game()
	: m_window(nullptr),
	m_winWidth(0),
	m_winHeight(0),
	m_winColor(0),
	m_fps(0.0f),
	m_accumulator(0.0f),
	m_freq{},
	m_prevTime{},
	m_running(true),
	m_ended(false)
{
}

Game::~Game() {


;
}

bool Game::Init(const std::string& title, UINT width, UINT height, UINT color, const std::string& gamefont, const std::string& debugfont) {
	//	ウィンドウモードに変更
	ChangeWindowMode(TRUE);

	//	画面モード設定
	SetGraphMode(width, height, color);
	SetBackgroundColor(0, 0, 0);
	SetWaitVSyncFlag(TRUE);

	// ウィンドウタイトル設定
	SetWindowTextA(m_window, title.c_str());
	//	DirectInput系コントローラなどXInput系を使う設定
	SetUseXInputFlag(TRUE);
	//	初期化
	//	DxLib初期化
	if (DxLib_Init() != 0) {
		std::cerr << "DxLib初期化失敗\n";
		MessageBoxA(m_window, "DxLib初期化失敗", "エラー", MB_OK);
		return false;
	}

	m_window = GetMainWindowHandle();
	//	Renderer初期化
	m_renderer = std::make_unique<Renderer>(m_window, width, height);
	if (!m_renderer) {
		std::cerr << "Renderer初期化失敗\n";
		MessageBoxA(m_window, "Renderer初期化失敗", "エラー", MB_OK);
		return false;
	}

	m_input.Init();



	SetDrawScreen(DX_SCREEN_BACK);
	m_winWidth = width;
	m_winHeight = height;
	m_winColor = color;
	m_gameFont = gamefont;
	m_debugFont = debugfont;
	m_running = true;

	m_scene = std::make_unique<TitleScene>(this);
	if (!m_scene->Init()) {
		std::cerr << "TitleScene 初期化失敗" << std::endl;
		return false;
	}

	QueryPerformanceFrequency(&m_freq);
	QueryPerformanceCounter(&m_prevTime);

	std::cout << "アプリ初期化成功\n";
	return true;

}

bool Game::Run() {
	const float FIXED_DT = 1.0f / 60.0f;

	while (ProcessMessage() == 0 && m_running) {
		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);

		float frameTime =
			float(currentTime.QuadPart - m_prevTime.QuadPart)
			/ float(m_freq.QuadPart);

		m_prevTime = currentTime;

		frameTime = min(frameTime, 0.25f);

		m_accumulator += frameTime;

		while (m_accumulator >= FIXED_DT)
		{
			m_input.Update();
			Update(FIXED_DT);
			m_accumulator -= FIXED_DT;
		}

		Draw();

		m_fps =
			(frameTime > 0.0f)
			? 1.0f / frameTime
			: 0.0f;
	}

	return m_scene->IsRunning();
}

void Game::Update(float deltaTime) {
	//	ESCキーで終了

	if (m_input.IsTrigger(Action::ESCAPE)) {
		switch (m_scene->GetType())
		{
		case Scene::Type::Title:
			m_running = false;
			break;

		case Scene::Type::Play:
			m_scene = std::make_unique<TitleScene>(this);
			m_scene->Init();
			break;
		}
	}


	//	シーン遷移
	if (m_input.IsTrigger(Action::ENTER)) {
		switch (m_scene->GetType())
		{
		case Scene::Type::Title:
			m_scene = std::make_unique<PlayScene>(this);
			m_scene->Init();
			break;

		case Scene::Type::Clear:
			m_scene = std::make_unique<TitleScene>(this);
			m_scene->Init();
			break;

		case Scene::Type::Play:
			auto playScene = static_cast<PlayScene*>(m_scene.get());

			//	Ball の HP が 0（＝リザルト中）のときだけ反応
			if (playScene->IsResult()) {
				m_scene = std::make_unique<PlayScene>(this);
				m_scene->Init();
			}
			break;
		}
	}


	// シーン更新
	if (m_scene) { m_scene->Update(deltaTime); }

	//	以後処理を書く

	//クリアシーンのために追加
	if (m_nextScene)
	{
		m_scene = std::move(m_nextScene);
		m_scene->Init();
	}
}

void Game::Draw() {
	ClearDrawScreen();
	//	以後描画処理を書く
	if (m_scene) m_scene->Draw();

#ifdef _DEBUG

	Vector2d ls = m_input.GetPad().GetStickL();
	Vector2d rs = m_input.GetPad().GetStickR();
	// FPS描画
	std::vector<NumberInfo> GameInfo = {
		{ m_fps, 2 },
		{ ls.x, 1 },
		{ ls.y, 1 },
		{ rs.x, 1 },
		{ rs.y, 1 }
	};
	m_renderer->DrawNumberFormatW(Vector2d(m_winWidth - 150.0f, 0), Color(255, 255, 0), m_debugFont, 20, "\nFPS: {0}\nLX: {1}\nLY: {2}\nRX: {3}\nRY: {4}", GameInfo, false);


	if (m_input.GetPad().IsDown(Joypad::UP)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "UP", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::DOWN)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "DOWN", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::LEFT)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "LEFT", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::RIGHT)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "RIGHT", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::A)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "A", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::B)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "B", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::X)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "X", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::Y)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "Y", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::LB)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "LB", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::RB)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "RB", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::BACK)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "BACK", Color(255, 64, 0), m_debugFont, 24, false);
	if (m_input.GetPad().IsDown(Joypad::START)) m_renderer->DrawTextL(Vector2d(150.0f, 0), "START", Color(255, 64, 0), m_debugFont, 24, false);

	if (!m_renderer) return;

	std::vector<NumberInfo> stickIndex = {
		{ m_input.GetPad().GetStickLT(), 1},
		{ m_input.GetPad().GetStickRT(), 1 }
	};

	m_renderer->DrawNumberFormatW(Vector2d(m_winWidth - 150.0f, 110), Color(255, 255, 0), m_debugFont, 20, "\nLT: {0}\nRT: {1} ", stickIndex, false);

#endif // _DEBUG

	ScreenFlip();
}

void Game::End() {
	if (m_ended)
	{
		return;
	}

	m_ended = true;

	if (m_scene) {
		m_scene.reset();
	}
	if (m_renderer) {
		m_renderer.reset();
	}
	SpriteComponent::ReleaseTextures();
	DxLib_End();	//	DxLib終了処理
	std::cout << "アプリ終了" << std::endl;
}

void Game::InitConsole() {
	// すでにコンソールが存在する場合は何もしない
	if (GetConsoleWindow()) return;
	//	コンソール作成
	AllocConsole();
	//	標準出力とエラー出力と標準入力をコンソールから受け取る
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);

	//	日本語表示対応
	SetConsoleOutputCP(932);

	std::cout << "コンソール初期化完了" << std::endl;


}

//クリアシーンのために追加
void Game::ChangeScene(std::unique_ptr<Scene>nextScene)
{
	if (!nextScene) return;

	m_nextScene = std::move(nextScene);
}

//bool Game::tick(float& deltaTime, int targetFPS, float maxDeltaTime) {
//	LARGE_INTEGER currentTime;
//	QueryPerformanceCounter(&currentTime);
//
//	deltaTime = float(currentTime.QuadPart - m_prevTime.QuadPart) / float(m_freq.QuadPart);
//
//	float minDelta = 1.0f / targetFPS;
//	if (deltaTime < minDelta) return false; // FPS制御
//
//	if (deltaTime > maxDeltaTime) deltaTime = maxDeltaTime;
//
//	m_prevTime = currentTime;
//	return true;
//}