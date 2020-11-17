#define DIRECTINPUT_VERSION	0x0800	//DirectInput�̃o�[�W�����w��

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

 const int DX12Init::window_width = 1280;// ����
 const int DX12Init::window_height = 720; // �c��

 WNDCLASSEX DX12Init::w; // �E�B���h�E�N���X�̐ݒ�
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
	//���b�Z�[�W�ŕ���
	switch (msg)
	{
	case WM_DESTROY: //�E�B���h�E���j�����ꂽ
		PostQuitMessage(0); //OS�ɑ΂��āA�A�v���̏I����`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam); //�W���̏������s��
}

void DX12Init::Initialize()
{
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc; // �E�B���h�E�v���V�[�W����ݒ�
	w.lpszClassName = L"DirectXGame"; // �E�B���h�E�N���X��
	w.hInstance = GetModuleHandle(nullptr); // �E�B���h�E�n���h��
	w.hCursor = LoadCursor(NULL, IDC_ARROW); // �J�[�\���w��
	// �E�B���h�E�N���X�� OS �ɓo�^
	RegisterClassEx(&w);
	// �E�B���h�E�T�C�Y{ X ���W Y ���W ���� �c�� }
	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); // �����ŃT�C�Y�␳
	// �E�B���h�E�I�u�W�F�N�g�̐���
	hwnd = CreateWindow(w.lpszClassName, // �N���X��
		L"DirectXGame", // �^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW, // �W���I�ȃE�B���h�E�X�^�C��
		CW_USEDEFAULT, // �\�� X ���W(OS �ɔC����)
		CW_USEDEFAULT, // �\�� Y ���W(OS �ɔC����)
		wrc.right - wrc.left, // �E�B���h�E����
		wrc.bottom - wrc.top, // �E�B���h�E�c��
		nullptr, // �e�E�B���h�E�n���h��
		nullptr, // ���j���[�n���h��
		w.hInstance, // �Ăяo���A�v���P�[�V�����n���h��
		nullptr); // �I�v�V����
		// �E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	// DirectX ���������� ��������
	HRESULT result;
	dev = nullptr;

	// DXGI �t�@�N�g���[�̐���
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	// �A�_�v�^�[�̗񋓗p
	std::vector< ComPtr<IDXGIAdapter>> adapters;
	// �����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	ComPtr < IDXGIAdapter> tmpAdapter = nullptr;
	for (int i = 0;
		dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		i++)
	{
		adapters.push_back(tmpAdapter); // ���I�z��ɒǉ�����
	}
	for (int i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC adesc{};
		adapters[i]->GetDesc(&adesc); // �A�_�v�^�[�̏����擾
		std::wstring strDesc = adesc.Description; // �A�_�v�^�[��
		// Microsoft Basic Render Driver �����
		if (strDesc.find(L"Microsoft") == std::wstring::npos && strDesc.find(L"Intel") == std::wstring::npos)
		{
			tmpAdapter = adapters[i]; // �̗p
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
		//�̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&dev));
		if (result == S_OK)
		{
			//�f�o�C�X�𐶐��ł������_�Ń��[���𔲂���
			featureLevel = levels[i];
			break;
		}
	}

	// �R�}���h�A���P�[�^�𐶐�
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));
	// �R�}���h���X�g�𐶐�
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList));

	// �W���ݒ�ŃR�}���h�L���[�𐶐�
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));



	// �e��ݒ�����ăX���b�v�`�F�[���𐶐�
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = 1280;
	swapchainDesc.Height = 720;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �F���̏���
	swapchainDesc.SampleDesc.Count = 1; // �}���`�T���v�����Ȃ�
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // �o�b�N�o�b�t�@�p
	swapchainDesc.BufferCount = 2; // �o�b�t�@����2�ɐݒ�
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // �t���b�v��͔j��
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//IDXGISwapChain1 ��ComPtr��p��
	ComPtr<IDXGISwapChain1> swapchain1;
	//�X���b�v�`�F�[���̐���
	result = dxgiFactory->CreateSwapChainForHwnd(
		cmdQueue.Get(),
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		&swapchain1);
	//��������IDXGISwapChain1�̃I�u�W�F�N�g��IDXGISwapChain4�ɕϊ�����
	swapchain1.As(&swapchain);

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = swapchain->GetDesc(&swcDesc);

	// �e��ݒ�����ăf�X�N���v�^�q�[�v�𐶐�
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // �����_�[�^�[�Q�b�g�r���[
	heapDesc.NumDescriptors = 2; // ���\��2��
	dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	backBuffers.resize(swcDesc.BufferCount);
	// ���\��2���ɂ���
	for (int i = 0; i < 2; i++)
	{
		// �X���b�v�`�F�[������o�b�t�@���擾
		result = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		// �f�B�X�N���v�^�q�[�v�̃n���h�����擾
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle =
			CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(), i,
				dev.Get()->GetDescriptorHandleIncrementSize(heapDesc.Type));
		// �����_�[�^�[�Q�b�g�r���[�̐���
		dev->CreateRenderTargetView(
			backBuffers[i].Get(),
			nullptr,
			handle);

	}

	// �t�F���X�̐���
	fenceVal = 0;
	result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	//�[�x�o�b�t�@�̍쐬
	//���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		DX12Init::GetWindowWidth(),
		DX12Init::GetWindowHeight(),
		1, 0,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	//���\�[�X����
	ID3D12Resource* depthBuffer = nullptr;

	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//�[�x�l�������݂Ɏg�p
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
		IID_PPV_ARGS(&depthBuffer)
	);


	//�R�}���h���s�̂��߂ɐ[�x�r���[�𐶐�����
	//�[�x�r���[�p�f�X�N���v�^�q�[�v����
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapdesc{};
	dsvHeapdesc.NumDescriptors = 1;	//�[�x�r���[��1��
	dsvHeapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//�f�v�X�X�e���V���r���[
	result = DX12Init::GetDevice()->CreateDescriptorHeap(
		&dsvHeapdesc,
		IID_PPV_ARGS(&dsvHeap)
	);

	//�[�x�r���[�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DX12Init::GetDevice()->CreateDepthStencilView(
		depthBuffer,
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);



}

void DX12Init::BeginDraw()
{
	// �o�b�N�o�b�t�@�̔ԍ����擾(2 �Ȃ̂� 0 �Ԃ� 1 ��)
	bbIndex = swapchain->GetCurrentBackBufferIndex();

	// 1.���\�[�X�o���A��ύX
	//�\����Ԃ���`���ԂɕύX
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 2-1.�����_�[�^�[�Q�b�g�r���[�p�f�B�X�N���v�^�q�[�v�̃n���h�����擾
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(), bbIndex, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	// �[�x�X�e���V���r���[�p�f�X�N���v�^�q�[�v�̃n���h�����擾
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());

	//2-2.�[�x�X�e���V���r���[�p�f�X�N���v�^�q�[�v�̃n���h�����擾
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	// 3.��ʃN���A�R�}���h��������

	// �S��ʃN���A R G B A		
	float clearColor[4] = { 0.1,0.25,0.5 };
	cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	// 3.��ʃN���A�R�}���h�����܂�

	//4.�r���[�|�[�g�̐ݒ�R�}���h
	cmdList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, window_width, window_height));
	cmdList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, window_width, window_height));

	//5.�V�U�[��`�̐ݒ�R�}���h
	D3D12_RECT scissorrect{};
	scissorrect.left = 0; // �؂蔲�����W��
	scissorrect.right = scissorrect.left + window_width; // �؂蔲�����W�E
	scissorrect.top = 0; // �؂蔲�����W��
	scissorrect.bottom = scissorrect.top + window_height; // �؂蔲�����W��
	cmdList->RSSetScissorRects(1, &scissorrect);
}

void DX12Init::EndDraw()
{
	//�\����Ԃ���`���ԂɕύX
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//11.���߂̃N���[�Y
	cmdList->Close();

	//12.�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdLists[] = { cmdList.Get() }; // �R�}���h���X�g�̔z��
	cmdQueue->ExecuteCommandLists(1, cmdLists);

	//13.�R�}���h���X�g�̎��s������҂�
	cmdQueue->Signal(fence.Get(), ++fenceVal);
	if (fence->GetCompletedValue() != fenceVal) {
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	//14.�R�}���h�A���P�[�^�[�̃��Z�b�g
	cmdAllocator->Reset(); // �L���[���N���A

	//15.�R�}���h���X�g�̃��Z�b�g
	cmdList->Reset(cmdAllocator.Get(), nullptr); // �ĂуR�}���h���X�g�𒙂߂鏀��


	//16.�o�b�t�@���t���b�v(���\�̓��ւ�)
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
	// �[�x�X�e���V���r���[�p�f�X�N���v�^�q�[�v�̃n���h�����擾
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	// �[�x�o�b�t�@�̃N���A
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

}
