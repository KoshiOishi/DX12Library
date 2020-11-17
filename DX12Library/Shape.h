#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <vector>
#pragma once
#pragma comment(lib, "d3d12.lib")

class Shape {
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
private:
    int vertSize;
	std::vector<XMFLOAT3> vertices;
	std::vector<unsigned short> indices;
	float R;
	HRESULT* result;
	ID3D12Resource* vertBuff;
	ID3D12Resource* constBuff;
	ID3D12DescriptorHeap* basicDescHeap;
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	D3D12_INDEX_BUFFER_VIEW ibView{};

public:
	Shape(int vertSize, float x, float y, HRESULT* result, float R = 0.5f);
	int GetVertSize();
	void SetVertices(float x, float y, float z, int index);
	void SetIndices(unsigned short indices, int index);
	void ShapeInit(ID3D12Device* dev);
	void constInit(XMFLOAT4 color, ID3D12Device* dev);
	void UpdateVertMap();
	std::vector<XMFLOAT3> GetVertices();
	std::vector<unsigned short> GetIndices();
	D3D12_VERTEX_BUFFER_VIEW GetVbView();
	D3D12_INDEX_BUFFER_VIEW GetIbView();
	ID3D12DescriptorHeap* GetBasicDescHeap();
};