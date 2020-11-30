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

UINT Object3D::descriptorHandleIncrementSize;
Object3D::ComPtr <ID3D12DescriptorHeap> Object3D::basicDescHeap = nullptr;
Object3D::ComPtr <ID3D12RootSignature> Object3D::rootsignature = nullptr;
Object3D::ComPtr <ID3D12PipelineState> Object3D::pipelinestate = nullptr;

void Object3D::FirstInit()
{
	HRESULT result;

	//デスクリプタヒープを生成
	// 設定構造体
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // シェーダーから見える
	descHeapDesc.NumDescriptors = 1; // SRV1つ
	// 生成
	result = DX12Init::GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	// デスクリプタサイズを取得
	descriptorHandleIncrementSize = DX12Init::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


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
		IID_PPV_ARGS(&rootsignature));

	// パイプラインにルートシグネチャをセット
	gpipeline.pRootSignature = rootsignature.Get();

	//パイプラインステートの生成
	result = DX12Init::GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate));

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


	//定数バッファの作成
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB0) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffB1));


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
	constMap->mat = matWorld * matView * matProjection;	// 行列の合成
	constBuffB0->Unmap(0, nullptr);

	// 定数バッファへデータ転送
	ConstBufferDataB1* constMap1 = nullptr;
	result = constBuffB1->Map(0, nullptr, (void**)&constMap1);
	constMap1->ambient = material.ambient;
	constMap1->diffuse = material.diffuse;
	constMap1->specular = material.specular;
	constMap1->alpha = material.alpha;
	constBuffB1->Unmap(0, nullptr);


}

void Object3D::Draw()
{


	//インデックスバッファのセットコマンド
	DX12Init::GetCmdList()->IASetIndexBuffer(&ibView);

	//パイプラインステートの設定コマンド
	DX12Init::GetCmdList()->SetPipelineState(pipelinestate.Get());


	//ルートシグネチャの設定コマンド
	DX12Init::GetCmdList()->SetGraphicsRootSignature(rootsignature.Get());

	//プリミティブ形状の設定コマンド(三角形リスト)
	DX12Init::GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//頂点バッファの設定コマンド
	DX12Init::GetCmdList()->IASetVertexBuffers(0, 1, &vbView);

	// デスクリプタヒープの配列
	ID3D12DescriptorHeap* ppHeaps[] = { basicDescHeap.Get() };
	DX12Init::GetCmdList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// 定数バッファビューをセット
	DX12Init::GetCmdList()->SetGraphicsRootConstantBufferView(0, constBuffB0->GetGPUVirtualAddress());
	DX12Init::GetCmdList()->SetGraphicsRootConstantBufferView(1, constBuffB1->GetGPUVirtualAddress());
	// シェーダリソースビューをセット
	DX12Init::GetCmdList()->SetGraphicsRootDescriptorTable(2, gpuDescHandleSRV);


	//描画コマンド
	DX12Init::GetCmdList()->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);

}

void Object3D::LoadOBJ(const std::string & modelname)
{
	HRESULT result;

	//ファイルストリーム
	std::fstream file;
	//.objファイルを開く
	const string filename = modelname + ".obj";	//"triangle_mat.obj"
	const string directoryPath = "Resources/" + modelname + "/";	//"Resources/triangle_mat/"
	file.open(directoryPath + filename);	//"Resources/triangle_mat/triangle_mat.obj"
	//ファイルオープン失敗をチェック
	if (file.fail()) {
		assert(0);
	}

	vector<XMFLOAT3> positions;	//頂点座標
	vector<XMFLOAT3> normals;	//法線ベクトル
	vector<XMFLOAT2> texcoords;	//テクスチャUV
	//1行ずつ読み込む
	string line;
	while (getline(file, line)) {

		//1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);

		//半角スペース区切りで行の先頭文字列を取得
		string key;
		getline(line_stream, key, ' ');

		//先頭文字列がmtllibならマテリアル
		if (key == "mtllib")
		{
			//マテリアルのファイル名読み込み
			string filename;
			line_stream >> filename;
			//マテリアル読み込み
			LoadMaterial(directoryPath, filename);
		}

		//先頭文字列がvなら頂点座標
		if (key == "v") {
			//X,Y,Z座標読み込み
			XMFLOAT3 position{};
			line_stream >> position.x;
			line_stream >> position.y;
			line_stream >> position.z;
			//座標データに追加
			positions.emplace_back(position);
			////頂点データに追加
			//VertexPosNormalUv vertex{};
			//vertex.pos = position;
			//vertices.emplace_back(vertex);
		}

		//先頭文字列がvtならテクスチャ
		if (key == "vt") {
			//U,V成分読み込み
			XMFLOAT2 texcoord{};
			line_stream >> texcoord.x;
			line_stream >> texcoord.y;
			//V方向反転
			texcoord.y = 1.0f - texcoord.y;
			//テクスチャ座標データに追加
			texcoords.emplace_back(texcoord);
		}

		//先頭文字列がvnなら法線ベクトル
		if (key == "vn") {
			//X,Y,Z成分読み込み
			XMFLOAT3 normal{};
			line_stream >> normal.x;
			line_stream >> normal.y;
			line_stream >> normal.z;
			//法線ベクトルデータに追加
			normals.emplace_back(normal);
		}

		//先頭文字列がfならポリゴン(三角形)
		if (key == "f")
		{
			//半角スペース区切りで行の続きを読み込む
			string index_string;
			while (getline(line_stream, index_string, ' ')) {

				//頂点インデックス1個分の文字列をストリームに変換して解析しやすくする
				std::istringstream index_stream(index_string);
				unsigned short indexPosition, indexNormal, indexTexcoord;
				index_stream >> indexPosition;
				index_stream.seekg(1, std::ios_base::cur);	//スラッシュを飛ばす
				index_stream >> indexTexcoord;
				index_stream.seekg(1, std::ios_base::cur);	//スラッシュを飛ばす
				index_stream >> indexNormal;

				//頂点データの追加
				Vertex vertex{};
				vertex.pos = positions[indexPosition - 1];
				vertex.normal = normals[indexNormal - 1];
				vertex.uv = texcoords[indexTexcoord - 1];
				vertices.emplace_back(vertex);
				//インデックスデータの追加
				indices.emplace_back(indices.size());
			}
		}
	}

	file.close();

	UINT sizeVB = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());


//頂点バッファの確保
//GPU 側のメモリを確保して、頂点データ用の領域を設定。
//頂点バッファ用 GPU リソースの生成

	// 頂点バッファの生成
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB), // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//インデックスバッファの生成
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB), // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));


	//頂点バッファへのデータ転送
	// GPU 上のバッファに対応した仮想メモリを取得
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	// 全頂点に対して
	for (int i = 0; i < vertices.size(); i++)
	{
		vertMap[i] = vertices[i]; // 座標をコピー
	}
	// マップを解除
	vertBuff->Unmap(0, nullptr);

	// 頂点バッファビューの作成
	vbView.BufferLocation = vertBuff.Get()->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	vbView.StrideInBytes = sizeof(vertices[0]);



	//インデックスバッファへのデータ転送
	// GPU 上のバッファに対応した仮想メモリを取得
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	// 全インデックスに対して
	for (int i = 0; i < indices.size(); i++)
	{
		indexMap[i] = indices[i]; // インデックスをコピー
	}
	indexBuff->Unmap(0, nullptr);

	//インデックスバッファビューの作成とセット
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices[0]) * indices.size();

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

void Object3D::LoadMaterial(const std::string & directoryPath, const std::string & filename)
{
	//ファイルストリーム
	std::ifstream file;
	//マテリアルファイルを開く
	file.open(directoryPath + filename);
	//ファイルオープン失敗をチェック
	if (file.fail()) {
		assert(0);
	}

	//1行ずつ読み込む
	string line;
	while (getline(file, line)) {

		//1行分の文字列をストリームに変換
		std::istringstream line_stream(line);
		//半角スペース区切りで行の先頭文字列を取得
		string key;
		getline(line_stream, key, ' ');

		//先頭のタブ文字は無視する
		if (key[0] == '\t') {
			key.erase(key.begin());//先頭も文字を削除
		}

		//先頭文字列がnewmtlならマテリアル名
		if (key == "newmtl") {
			//マテリアル名読み込み
			line_stream >> material.name;
		}

		//先頭文字がKaならアンビエント色
		if (key == "Ka") {
			line_stream >> material.ambient.x;
			line_stream >> material.ambient.y;
			line_stream >> material.ambient.z;
		}

		//先頭文字列がKdならディフューズ色
		if (key == "Kd") {
			line_stream >> material.diffuse.x;
			line_stream >> material.diffuse.y;
			line_stream >> material.diffuse.z;
		}

		//先頭文字列がKsならスペキュラー色
		if (key == "Ks") {
			line_stream >> material.specular.x;
			line_stream >> material.specular.y;
			line_stream >> material.specular.z;
		}

		//先頭文字列がmap_Kdならテクスチャファイル名
		if (key == "map_Kd") {
			//テクスチャのファイル名読み込み
			line_stream >> material.textureFilename;
			//テクスチャ読み込み
			LoadTexture(directoryPath, material.textureFilename);
		}
	}

	//ファイルを閉じる
	file.close();

}

bool Object3D::LoadTexture(const std::string & directoryPath, const std::string & filename)
{
	HRESULT result = S_FALSE;

	// WICテクスチャのロード
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	//ファイルパスを結合
	string filepath = directoryPath + filename;

	//ユニコード文字列に変換する
	wchar_t wfilepath[128];
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, filepath.c_str(), -1, wfilepath, _countof(wfilepath));
	directoryPath + filename;

	result = LoadFromWICFile(
		wfilepath, WIC_FLAGS_NONE,
		&metadata, scratchImg);
	if (FAILED(result)) {
		return result;
	}

	const Image* img = scratchImg.GetImage(0, 0, 0); // 生データ抽出

	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	// テクスチャ用バッファの生成
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texbuff));
	if (FAILED(result)) {
		return result;
	}

	// テクスチャバッファにデータ転送
	result = texbuff->WriteToSubresource(
		0,
		nullptr, // 全領域へコピー
		img->pixels,    // 元データアドレス
		(UINT)img->rowPitch,  // 1ラインサイズ
		(UINT)img->slicePitch // 1枚サイズ
	);
	if (FAILED(result)) {
		return result;
	}

	// シェーダリソースビュー作成
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(basicDescHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(basicDescHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	DX12Init::GetDevice()->CreateShaderResourceView(texbuff.Get(), //ビューと関連付けるバッファ
		&srvDesc, //テクスチャ設定情報
		cpuDescHandleSRV
	);

	return true;

}

void Object3D::UpdateViewMatrix()
{
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
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
