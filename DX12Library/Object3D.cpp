#pragma once
#include <DirectXTex.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include "Object3D.h"
#include "DX12Init.h"
#include <fstream>
#include <sstream>

using namespace DirectX;

//静的メンバ変数の実体
XMMATRIX Object3D::matView{};
XMMATRIX Object3D::matProjection{};
XMFLOAT3 Object3D::eye = { 0, 0, -100 };
XMFLOAT3 Object3D::target = { 0, 0, 0 };
XMFLOAT3 Object3D::up = { 0, 1, 0 };

Object3D::ComPtr <ID3D12DescriptorHeap> Object3D::basicDescHeap = nullptr;
Object3D::ComPtr <ID3D12RootSignature> Object3D::rootsignatureOBJ = nullptr;
Object3D::ComPtr <ID3D12RootSignature> Object3D::rootsignatureMath = nullptr;
Object3D::ComPtr <ID3D12PipelineState> Object3D::pipelinestateOBJ = nullptr;
Object3D::ComPtr <ID3D12PipelineState> Object3D::pipelinestateOBJ_wire = nullptr;
Object3D::ComPtr <ID3D12PipelineState> Object3D::pipelinestateMath = nullptr;
Object3D::ComPtr <ID3D12PipelineState> Object3D::pipelinestateMath_wire = nullptr;

void Object3D::FirstInit()
{
	HRESULT result;

	//デスクリプタヒープを生成
	// 設定構造体
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // シェーダーから見える
	descHeapDesc.NumDescriptors = 128; // SRV1つ
	// 生成
	result = DX12Init::GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	InitPipelineMath();
	InitPipelineOBJ();
}

void Object3D::InitPipelineOBJ()
{
	HRESULT result;

	ComPtr <ID3DBlob> vsBlob = nullptr; // 頂点シェーダオブジェクト
	ComPtr <ID3DBlob> psBlob = nullptr; // ピクセルシェーダオブジェクト
	ComPtr <ID3DBlob> errorBlob = nullptr; // エラーオブジェクト

	//各シェーダファイルの読み込みとコンパイル
	vsBlob = nullptr; // 頂点シェーダオブジェクト
	psBlob = nullptr; // ピクセルシェーダオブジェクト
	errorBlob = nullptr; // エラーオブジェクト

	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"OBJVertexShader.hlsl",	// シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0",	// エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob);

	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"OBJPixelShader.hlsl",	// シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0",	// エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);

	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	//頂点シェーダに渡すための頂点データを整える。
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{//xy座標(1行で書いたほうが見やすい)
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{//法線ベクトル(1行で書いたほうが見やすい)
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{//uv座標(1 行で書いたほうが見やすい)
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

	};
	//グラフィックスパイプラインの各ステージの設定をする構造体を用意する。
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};

	//頂点シェーダ、ピクセルシェーダをパイプラインに設定
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());


	//サンプルマスクとラスタライザステートの設定
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
	//標準的な設定(背面カリング、塗りつぶし、深度クリッピング有効)
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	//ブレンドステートの設定
	// レンダーターゲットのブレンド設定(8 個あるがいまは一つしか使わない)
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // 標準設定

	blenddesc.BlendEnable = true; // ブレンドを有効にする
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; // 加算
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE; // ソースの値を 100% 使う
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO; // デストの値を 0% 使う

	blenddesc.BlendOp = D3D12_BLEND_OP_ADD; // 加算
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA; // ソースのアルファ値
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // 1.0f-ソースのアルファ値

	// ブレンドステートに設定する
	gpipeline.BlendState.RenderTarget[0] = blenddesc;


	//頂点レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	//図形の形状を三角形に設定
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//その他の設定
	gpipeline.NumRenderTargets = 1; // 描画対象は 1 つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0~255 指定の RGBA
	gpipeline.SampleDesc.Count = 1; // 1 ピクセルにつき 1 回サンプリング

	//デプスステンシルステートの設定
	//標準的な設定(深度テストを行う、書き込み許可、深度が小さければ合格)
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//デスクリプタテーブルの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); //t0 レジスタ

	//ルートパラメータの設定
	CD3DX12_ROOT_PARAMETER rootparams[3];
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	//サンプラーの設定
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	//ルートシグネチャの生成
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr <ID3DBlob> rootSigBlob;
	//バージョン自動判定でのシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);

	result = DX12Init::GetDevice()->CreateRootSignature(0, rootSigBlob.Get()->GetBufferPointer(), rootSigBlob.Get()->GetBufferSize(),
		IID_PPV_ARGS(&rootsignatureOBJ));

	// パイプラインにルートシグネチャをセット
	gpipeline.pRootSignature = rootsignatureOBJ.Get();

	//パイプラインステートの生成
	result = DX12Init::GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestateOBJ));

	//ワイヤフレーム用パイプラインを作る
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline_wire{};
	gpipeline_wire = gpipeline;
	gpipeline_wire.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

	//パイプラインステートの生成
	result = DX12Init::GetDevice()->CreateGraphicsPipelineState(&gpipeline_wire, IID_PPV_ARGS(&pipelinestateOBJ_wire));
}

void Object3D::InitPipelineMath()
{
	HRESULT result;

	ComPtr <ID3DBlob> vsBlob = nullptr; // 頂点シェーダオブジェクト
	ComPtr <ID3DBlob> psBlob = nullptr; // ピクセルシェーダオブジェクト
	ComPtr <ID3DBlob> errorBlob = nullptr; // エラーオブジェクト

	//各シェーダファイルの読み込みとコンパイル
	vsBlob = nullptr; // 頂点シェーダオブジェクト
	psBlob = nullptr; // ピクセルシェーダオブジェクト
	errorBlob = nullptr; // エラーオブジェクト

	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",	// シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"VSmain", "vs_5_0",	// エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob);

	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",	// シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"PSmain", "ps_5_0",	// エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);

	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	//頂点シェーダに渡すための頂点データを整える。
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{//xy座標(1行で書いたほうが見やすい)
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{//法線ベクトル(1行で書いたほうが見やすい)
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{//uv座標(1 行で書いたほうが見やすい)
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

	};
	//グラフィックスパイプラインの各ステージの設定をする構造体を用意する。
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};

	//頂点シェーダ、ピクセルシェーダをパイプラインに設定
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());


	//サンプルマスクとラスタライザステートの設定
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
	//標準的な設定(背面カリング、塗りつぶし、深度クリッピング有効)
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	//ブレンドステートの設定
	// レンダーターゲットのブレンド設定(8 個あるがいまは一つしか使わない)
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // 標準設定

	blenddesc.BlendEnable = true; // ブレンドを有効にする
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; // 加算
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE; // ソースの値を 100% 使う
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO; // デストの値を 0% 使う

	blenddesc.BlendOp = D3D12_BLEND_OP_ADD; // 加算
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA; // ソースのアルファ値
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // 1.0f-ソースのアルファ値

	// ブレンドステートに設定する
	gpipeline.BlendState.RenderTarget[0] = blenddesc;


	//頂点レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	//図形の形状を三角形に設定
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//その他の設定
	gpipeline.NumRenderTargets = 1; // 描画対象は 1 つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0~255 指定の RGBA
	gpipeline.SampleDesc.Count = 1; // 1 ピクセルにつき 1 回サンプリング

	//デプスステンシルステートの設定
	//標準的な設定(深度テストを行う、書き込み許可、深度が小さければ合格)
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//デスクリプタテーブルの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); //t0 レジスタ

	//ルートパラメータの設定
	CD3DX12_ROOT_PARAMETER rootparams[3];
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	//サンプラーの設定
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	//ルートシグネチャの生成
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr <ID3DBlob> rootSigBlob;
	//バージョン自動判定でのシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);

	result = DX12Init::GetDevice()->CreateRootSignature(0, rootSigBlob.Get()->GetBufferPointer(), rootSigBlob.Get()->GetBufferSize(),
		IID_PPV_ARGS(&rootsignatureMath));

	// パイプラインにルートシグネチャをセット
	gpipeline.pRootSignature = rootsignatureMath.Get();

	//パイプラインステートの生成
	result = DX12Init::GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestateMath));

	//ワイヤフレーム用パイプラインを作る
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline_wire{};
	gpipeline_wire = gpipeline;
	gpipeline_wire.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

	//パイプラインステートの生成
	result = DX12Init::GetDevice()->CreateGraphicsPipelineState(&gpipeline_wire, IID_PPV_ARGS(&pipelinestateMath_wire));

}

void Object3D::Initialize()
{
	HRESULT result;

	//定数バッファの作成
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB0) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffB0));


	//射影行列の作成
	//透視投影
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f),
		(float)DX12Init::GetWindowWidth() / DX12Init::GetWindowHeight(),
		0.1f,
		1000.0f
	);

	//値の初期化
	scale = { 1.0f,1.0f,1.0f };
	rotation = { 0.0f,0.0f,0.0f };
	position = { 0.0f,0.0f,0.0f };

	//ビュー行列の作成
	matView = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up)
	);

}

void Object3D::Update()
{
	HRESULT result;

	XMMATRIX matScale;	//スケーリング行列
	XMMATRIX matRot;	//回転行列
	XMMATRIX matTrans;	//平行移動行列

	////スケーリング (SCALING)
	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	////回転 (ROTATION)
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));	//Z軸まわりに回転
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));	//X軸まわりに回転
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));	//Y軸まわりに回転

	////平行移動 (TRANSLATION)
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);


	//各変形行列からワールド行列を合成する
	matWorld = XMMatrixIdentity();
	matWorld *= matScale;	//ワールド行列にスケーリングを反映
	matWorld *= matRot;		//ワールド行列に回転を反映
	matWorld *= matTrans;	//ワールド行列に平行移動を反映

	//定数バッファにデータを転送する
	ConstBufferDataB0* constMap = nullptr;

	result = constBuffB0->Map(0, nullptr, (void**)&constMap);
	constMap->color = color;
	constMap->mat = matWorld * matView * matProjection;	// 行列の合成
	constBuffB0->Unmap(0, nullptr);

	model.Update();
}

void Object3D::Draw()
{
	if (model.GetIsOBJ())
	{
		if (isWireFlane)
		{
			//パイプラインステートの設定コマンド
			DX12Init::GetCmdList()->SetPipelineState(pipelinestateOBJ_wire.Get());
		}
		else
		{
			//パイプラインステートの設定コマンド
			DX12Init::GetCmdList()->SetPipelineState(pipelinestateOBJ.Get());
		}

		//ルートシグネチャの設定コマンド
		DX12Init::GetCmdList()->SetGraphicsRootSignature(rootsignatureOBJ.Get());
	}
	else
	{
		if (isWireFlane)
		{
			//パイプラインステートの設定コマンド
			DX12Init::GetCmdList()->SetPipelineState(pipelinestateMath_wire.Get());
		}
		else
		{
			//パイプラインステートの設定コマンド
			DX12Init::GetCmdList()->SetPipelineState(pipelinestateMath.Get());
		}

		//ルートシグネチャの設定コマンド
		DX12Init::GetCmdList()->SetGraphicsRootSignature(rootsignatureMath.Get());
	}



	// デスクリプタヒープの配列
	ID3D12DescriptorHeap* ppHeaps[] = { basicDescHeap.Get() };
	DX12Init::GetCmdList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// 定数バッファビューをセット
	DX12Init::GetCmdList()->SetGraphicsRootConstantBufferView(0, constBuffB0->GetGPUVirtualAddress());

	model.Draw();
}

void Object3D::SetModel(Model model)
{
	this->model = model;
}


void Object3D::AddEye(const float eyeX, const float eyeY, const float eyeZ)
{
	Object3D::eye.x += eyeX;
	Object3D::eye.y += eyeY;
	Object3D::eye.z += eyeZ;
	UpdateViewMatrix();
}

void Object3D::AddTarget(const float targetX, const float targetY, const float targetZ)
{
	Object3D::target.x += targetX;
	Object3D::target.y += targetY;
	Object3D::target.z += targetZ;
	UpdateViewMatrix();
}

void Object3D::AddUp(const float upX, const float upY, const float upZ)
{
	Object3D::up.x += upX;
	Object3D::up.y += upY;
	Object3D::up.z += upZ;
	UpdateViewMatrix();
}

void Object3D::UpdateViewMatrix()
{
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
}

ID3D12DescriptorHeap * Object3D::GetDescHeap()
{
	return basicDescHeap.Get();
}

void Object3D::SetEye(const XMFLOAT3 eye)
{
	Object3D::eye = eye;
	UpdateViewMatrix();
}

void Object3D::SetTarSet(const XMFLOAT3 target)
{
	Object3D::target = target;
	UpdateViewMatrix();
}

void Object3D::SetUp(const XMFLOAT3 up)
{
	Object3D::up = up;
	UpdateViewMatrix();
}
