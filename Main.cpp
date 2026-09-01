#include <memory>
#include "Game.h"

//	グローバル変数
namespace {
	//	ウィンドウのクラス名とタイトル
	const std::string WinClassName = "SampleGame";
	const std::string WinGameName = "SHINOBI";

	//フォント名
	const std::string GameFont = "x12y16pxMaruMonica";
	const std::string DebugFont = "HGSｺﾞｼｯｸE";

	//	ウィンドウの幅と長さ
	constexpr UINT WinWidth = 1280;
	constexpr UINT WinHeight = 720;
	constexpr UINT WinColor = 32;

	//	ゲームデータ
	std::unique_ptr<Game> game;
};

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hRrevInstance, _In_ LPSTR IpCmdLine, _In_ int nShowCmd) {

	//	インスタンス作成	
	game = std::make_unique<Game>();
#ifdef _DEBUG
	//	コンソール画面表示
	game->InitConsole();
#endif // _DEBUG


	//	ゲーム初期化
	if (game->Init(WinGameName, WinWidth, WinHeight, WinColor, GameFont, DebugFont)) {
		game->Run();
	}
	game->End();
	return 0;

}