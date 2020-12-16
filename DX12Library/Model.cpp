#include "Model.h"
#include <fstream>
#include <sstream>
#include "DX12Util.h"
#include <DirectXTex.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include "Object3D.h"

using namespace DirectX;

void Model::Initialize()
{
	HRESULT result;

	//定数バッファの作成
	result = DX12Util::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB1) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffB1));
}

void Model::Update()
{
	HRESULT result;

	// 定数バッファへデータ転送
	ConstBufferDataB1* constMap1 = nullptr;
	result = constBuffB1->Map(0, nullptr, (void**)&constMap1);
	constMap1->ambient = material.ambient;
	constMap1->diffuse = material.diffuse;
	constMap1->specular = material.specular;
	constMap1->alpha = material.alpha;
	constBuffB1->Unmap(0, nullptr);
}

void Model::Draw()
{
	//インデックスバッファのセットコマンド
	DX12Util::GetCmdList()->IASetIndexBuffer(&ibView);

	//プリミティブ形状の設定コマンド(三角形リスト)
	DX12Util::GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//頂点バッファの設定コマンド
	DX12Util::GetCmdList()->IASetVertexBuffers(0, 1, &vbView);

	DX12Util::GetCmdList()->SetGraphicsRootConstantBufferView(1, constBuffB1->GetGPUVirtualAddress());
	
	// シェーダリソースビューをセット
	DX12Util::GetCmdList()->SetGraphicsRootDescriptorTable(2, gpuDescHandleSRV);

	//描画コマンド
	DX12Util::GetCmdList()->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);
}

void Model::CreateFromOBJ(const std::string & modelname, int index, bool smoothing)
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
			LoadMaterial(directoryPath, filename, index);
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
				//エッジ平滑化用のデータを追加
				if (smoothing) {
					//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
					AddSmoothData(indexPosition, (unsigned short)vertices.size() - 1);
				}
			}
		}
	}

	file.close();

	if (smoothing) {
		CalculateSmoothedVertexNormals();
	}

	CreateBuffer();
}

void Model::LoadMaterial(const std::string & directoryPath, const std::string & filename, int index)
{
	bool isLoadTexture = false;

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
			LoadTexture(directoryPath, material.textureFilename, index);
			isLoadTexture = true;
		}
	}

	//ファイルを閉じる
	file.close();

	if (!isLoadTexture)
	{
		//テクスチャがないときは白テクスチャを読み込む
		LoadTexture("Resources/", "white1x1.png", index);
	}
}

bool Model::LoadTexture(const std::string & directoryPath, const std::string & filename, int index)
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
	result = DX12Util::GetDevice()->CreateCommittedResource(
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

	// デスクリプタサイズを取得
	UINT descriptorHandleIncrementSize = DX12Util::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// シェーダリソースビュー作成
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(Object3D::GetDescHeap()->GetCPUDescriptorHandleForHeapStart(), index, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(Object3D::GetDescHeap()->GetGPUDescriptorHandleForHeapStart(), index, descriptorHandleIncrementSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	DX12Util::GetDevice()->CreateShaderResourceView(texbuff.Get(), //ビューと関連付けるバッファ
		&srvDesc, //テクスチャ設定情報
		cpuDescHandleSRV
	);

	return true;

}

bool Model::LoadTextureReturnTexSize(const std::string & directoryPath, const std::string & filename, int index, float* texWidth, float* texHeight)
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

	result = LoadFromWICFile(
		wfilepath, WIC_FLAGS_NONE,
		&metadata, scratchImg);
	if (FAILED(result)) {
		return false;
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
	result = DX12Util::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texbuff));
	if (FAILED(result)) {
		return false;
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
		return false;
	}

	// デスクリプタサイズを取得
	UINT descriptorHandleIncrementSize = DX12Util::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// シェーダリソースビュー作成
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(Object3D::GetDescHeap()->GetCPUDescriptorHandleForHeapStart(), index, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(Object3D::GetDescHeap()->GetGPUDescriptorHandleForHeapStart(), index, descriptorHandleIncrementSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	DX12Util::GetDevice()->CreateShaderResourceView(texbuff.Get(), //ビューと関連付けるバッファ
		&srvDesc, //テクスチャ設定情報
		cpuDescHandleSRV
	);

	if (texWidth != nullptr)
		*texWidth = metadata.width;
	if (texHeight != nullptr)
		*texHeight = metadata.height;
	return false;
}

void Model::CreateBuffer()
{
	HRESULT result;

	UINT sizeVB = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());


	//頂点バッファの確保
	//GPU 側のメモリを確保して、頂点データ用の領域を設定。
	//頂点バッファ用 GPU リソースの生成

		// 頂点バッファの生成
	result = DX12Util::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB), // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//インデックスバッファの生成
	result = DX12Util::GetDevice()->CreateCommittedResource(
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

void Model::AddSmoothData(unsigned short indexPosition, unsigned short indexVertex)
{
	smoothData[indexPosition].emplace_back(indexVertex);
}

void Model::CalculateSmoothedVertexNormals()
{
	auto itr = smoothData.begin();
	for (; itr != smoothData.end(); ++itr)
	{
		//各面用の共通頂点コレクション
		std::vector<unsigned short>& v = itr->second;
		//全頂点の法線を平均する
		XMVECTOR normal = {};
		for (unsigned short index : v) {
			normal += XMVectorSet(vertices[index].normal.x, vertices[index].normal.y, vertices[index].normal.z, 0);
		}
		normal = XMVector3Normalize(normal / (float)v.size());
		//共通座標を使用する全ての頂点データに書き込む
		for (unsigned short index : v) {
			vertices[index].normal = { normal.m128_f32[0],normal.m128_f32[1], normal.m128_f32[2] };
		}
	}

}

void Model::CreateBox(float width, float height, float depth, int index, bool smoothing)
{
	isLoadFromOBJFile = false;

	material.ambient = { 1,1,1 };
	material.diffuse = { 0.5f,0.5f,0.5f };

	material.name = "default_box";

	material.textureFilename = "white1x1.png";
	LoadTexture("Resources/" , material.textureFilename, index);
	
	float x = width / 2;
	float y = height / 2;
	float z = depth / 2;

	//頂点計算
	Vertex vertex{};
	XMFLOAT3 samplePos[8];
	XMFLOAT2 sampleUV[4];
	//手前
	samplePos[0] = { -x, -y, -z };//左下
	samplePos[1] = { -x,  y, -z };//左上
	samplePos[2] = {  x, -y, -z };//右下
	samplePos[3] = {  x,  y, -z };//右上
	//奥
	samplePos[4] = { -x, -y,  z };//左下
	samplePos[5] = { -x,  y,  z };//左上
	samplePos[6] = {  x, -y,  z };//右下
	samplePos[7] = {  x,  y,  z };//右上

	sampleUV[0] = { 0,1 };//左下
	sampleUV[1] = { 0,0 };//左上
	sampleUV[2] = { 1,1 };//右下
	sampleUV[3] = { 1,0 };//右下

	int nUV[6]{ 0,1,2,2,1,3 };

	//面1 0123
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 0,1,2,3 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}
	//面2 2367
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 2,3,6,7 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}
	//面3 6745
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 6,7,4,5 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}
	//面4 4501
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 4,5,0,1 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}
	//面5 1537
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 1,5,3,7 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}
	//面6 4062
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 4,0,6,2 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}

	//インデックス配列
	for (int i = 0; i < 36; i++)
	{
		indices.emplace_back(i);
	}

	
	//法線の計算
	for (int i = 0; i < vertices.size() / 3; i++)
	{
		//三角形1つごとに計算していく
		//三角形のインデックスを取り出して、一時的な変数に入れる
		unsigned short index0 = indices[i * 3 + 0];
		unsigned short index1 = indices[i * 3 + 1];
		unsigned short index2 = indices[i * 3 + 2];
		//三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//p0⇒p1ベクトル、p0⇒p2ベクトルを計算（ベクトルの減算）
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//正規化（長さを1にする）
		normal = XMVector3Normalize(normal);
		//求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[index0].normal, normal);
		XMStoreFloat3(&vertices[index1].normal, normal);
		XMStoreFloat3(&vertices[index2].normal, normal);

	}

	if (smoothing) {
		CalculateSmoothedVertexNormals();
	}
	
	CreateBuffer();
}

void Model::CreateSphere(int vertexX, int vertexY, float radius, int index, bool smoothing)
{
	isLoadFromOBJFile = false;

	material.ambient = { 0.5f,0.5f,0.5f };
	material.diffuse = { 0.5f,0.5f,0.5f };

	material.name = "default_sphere";

	material.textureFilename = "white1x1.png";
	LoadTexture("Resources/", material.textureFilename, index);


	if (vertexX < 3 || vertexY < 3) {
		assert(0);
		return;
	}

	Vertex upper, downer;
	//upper...0 downer...1 side...side + 2
	upper.pos = { 0,radius,0 };
	downer.pos = { 0,-radius,0 };

	vector<vector<Vertex>> side;

	for (int i = 1; i < vertexY; i++)
	{
		float fY = 180.0f / vertexY;
		float radY = XMConvertToRadians(fY * i);
		vector<Vertex> v;
		side.emplace_back(v);
		for (int j = 0; j < vertexX; j++)
		{
			Vertex vert;
			float fX = 360.0f / vertexX;
			float radXZ = XMConvertToRadians(fX * j);
			vert.pos = { std::cos(radXZ) * std::sin(radY) * radius, std::cos(radY) * radius, std::sin(radXZ) * std::sin(radY) * radius };
			side[i - 1].emplace_back(vert);
		}
	}

	//上
	for (int i = 0; i < vertexX; i++)
	{
		Vertex v[3];
		unsigned short indice[3];
		if (i < vertexX - 1)
		{
			v[0] = upper;
			v[1] = side[0][i + 1];
			v[2] = side[0][i];
			indice[0] = 0;
			indice[1] = i + 1 + 2;
			indice[2] = i + 2;
		}
		else
		{
			v[0] = upper;
			v[1] = side[0][0];
			v[2] = side[0][i];
			indice[0] = 0;
			indice[1] = 2;
			indice[2] = i + 2;
		}
		for (int j = 0; j < 3; j++) 
		{
			vertices.emplace_back(v[j]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(indice[j], (unsigned short)vertices.size() - 1);
			}
		}
	}

	//下
	for (int i = 0; i < vertexX; i++)
	{
		Vertex v[3];
		unsigned short indice[3];
		if (i < vertexX - 1)
		{
			v[0] = downer;
			v[1] = side[vertexY - 2][i];
			v[2] = side[vertexY - 2][i + 1];
			indice[0] = 1;
			indice[1] = (vertexY - 2) * vertexX + i + 2;
			indice[2] = (vertexY - 2) * vertexX + i + 1 + 2;
		}
		else
		{
			v[0] = downer;
			v[1] = side[vertexY - 2][i];
			v[2] = side[vertexY - 2][0];
			indice[0] = 1;
			indice[1] = (vertexY - 2) * vertexX + i + 2;
			indice[2] = (vertexY - 2) * vertexX + 2;
		}
		for (int j = 0; j < 3; j++)
		{
			vertices.emplace_back(v[j]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(indice[j], (unsigned short)vertices.size() - 1);
			}
		}
	}

	//横
	for (int i = 0; i < vertexY - 2; i++)
	{
		for (int j = 0; j < vertexX; j++)
		{
			Vertex v[6];
			unsigned short indice[6];
			if (j < vertexX - 1)
			{
				v[0] = side[i + 1][j];	//左下
				v[1] = side[i][j];		//左上
				v[2] = side[i + 1][j + 1];	//右下
				v[3] = side[i + 1][j + 1];	//右下
				v[4] = side[i][j];		//左上
				v[5] = side[i][j + 1];		//右上
				indice[0] = (i + 1) * vertexX + j + 2;
				indice[1] = (i) * vertexX + j + 2;
				indice[2] = (i + 1) * vertexX + j + 1 + 2;
				indice[3] = (i + 1) * vertexX + j + 1 + 2;
				indice[4] = (i) * vertexX + j + 2;
				indice[5] = (i) * vertexX + j + 1 + 2;

			}
			else
			{
				v[0] = side[i + 1][j];	//左下
				v[1] = side[i][j];		//左上
				v[2] = side[i + 1][0];	//右下
				v[3] = side[i + 1][0];	//右下
				v[4] = side[i][j];		//左上
				v[5] = side[i][0];		//右上
				indice[0] = (i + 1) * vertexX + j + 2;
				indice[1] = (i)* vertexX + j + 2;
				indice[2] = (i + 1) * vertexX + 2;
				indice[3] = (i + 1) * vertexX + 2;
				indice[4] = (i)* vertexX + j + 2;
				indice[5] = (i)* vertexX + 2;
			}
			for (int k = 0; k < 6; k++)
			{
				vertices.emplace_back(v[k]);
				//エッジ平滑化用のデータを追加
				if (smoothing) {
					//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
					AddSmoothData(indice[k], (unsigned short)vertices.size() - 1);
				}
			}
		}
	}


	for (int i = 0; i < vertexX * 6 + vertexX * 6 * (vertexY - 2); i++)
	{
		indices.emplace_back(i);
	}

	//法線の計算
	for (int i = 0; i < vertices.size() / 3; i++)
	{
		//三角形1つごとに計算していく
		//三角形のインデックスを取り出して、一時的な変数に入れる
		unsigned short index0 = indices[i * 3 + 0];
		unsigned short index1 = indices[i * 3 + 1];
		unsigned short index2 = indices[i * 3 + 2];
		//三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//p0⇒p1ベクトル、p0⇒p2ベクトルを計算（ベクトルの減算）
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//正規化（長さを1にする）
		normal = XMVector3Normalize(normal);
		//求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[index0].normal, normal);
		XMStoreFloat3(&vertices[index1].normal, normal);
		XMStoreFloat3(&vertices[index2].normal, normal);

	}

	if (smoothing) {
		CalculateSmoothedVertexNormals();
	}

	CreateBuffer();
}

void Model::CreatePoll(int vertex, float radius, float height, int index, bool smoothing)
{
	isLoadFromOBJFile = false;

	material.ambient = { 0.5f,0.5f,0.5f };
	material.diffuse = { 0.5f,0.5f,0.5f };

	material.name = "default_poll";

	material.textureFilename = "white1x1.png";
	LoadTexture("Resources/", material.textureFilename, index);

	vector<Vertex> v;
	Vertex v0_up;
	v0_up.pos = { 0,height / 2,0 };
	v.emplace_back(v0_up);

	//天井の座標も
	for (int i = 0; i < vertex; i++)
	{
		Vertex vert;
		float f = 360.0f / vertex;
		float rad = XMConvertToRadians(f * i);
		vert.pos = { std::cos(rad) * radius, height / 2, std::sin(rad) * radius };
		v.emplace_back(vert);
	}

	Vertex v0_down;
	v0_down.pos = { 0,-height / 2,0 };
	v.emplace_back(v0_down);

	//底面の頂点座標を求める
	for (int i = 0; i < vertex; i++)
	{
		Vertex vert;
		float f = 360.0f / vertex;
		float rad = XMConvertToRadians(f * i);
		vert.pos = { std::cos(rad) * radius, -height / 2, std::sin(rad) * radius };
		v.emplace_back(vert);
	}

	//for (int i = 0; i < vertex * 2 + 2; i++)
	//	v[i].pos.x = i;

	//格納
	//上面
	for (int i = 0; i < vertex; i++)
	{
		if (i + 1 < vertex)
		{
			vertices.emplace_back(v[i + 1]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(i + 1, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[0]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(0, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[i + 2]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(i + 2, (unsigned short)vertices.size() - 1);
			}
		}
		else
		{
			vertices.emplace_back(v[i + 1]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(i + 1, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[0]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(0, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[vertex - i]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(vertex - i, (unsigned short)vertices.size() - 1);
			}
		}
	}

	//下面
	for (int i = vertex; i < vertex * 2; i++)
	{
		if (i + 1 < vertex * 2)
		{
			vertices.emplace_back(v[i + 2]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(i + 2, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[i + 3]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(i + 3, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[vertex + 1]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(vertex + 1, (unsigned short)vertices.size() - 1);
			}
		}
		else
		{
			vertices.emplace_back(v[i + 2]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(i + 2, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[vertex * 2 - i + vertex + 1]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(vertex * 2 - i + vertex + 1, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[vertex + 1]);
			//エッジ平滑化用のデータを追加
			if (smoothing) {
				//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
				AddSmoothData(vertex + 1, (unsigned short)vertices.size() - 1);
			}
		}
	}


	for (int i = 0; i < vertex; i++)
	{
		int n[4];
		if (i + 1 < vertex)
		{
			n[0] = i + 1;
			n[1] = i + 2;
			n[2] = vertex + 2 + i;
			n[3] = vertex + 3 + i;
		}
		else
		{
			n[0] = i + 1;
			n[1] = 1;
			n[2] = vertex + 2 + i;
			n[3] = vertex + 2;
		}
		vertices.emplace_back(v[n[2]]);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[2], (unsigned short)vertices.size() - 1);
		}
		vertices.emplace_back(v[n[0]]);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[0], (unsigned short)vertices.size() - 1);
		}
		vertices.emplace_back(v[n[3]]);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[3], (unsigned short)vertices.size() - 1);
		}
		vertices.emplace_back(v[n[3]]);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[3], (unsigned short)vertices.size() - 1);
		}
		vertices.emplace_back(v[n[0]]);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[0], (unsigned short)vertices.size() - 1);
		}
		vertices.emplace_back(v[n[1]]);
		//エッジ平滑化用のデータを追加
		if (smoothing) {
			//vキー(座標データ)の番号と、全て合成した頂点のインデックスをセットで登録する
			AddSmoothData(n[1], (unsigned short)vertices.size() - 1);
		}

	}

	//インデックス
	for (int i = 0; i < vertex * 3 * 2 + 6 * vertex; i++)
	{
		indices.emplace_back(i);
	}

	//for (int i = vertices.size(); i > vertex * 6; i--)
	//{
	//	vertices.pop_back();
	//	indices.pop_back();
	//}

	//法線の計算
	for (int i = 0; i < vertices.size() / 3; i++)
	{
		//三角形1つごとに計算していく
		//三角形のインデックスを取り出して、一時的な変数に入れる
		unsigned short index0 = indices[i * 3 + 0];
		unsigned short index1 = indices[i * 3 + 1];
		unsigned short index2 = indices[i * 3 + 2];
		//三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//p0⇒p1ベクトル、p0⇒p2ベクトルを計算（ベクトルの減算）
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//正規化（長さを1にする）
		normal = XMVector3Normalize(normal);
		//求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[index0].normal, normal);
		XMStoreFloat3(&vertices[index1].normal, normal);
		XMStoreFloat3(&vertices[index2].normal, normal);

	}

	if (smoothing) {
		CalculateSmoothedVertexNormals();
	}

	CreateBuffer();
}

void Model::CreateSquare(float width, float height, int index)
{
	isLoadFromOBJFile = false;

	material.ambient = { 0.5f,0.5f,0.5f };
	material.diffuse = { 0.5f,0.5f,0.5f };

	material.name = "default_square";

	material.textureFilename = "white1x1.png";
	LoadTexture("Resources/", material.textureFilename, index);

	float div2Width = width / 2.0f;
	float div2Height = height / 2.0f;

	vertices.push_back({ {-div2Width,-div2Height,0.0f},{0,0,1},{0,1} });	//左下
	vertices.push_back({ {-div2Width,+div2Height,0.0f},{0,0,1},{0,0} });	//左上
	vertices.push_back({ {+div2Width,-div2Height,0.0f},{0,0,1},{1,1} });	//右下
	vertices.push_back({ {+div2Width,+div2Height,0.0f},{0,0,1},{1,0} });	//右上

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);//三角形1

	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);//三角形2

	CreateBuffer();

}

void Model::CreateSquareTex(float standardLength, std::string texName, int index)
{
	isLoadFromOBJFile = false;

	material.ambient = { 0.5f,0.5f,0.5f };
	material.diffuse = { 0.5f,0.5f,0.5f };

	material.name = "square_tex";

	material.textureFilename = texName;
	float* texWidth = new float();
	float* texHeight = new float();
	LoadTextureReturnTexSize("Resources/", material.textureFilename, index, texWidth, texHeight);

	float than = 0.0f;
	float div2Width = 0.0f;
	float div2Height = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	if (*texWidth >= *texHeight)//横長
	{
		than = *texWidth / *texHeight;
		width = standardLength * than;
		height = standardLength;
	}
	else
	{
		than = *texHeight / *texWidth;
		width = standardLength;
		height = standardLength * than;
	}
	div2Width = width / 2.0f;
	div2Height = height / 2.0f;

	vertices.push_back({ {-div2Width,-div2Height,0.0f},{0,0,1},{0,1} });	//左下
	vertices.push_back({ {-div2Width,+div2Height,0.0f},{0,0,1},{0,0} });	//左上
	vertices.push_back({ {+div2Width,-div2Height,0.0f},{0,0,1},{1,1} });	//右下
	vertices.push_back({ {+div2Width,+div2Height,0.0f},{0,0,1},{1,0} });	//右上

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);//三角形1

	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);//三角形2

	delete texWidth;
	delete texHeight;

	CreateBuffer();
}
