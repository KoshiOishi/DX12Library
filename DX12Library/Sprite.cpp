#include <d3dcompiler.h>
#include <string>
#include <d3dx12.h>
#include <DirectXTex.h>
#include <wrl.h>
#include "Sprite.h"
#include "DX12Init.h"

#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;


ComPtr<ID3D12RootSignature> Sprite::spriteRootSignature = nullptr;	//ルートシグネチャ
ComPtr<ID3D12PipelineState> Sprite::spritePipelineState = nullptr;	//パイプラインステート
ComPtr<ID3DBlob> Sprite::vsBlob = nullptr; // 頂点シェーダオブジェクト
ComPtr<ID3DBlob> Sprite::psBlob = nullptr; // ピクセルシェーダオブジェクト
ComPtr<ID3DBlob> Sprite::errorBlob = nullptr; // エラーオブジェクト
XMMATRIX Sprite::spriteMatProjection;		//射影行列
ComPtr <ID3D12DescriptorHeap> Sprite::spriteDescHeap = nullptr;
const int Sprite::spriteSRVCount = 512;
ComPtr <ID3D12Resource> Sprite::spriteTexbuff[Sprite::spriteSRVCount];	//テクスチャバッファ


HRESULT Sprite::Initialize()
{
	HRESULT result = S_FALSE;

	//各シェーダファイルの読み込みとコンパイル
	vsBlob = nullptr; // 頂点シェーダオブジェクト
	psBlob = nullptr; // ピクセルシェーダオブジェクト
	errorBlob = nullptr; // エラーオブジェクト
	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"SpriteVertexShader.hlsl", // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"VSmain", "vs_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob);

	if (FAILED(result)) {
		// errorBlob からエラー内容を string 型にコピー
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
		L"SpritePixelShader.hlsl", // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"PSmain", "ps_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);

	if (FAILED(result)) {
		// errorBlob からエラー内容を string 型にコピー
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

	//頂点レイアウト配列の宣言と、各種項目の設定
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION",
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,
		0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0},

		{"TEXCOORD",
		0,
		DXGI_FORMAT_R32G32_FLOAT,
		0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0},

	};

	//グラフィックスパイプラインの各ステージの設定をする構造体を用意する。
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};

	//頂点シェーダ、ピクセルシェーダをパイプラインに設定
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());


	//サンプルマスクとラスタライザステートの設定
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
	//標準的な設定(背面カリング、塗りつぶし、深度クリッピング有効)
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);		//一旦標準値をセット
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;				//カリングしない

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
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);	//一旦標準値をセット
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;		//常に上書きルール
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//デスクリプタテーブルの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); //t0 レジスタ

	//ルートパラメータの設定
	CD3DX12_ROOT_PARAMETER rootparams[2];
	//定数用
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);	//定数バッファビューとして初期化(b0)
	//テクスチャ用
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

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
		IID_PPV_ARGS(&spriteRootSignature));

	// パイプラインにルートシグネチャをセット
	gpipeline.pRootSignature = spriteRootSignature.Get();

	//パイプラインステートの生成
	result = DX12Init::GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&spritePipelineState));


	//スプライト用デスクリプタヒープの生成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = spriteSRVCount;
	result = DX12Init::GetDevice()->CreateDescriptorHeap(
		&descHeapDesc, IID_PPV_ARGS(&spriteDescHeap)
	);

}

HRESULT Sprite::SpriteLoadTexture(UINT texnumber, const wchar_t* filename)
{
	HRESULT result = S_FALSE;

	//WICテクスチャのロード
	TexMetadata metadata{};
	ScratchImage scratchimg{};

	result = LoadFromWICFile(
		filename,		//「Resources」フォルダの「gazoudayo.png」
		WIC_FLAGS_NONE,
		&metadata,
		scratchimg
	);

	const Image* img = scratchimg.GetImage(0, 0, 0);	//生データ抽出

		//テクスチャバッファの生成
	//GPU側にテクスチャバッファを作成する
	D3D12_HEAP_PROPERTIES texHeapProp{};	//テクスチャヒープ設定
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	result = DX12Init::GetDevice()->CreateCommittedResource(	//GPUリソースの生成
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,	//テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&spriteTexbuff[texnumber])
	);

	//データ転送
	//テクスチャバッファにデータ転送
	result = spriteTexbuff[texnumber]->WriteToSubresource(
		0,
		nullptr,	//全領域にコピー
		img->pixels,	//元データアドレス
		(UINT)img->rowPitch,	//1ラインサイズ
		(UINT)img->slicePitch	//全サイズ
	);

	//実際にシェーダーリソースビューを生成する
	//シェーダーリソースビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};	//設定構造体
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	DX12Init::GetDevice()->CreateShaderResourceView(
		spriteTexbuff[texnumber].Get(),	//ビューと関連付けるバッファ
		&srvDesc,	//テクスチャ設定情報
		CD3DX12_CPU_DESCRIPTOR_HANDLE(
		spriteDescHeap->GetCPUDescriptorHandleForHeapStart(),
			texnumber,
			DX12Init::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		)
	);

	return S_OK;
}

HRESULT Sprite::GenerateSprite(XMFLOAT2 anchorpoint, bool isFlipX, bool isFlipY)
{
	HRESULT result = S_FALSE;
	this->anchorpoint = anchorpoint;
	this->isFlipX = isFlipX;
	this->isFlipY = isFlipY;

	//頂点データ
	VertexPosUv vertices[] = {
		//	 x		y	  z		u	  v
		{{	0.0f,100.0f,0.0f},{0.0f,1.0f}},	//左下
		{{	0.0f,  0.0f,0.0f},{0.0f,0.0f}},	//左上
		{{100.0f,100.0f,0.0f},{1.0f,1.0f}},	//右下
		{{100.0f,  0.0f,0.0f},{1.0f,0.0f}},	//右上

	};

	//頂点バッファ生成
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&spriteVertBuff)
	);

	//画像の大きさから表示サイズを設定
	//左下、左上、右下、右上
	enum { LB, LT, RB, RT };
	//テクスチャ情報取得
	D3D12_RESOURCE_DESC resDesc =
		spriteTexbuff[texNumber]->GetDesc();
	width = (float)resDesc.Width;		//画像の横幅
	height = (float)resDesc.Height;		//画像の縦幅

	float left = (0.0f - anchorpoint.x) * width;
	float right = (1.0f - anchorpoint.x) * width;
	float top = (0.0f - anchorpoint.y) * height;
	float bottom = (1.0f - anchorpoint.y) * height;

	if (isFlipX)
	{
		//左右入れ替え
		left = -left;
		right = -right;
	}

	if (isFlipY)
	{
		//上下入れ替え
		top = -top;
		bottom = -bottom;
	}

	vertices[LB].pos = { left,	bottom,	0.0f };	//左下
	vertices[LT].pos = { left,	top,	0.0f };	//左上
	vertices[RB].pos = { right,	bottom,	0.0f };	//右下
	vertices[RT].pos = { right,	top,	0.0f };	//右上

	//テクスチャ情報取得
	if (spriteTexbuff[texNumber])
	{
		D3D12_RESOURCE_DESC resDesc =
			spriteTexbuff[texNumber]->GetDesc();

		tex_x = 0;
		tex_y = 0;
		tex_width = resDesc.Width;
		tex_height = resDesc.Height;

		float tex_left = tex_x / resDesc.Width;
		float tex_right = (tex_x + tex_width) / resDesc.Width;
		float tex_top = tex_y / resDesc.Height;
		float tex_bottom = (tex_y + tex_height) / resDesc.Height;

		vertices[LB].uv = { tex_left,	tex_bottom };	//左下
		vertices[LT].uv = { tex_left,	tex_top };	//左上
		vertices[RB].uv = { tex_right,	tex_bottom };	//右下
		vertices[RT].uv = { tex_right,	tex_top };	//右上
	}


	//バッファへのデータ転送
	VertexPosUv* vertMap = nullptr;
	result = spriteVertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	spriteVertBuff->Unmap(0, nullptr);

	//頂点バッファビューの作成
	spriteVBView.BufferLocation = spriteVertBuff->GetGPUVirtualAddress();
	spriteVBView.SizeInBytes = sizeof(vertices);
	spriteVBView.StrideInBytes = sizeof(vertices[0]);

	//定数バッファの生成
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,
		IID_PPV_ARGS(&spriteConstBuff)
		);

	//定数バッファにデータを転送
	ConstBufferData* constMap = nullptr;
	result = spriteConstBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = XMFLOAT4(1, 1, 1, 1);		//色指定 (RGBA)
	constMap->mat = XMMatrixOrthographicOffCenterLH(
		0.0f, DX12Init::GetWindowWidth(), DX12Init::GetWindowHeight(), 0.0f, 0.0f, 1.0f);	//平行投影行列の合成
	spriteConstBuff->Unmap(0, nullptr);


	//行列初期化
	spriteMatProjection = XMMatrixOrthographicOffCenterLH(
		0.0f, DX12Init::GetWindowWidth(), DX12Init::GetWindowHeight(), 0.0f, 0.0f, 1.0f
	);


	return result;
}

void Sprite::SpriteSetPipeline()
{
	//パイプラインステートの設定
	DX12Init::GetCmdList()->SetPipelineState(spritePipelineState.Get());
	//ルートシグネチャの設定
	DX12Init::GetCmdList()->SetGraphicsRootSignature(spriteRootSignature.Get());
	//プリミティブ形状を設定
	DX12Init::GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void Sprite::SpriteDraw()
{
	SpriteSetPipeline();

	if (!isDisplay)
		return;

	//ワールド行列の更新
	spriteMatWorld = XMMatrixIdentity();
	spriteMatWorld *= XMMatrixRotationZ(XMConvertToRadians(spriteRotation));
	spriteMatWorld *= XMMatrixTranslationFromVector(spritePosition);
	//行列の転送
	ConstBufferData* constMap = nullptr;
	HRESULT result = spriteConstBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = spriteColor;
	constMap->mat = spriteMatWorld * spriteMatProjection;		//行列の合成
	spriteConstBuff->Unmap(0, nullptr);


	//デスクリプタヒープの配列
	ID3D12DescriptorHeap* ppHeaps[] = { spriteDescHeap.Get() };
	DX12Init::GetCmdList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//頂点バッファをセット
	DX12Init::GetCmdList()->IASetVertexBuffers(0, 1, &spriteVBView);

	//定数バッファをセット
	DX12Init::GetCmdList()->SetGraphicsRootConstantBufferView(0, spriteConstBuff->GetGPUVirtualAddress());


	//シェーダリソースビューをセット
	DX12Init::GetCmdList()->SetGraphicsRootDescriptorTable(1,
		CD3DX12_GPU_DESCRIPTOR_HANDLE(
			spriteDescHeap->GetGPUDescriptorHandleForHeapStart(),
			texNumber,
			DX12Init::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		)
	);

	//描画コマンド
	DX12Init::GetCmdList()->DrawInstanced(4, 1, 0, 0);
}

void Sprite::SpriteSetSize(float width, float height)
{
	this->width = width;
	this->height = height;

	SpriteUpdateVertBuff();
}

void Sprite::SpriteSetAnchorpoint(XMFLOAT2 anchorpoint)
{
	this->anchorpoint = anchorpoint;

	SpriteUpdateVertBuff();
}

void Sprite::SpriteSetTexNumber(UINT texnumber)
{
	texNumber = texnumber;
}

void Sprite::SpriteSetIsFlipX(bool isFlipX)
{
	this->isFlipX = isFlipX;

	SpriteUpdateVertBuff();
}

void Sprite::SpriteSetIsFlipY(bool isFlipY)
{
	this->isFlipY = isFlipY;

	SpriteUpdateVertBuff();
}

void Sprite::SpriteSetDrawRectangle(float tex_x, float tex_y, float tex_width, float tex_height)
{
	this->tex_x = tex_x;
	this->tex_y = tex_y;
	this->tex_width = tex_width;
	this->tex_height = tex_height;

	width = tex_width;
	height = tex_height;

	SpriteUpdateVertBuff();

}

void Sprite::SpriteUpdateVertBuff()
{
	HRESULT result = S_FALSE;

	//頂点データ
	VertexPosUv vertices[] = {
		//	 x		y	  z		u	  v
		{{	0.0f,100.0f,0.0f},{0.0f,1.0f}},	//左下
		{{	0.0f,  0.0f,0.0f},{0.0f,0.0f}},	//左上
		{{100.0f,100.0f,0.0f},{1.0f,1.0f}},	//右下
		{{100.0f,  0.0f,0.0f},{1.0f,0.0f}},	//右上

	};

	//頂点バッファ生成
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&spriteVertBuff)
	);

	//画像の大きさから表示サイズを設定
	//左下、左上、右下、右上
	enum { LB, LT, RB, RT };
	float left = (0.0f - anchorpoint.x) * width;
	float right = (1.0f - anchorpoint.x) * width;
	float top = (0.0f - anchorpoint.y) * height;
	float bottom = (1.0f - anchorpoint.y) * height;

	if (isFlipX)
	{
		//左右入れ替え
		left = -left;
		right = -right;
	}

	if (isFlipY)
	{
		//上下入れ替え
		top = -top;
		bottom = -bottom;
	}

	vertices[LB].pos = { left,	bottom,	0.0f };	//左下
	vertices[LT].pos = { left,	top,	0.0f };	//左上
	vertices[RB].pos = { right,	bottom,	0.0f };	//右下
	vertices[RT].pos = { right,	top,	0.0f };	//右上

	//テクスチャ情報取得
	if (spriteTexbuff[texNumber])
	{
		D3D12_RESOURCE_DESC resDesc =
			spriteTexbuff[texNumber]->GetDesc();

		float tex_left = tex_x / resDesc.Width;
		float tex_right = (tex_x + tex_width) / resDesc.Width;
		float tex_top = tex_y / resDesc.Height;
		float tex_bottom = (tex_y + tex_height) / resDesc.Height;

		vertices[LB].uv = { tex_left,	tex_bottom};	//左下
		vertices[LT].uv = { tex_left,	tex_top};	//左上
		vertices[RB].uv = { tex_right,	tex_bottom};	//右下
		vertices[RT].uv = { tex_right,	tex_top};	//右上
	}


	//バッファへのデータ転送
	VertexPosUv* vertMap = nullptr;
	result = spriteVertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	spriteVertBuff->Unmap(0, nullptr);

}

