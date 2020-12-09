#pragma once
#include <Windows.h>
#include <wrl.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <dinput.h>
#include "DX12Util.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

class DX12Util
{
	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMVECTOR = DirectX::XMVECTOR;

	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;


private:
	// ウィンドウサイズ
	static const int window_width;
	static const int window_height;

	static WNDCLASSEX w; // ウィンドウクラスの設定
	static HWND hwnd;
	static ComPtr<ID3D12Device> dev;
	static ComPtr<IDXGIFactory6> dxgiFactory;
	static ComPtr<IDXGISwapChain4> swapchain;
	static ComPtr<ID3D12CommandAllocator> cmdAllocator;
	static ComPtr<ID3D12GraphicsCommandList> cmdList;
	static ComPtr<ID3D12CommandQueue> cmdQueue;
	static ComPtr<ID3D12DescriptorHeap> rtvHeaps;
	static D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	static std::vector<ComPtr<ID3D12Resource>> backBuffers;
	static ComPtr <ID3D12DescriptorHeap> dsvHeap;
	static ComPtr<ID3D12Fence> fence;
	static UINT64 fenceVal;
	static UINT bbIndex;

public:
	static void Initialize();
	static void BeginDraw();
	static void EndDraw();
	static void End();

	static HWND GetHwnd();
	static ID3D12Device* GetDevice();
	static ID3D12GraphicsCommandList* GetCmdList();

	static const int GetWindowWidth();
	static const int GetWindowHeight();

	/// <summary>
	/// ファイルが存在するかを返す
	/// </summary>
	/// <param name="path">ファイルパス</param>
	/// <returns>ファイルが存在するか否か</returns>
	static bool IsFileExist(const wchar_t* path);

	static void ClearDepthBuffer();
};

