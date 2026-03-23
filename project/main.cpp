#include "Game.h"
#include "Framework.h"

// Windowsアプリのエントリーポイント（main関数に相当）
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// DirectXリソースリーク検出用オブジェクト
	D3DResourceLeakChecker leakCheck;

	// COMライブラリ初期化（マルチスレッド対応）
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// デバッグ出力（Visual Studioの出力ウィンドウに表示）
	OutputDebugStringA("Hello, DirectX!\n");

#pragma region 基盤システム初期化
	// Frameworkを継承したGameクラスのインスタンスを生成
	std::unique_ptr<Framework> game = std::make_unique<Game>();

	// ゲーム実行（初期化 → メインループ → 終了処理）
	game->Run();
#pragma endregion

	return 0;
}
