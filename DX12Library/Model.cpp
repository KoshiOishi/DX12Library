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

void Model::LoadOBJ(const std::string & modelname, int index)
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
			}
		}
	}

	file.close();
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
			LoadTexture("Resources/", material.textureFilename, index);
			isLoadTexture = true;
		}
	}

	//ファイルを閉じる
	file.close();

	if (!isLoadTexture)
	{
		//テクスチャがないときは白テクスチャを読み込む
		LoadTexture(directoryPath, "white1x1.png", index);
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

void Model::CreateBox(float width, float height, float depth, int index)
{
	isOBJ = false;

	material.name = "default";

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
	}
	//面2 2367
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 2,3,6,7 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
	}
	//面3 6745
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 6,7,4,5 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
	}
	//面4 4501
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 4,5,0,1 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
	}
	//面5 1537
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 1,5,3,7 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
	}
	//面6 4062
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 4,0,6,2 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
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
	
	CreateBuffer();
}

void Model::CreateSphere(float radius, int index)
{
}

void Model::CreatePoll(int vertex, float radius, float height, int index)
{
	isOBJ = false;

	material.name = "default";

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
		float rad = (f * i) * (3.14159265f / 180);
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
		float rad = (f * i) * (3.14159265f / 180);
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
			vertices.emplace_back(v[0]);
			vertices.emplace_back(v[i + 2]);
		}
		else
		{
			vertices.emplace_back(v[i + 1]);
			vertices.emplace_back(v[0]);
			vertices.emplace_back(v[vertex - i]);
		}
	}

	//下面
	for (int i = vertex; i < vertex * 2; i++)
	{
		if (i + 1 < vertex * 2)
		{
			vertices.emplace_back(v[i + 2]);
			vertices.emplace_back(v[i + 3]);
			vertices.emplace_back(v[vertex + 1]);
		}
		else
		{
			vertices.emplace_back(v[i + 2]);
			vertices.emplace_back(v[vertex * 2 - i + vertex + 1]);
			vertices.emplace_back(v[vertex + 1]);
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
		vertices.emplace_back(v[n[0]]);
		vertices.emplace_back(v[n[3]]);
		vertices.emplace_back(v[n[3]]);
		vertices.emplace_back(v[n[0]]);
		vertices.emplace_back(v[n[1]]);

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

	CreateBuffer();
}