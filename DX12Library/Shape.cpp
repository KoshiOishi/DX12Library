#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <vector>
#include "Shape.h"
#pragma once
#pragma comment(lib, "d3d12.lib")


using namespace DirectX;

Shape::Shape(int vertSize, float x, float y, HRESULT* result, float R)
{
	this->vertSize = vertSize;
	this->R = R;
	this->result = result;

	const int window_width = 1280;
	const int window_height = 720;

	for (int i = 0; i < vertSize; i++)
	{
		vertices.push_back({ R * (float)sin(XM_2PI / vertSize * i) * (float)(window_height) / window_width + x, R * (float)cos(XM_2PI / vertSize * i) + y, 0 });
	}
	vertices.push_back({ 0+x,0+y,0 });

	for (int i = 0; i < vertSize; i++)
	{
		indices.push_back(vertSize);
		indices.push_back(i);
		indices.push_back(i + 1);
	}
	indices[vertSize * 3 - 1] = 0;

}

int Shape::GetVertSize()
{
	return vertSize;
}

void Shape::SetVertices(float x, float y, float z, int index)
{
	this->vertices[index] = XMFLOAT3(x,y,z);
}

void Shape::SetIndices(unsigned short indices, int index)
{
	this->indices[index] = indices;
}

void Shape::constInit(XMFLOAT4 color, ID3D12Device* dev)
{
	//バッファを確保するので、新たに専用のデスクリプタヒープを作成する。
	basicDescHeap = nullptr;
	// 設定構造体
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // シェーダーから見える
	descHeapDesc.NumDescriptors = 1; // バッファは 1 つ
	// 生成
	*result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	// 定数バッファ用データ構造体
	struct ConstBufferData {
		XMFLOAT4 color; // 色 (RGBA)
	};

	//定数バッファの作成
	D3D12_HEAP_PROPERTIES cbheapprop{}; // ヒープ設定
	cbheapprop.Type = D3D12_HEAP_TYPE_UPLOAD; // GPU への転送用
	D3D12_RESOURCE_DESC cbresdesc{}; // リソース設定
	cbresdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbresdesc.Width = (sizeof(ConstBufferData) + 0xff)&~0xff; // 256 バイトアラインメント
	cbresdesc.Height = 1;
	cbresdesc.DepthOrArraySize = 1;
	cbresdesc.MipLevels = 1;
	cbresdesc.SampleDesc.Count = 1;
	cbresdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// GPU リソースの生成
	ID3D12Resource* constBuff = nullptr;
	*result = dev->CreateCommittedResource(
		&cbheapprop, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&cbresdesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));

	////定数バッファにデータを転送する
	/// 
	ConstBufferData* constMap = nullptr;
	*result = constBuff->Map(0, nullptr, (void**)&constMap); // マッピング
	constMap->color = color; // RGBA で半透明の赤
	constBuff->Unmap(0, nullptr); // マッピング解除


	//定数バッファビューの作成
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)constBuff->GetDesc().Width;
	dev->CreateConstantBufferView(
		&cbvDesc, basicDescHeap->GetCPUDescriptorHandleForHeapStart());
}

void Shape::ShapeInit(ID3D12Device* dev)
{
	//頂点バッファの確保
	//GPU 側のメモリを確保して、頂点データ用の領域を設定。
	//頂点バッファ用 GPU リソースの生成
	D3D12_HEAP_PROPERTIES heapprop{}; //頂点ヒープ設定
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD; //GPUへの転送用

	D3D12_RESOURCE_DESC resdesc{}; //リソース設定
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(GetVertices()); // 頂点情報が入る分のサイズ
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// GPU リソースの生成
	vertBuff = nullptr;
	*result = dev->CreateCommittedResource(
		&heapprop, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resdesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//頂点バッファへのデータ転送
	// GPU 上のバッファに対応した仮想メモリを取得
	XMFLOAT3* vertMap = nullptr;
	*result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	// 全頂点に対して
	for (int i = 0; i < GetVertices().size(); i++)
	{
		vertMap[i] = GetVertices()[i]; // 座標をコピー
	}
	// マップを解除
	vertBuff->Unmap(0, nullptr);

	// 頂点バッファビューの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(GetVertices()[0]) * GetVertices().size();
	vbView.StrideInBytes = sizeof(GetVertices()[0]);

	//インデックスバッファの確保
	ID3D12Resource* indexBuff = nullptr;
	resdesc.Width = sizeof(GetIndices()); // インデックス情報が入る分のサイズ
	// GPU リソースの生成
	*result = dev->CreateCommittedResource(
		&heapprop, // ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resdesc, // リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));

	//インデックスバッファへのデータ転送
	// GPU 上のバッファに対応した仮想メモリを取得
	unsigned short* indexMap = nullptr;
	*result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	// 全インデックスに対して
	for (int i = 0; i < GetIndices().size(); i++)
	{
		indexMap[i] = GetIndices()[i]; // インデックスをコピー
	}
	indexBuff->Unmap(0, nullptr);

	//インデックスバッファビューの作成とセット
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(GetIndices()[0]) * GetIndices().size();
}

void Shape::UpdateVertMap()
{
	//頂点バッファへのデータ転送
// GPU 上のバッファに対応した仮想メモリを取得
	XMFLOAT3* vertMap = nullptr;
	*result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	// 全頂点に対して
	for (int i = 0; i < vertices.size(); i++)
	{
		vertMap[i] = vertices[i]; // 座標をコピー
	}
	// マップを解除
	vertBuff->Unmap(0, nullptr);

}

std::vector<XMFLOAT3> Shape::GetVertices()
{
	return vertices;
}

std::vector<unsigned short> Shape::GetIndices()
{
	return indices;
}

D3D12_VERTEX_BUFFER_VIEW Shape::GetVbView()
{
	return vbView;
}

D3D12_INDEX_BUFFER_VIEW Shape::GetIbView()
{
	return ibView;
}

ID3D12DescriptorHeap* Shape::GetBasicDescHeap()
{
	return basicDescHeap;
}



