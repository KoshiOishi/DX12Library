#define DIRECTINPUT_VERSION	0x0800	//DirectInputのバージョン指定

#include <Windows.h>
#include <wrl.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <dinput.h>
#include "DX12Init.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

 const int DX12Init::window_width = 1280;// 横幅
 const int DX12Init::window_height = 720; // 縦幅

 WNDCLASSEX DX12Init::w; // ウィンドウクラスの設定
 HWND DX12Init::hwnd;
 DX12Init::ComPtr<ID3D12Device> DX12Init::dev;
 DX12Init::ComPtr<IDXGIFactory6> DX12Init::dxgiFactory;
 DX12Init::ComPtr<IDXGISwapChain4> DX12Init::swapchain;
 DX12Init::ComPtr<ID3D12CommandAllocator> DX12Init::cmdAllocator;
 DX12Init::ComPtr<ID3D12GraphicsCommandList> DX12Init::cmdList;
 DX12Init::ComPtr<ID3D12CommandQueue> DX12Init::cmdQueue;
 DX12Init::ComPtr<ID3D12DescriptorHeap> DX12Init::rtvHeaps;
 D3D12_DESCRIPTOR_HEAP_DESC DX12Init::heapDesc;
 std::vector<DX12Init::ComPtr<ID3D12Resource>> DX12Init::backBuffers;
 DX12Init::ComPtr <ID3D12DescriptorHeap> DX12Init::dsvHeap;
 DX12Init::ComPtr<ID3D12Fence> DX12Init::fence;
 UINT64 DX12Init::fenceVal;
 UINT DX12Init::bbIndex;

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//メッセージで分岐
	switch (msg)
	{
	case WM_DESTROY: //ウィンドウが破棄された
		PostQuitMessage(0); //OSに対して、アプリの終了を伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam); //標準の処理を行う
}

void DX12Init::Initialize()
{
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc; // ウィンドウプロシージャを設定
	w.lpszClassName = L"DirectXGame"; // ウィンドウクラス名
	w.hInstance = GetModuleHandle(nullptr); // ウィンドウハンドル
	w.hCursor = LoadCursor(NULL, IDC_ARROW); // カーソル指定
	// ウィンドウクラスを OS に登録
	RegisterClassEx(&w);
	// ウィンドウサイズ{ X 座標 Y 座標 横幅 縦幅 }
	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); // 自動でサイズ補正
	// ウィンドウオブジェクトの生成
	hwnd = CreateWindow(w.lpszClassName, // クラス名
		L"DirectXGame", // タイトルバーの文字
		WS_OVERLAPPEDWINDOW, // 標準的なウィンドウスタイル
		CW_USEDEFAULT, // 表示 X 座標(OS に任せる)
		CW_USEDEFAULT, // 表示 Y 座標(OS に任せる)
		wrc.right - wrc.left, // ウィンドウ横幅
		wrc.bottom - wrc.top, // ウィンドウ縦幅
		nullptr, // 親ウィンドウハンドル
		nullptr, // メニューハンドル
		w.hInstance, // 呼び出しアプリケーションハンドル
		nullptr); // オプション
		// ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);

	// DirectX 初期化処理 ここから
	HRESULT result;
	dev = nullptr;

	// DXGI ファクトリーの生成
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	// アダプターの列挙用
	std::vector< ComPtr<IDXGIAdapter>> adapters;
	// ここに特定の名前を持つアダプターオブジェクトが入る
	ComPtr < IDXGIAdapter> tmpAdapter = nullptr;
	for (int i = 0;
		dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		i++)
	{
		adapters.push_back(tmpAdapter); // 動的配列に追加する
	}
	for (int i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC adesc{};
		adapters[i]->GetDesc(&adesc); // アダプターの情報を取得
		std::wstring strDesc = adesc.Description; // アダプター名
		// Microsoft Basic Render Driver を回避
		if (strDesc.find(L"Microsoft") == std::wstring::npos && strDesc.find(L"Intel") == std::wstring::npos)
		{
			tmpAdapter = adapters[i]; // 採用
			break;
		}
	}

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (int i = 0; i < _countof(levels); i++)
	{
		//採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&dev));
		if (result == S_OK)
		{
			//デバイスを生成できた時点でルー王を抜ける
			featureLevel = levels[i];
			break;
		}
	}

	// コマンドアロケータを生成
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));
	// コマンドリストを生成
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList));

	// 標準設定でコマンドキューを生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));



	// 各種設定をしてスワップチェーンを生成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = 1280;
	swapchainDesc.Height = 720;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色情報の書式
	swapchainDesc.SampleDesc.Count = 1; // マルチサンプルしない
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // バックバッファ用
	swapchainDesc.BufferCount = 2; // バッファ数を2つに設定
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // フリップ後は破棄
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//IDXGISwapChain1 のComPtrを用意
	ComPtr<IDXGISwapChain1> swapchain1;
	//スワップチェーンの生成
	result = dxgiFactory->CreateSwapChainForHwnd(
		cmdQueue.Get(),
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		&swapchain1);
	//生成したIDXGISwapChain1のオブジェクトをIDXGISwapChain4に変換する
	swapchain1.As(&swapchain);

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = swapchain->GetDesc(&swcDesc);

	// 各種設定をしてデスクリプタヒープを生成
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー
	heapDesc.NumDescriptors = 2; // 裏表の2つ
	dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	backBuffers.resize(swcDesc.BufferCount);
	// 裏表の2つ分について
	for (int i = 0; i < 2; i++)
	{
		// スワップチェーンからバッファを取得
		result = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		// ディスクリプタヒープのハンドルを取得
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle =
			CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(), i,
				dev.Get()->GetDescriptorHandleIncrementSize(heapDesc.Type));
		// レンダーターゲットビューの生成
		dev->CreateRenderTargetView(
			backBuffers[i].Get(),
			nullptr,
			handle);

	}

	// フェンスの生成
	fenceVal = 0;
	result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	//深度バッファの作成
	//リソース設定
	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		DX12Init::GetWindowWidth(),
		DX12Init::GetWindowHeight(),
		1, 0,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	//リソース生成
	ID3D12Resource* depthBuffer = nullptr;

	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値書き込みに使用
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
		IID_PPV_ARGS(&depthBuffer)
	);


	//コマンド発行のために深度ビューを生成する
	//深度ビュー用デスクリプタヒープ生成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapdesc{};
	dsvHeapdesc.NumDescriptors = 1;	//深度ビューは1つ
	dsvHeapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//デプスステンシルビュー
	result = DX12Init::GetDevice()->CreateDescriptorHeap(
		&dsvHeapdesc,
		IID_PPV_ARGS(&dsvHeap)
	);

	//深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;	//深度値フォーマット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DX12Init::GetDevice()->CreateDepthStencilView(
		depthBuffer,
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);



}

void DX12Init::BeginDraw()
{
	// バックバッファの番号を取得(2 つなので 0 番か 1 番)
	bbIndex = swapchain->GetCurrentBackBufferIndex();

	// 1.リソースバリアを変更
	//表示状態から描画状態に変更
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 2-1.レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(), bbIndex, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());

	//2-2.深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	// 3.画面クリアコマンドここから

	// 全画面クリア R G B A		
	float clearColor[4] = { 0.1,0.25,0.5 };
	cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	// 3.画面クリアコマンドここまで

	//4.ビューポートの設定コマンド
	cmdList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, window_width, window_height));
	cmdList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, window_width, window_height));

	//5.シザー矩形の設定コマンド
	D3D12_RECT scissorrect{};
	scissorrect.left = 0; // 切り抜き座標左
	scissorrect.right = scissorrect.left + window_width; // 切り抜き座標右
	scissorrect.top = 0; // 切り抜き座標上
	scissorrect.bottom = scissorrect.top + window_height; // 切り抜き座標下
	cmdList->RSSetScissorRects(1, &scissorrect);
}

void DX12Init::EndDraw()
{
	//表示状態から描画状態に変更
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//11.命令のクローズ
	cmdList->Close();

	//12.コマンドリストの実行
	ID3D12CommandList* cmdLists[] = { cmdList.Get() }; // コマンドリストの配列
	cmdQueue->ExecuteCommandLists(1, cmdLists);

	//13.コマンドリストの実行完了を待つ
	cmdQueue->Signal(fence.Get(), ++fenceVal);
	if (fence->GetCompletedValue() != fenceVal) {
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	//14.コマンドアロケーターのリセット
	cmdAllocator->Reset(); // キューをクリア

	//15.コマンドリストのリセット
	cmdList->Reset(cmdAllocator.Get(), nullptr); // 再びコマンドリストを貯める準備


	//16.バッファをフリップ(裏表の入替え)
	swapchain->Present(1, 0);
}

void DX12Init::End()
{
	UnregisterClass(w.lpszClassName, w.hInstance);
}

HWND DX12Init::GetHwnd()
{
	return hwnd;
}

ID3D12Device* DX12Init::GetDevice()
{
	return dev.Get();
}


ID3D12GraphicsCommandList* DX12Init::GetCmdList()
{
	return cmdList.Get();
}

const int DX12Init::GetWindowWidth()
{
	return window_width;
}

const int DX12Init::GetWindowHeight()
{
	return window_height;
}

void DX12Init::ClearDepthBuffer()
{
	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	// 深度バッファのクリア
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

}
