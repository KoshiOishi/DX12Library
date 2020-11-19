﻿#include <Windows.h>
#include <wrl.h>
#include "Input.h"
#include "Object3D.h"
#include "Sprite.h"
#include "DebugText.h"
#include "Sound.h"
#include "DX12Init.h"
#include "SceneManager.h"
#include "GamePlay.h"

using namespace DirectX;
using namespace Microsoft::WRL;

HRESULT result;

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

#ifdef _DEBUG
	//デバッグレイヤーをオンに
	ComPtr < ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
#endif

	DX12Init::Initialize();
	Input::Initialize(DX12Init::GetHwnd());
	Sound::Initialize();
	Sprite::FirstInit();
	Object3D::FirstInit();

	//デバッグテキスト初期化
	DebugText::Initialize(0, L"Resources/debugfont.png");

	SceneManager::AddScene(new GamePlay());
	SceneManager::SetScene("GamePlay");

	MSG msg{}; // メッセージ
	while (true)
	{
		// メッセージがある?
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg); // キー入力メッセージの処理
			DispatchMessage(&msg); // プロシージャにメッセージを送る
		}
		// 終了メッセージが来たらループを抜ける
		if (msg.message == WM_QUIT) {
			break;
		}

		SceneManager::Update();
		SceneManager::Draw();

	}
	// ウィンドウクラスを登録解除
	DX12Init::End();
}

