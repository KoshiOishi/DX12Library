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
	//�o�b�t�@���m�ۂ���̂ŁA�V���ɐ�p�̃f�X�N���v�^�q�[�v���쐬����B
	basicDescHeap = nullptr;
	// �ݒ�\����
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // �V�F�[�_�[���猩����
	descHeapDesc.NumDescriptors = 1; // �o�b�t�@�� 1 ��
	// ����
	*result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	// �萔�o�b�t�@�p�f�[�^�\����
	struct ConstBufferData {
		XMFLOAT4 color; // �F (RGBA)
	};

	//�萔�o�b�t�@�̍쐬
	D3D12_HEAP_PROPERTIES cbheapprop{}; // �q�[�v�ݒ�
	cbheapprop.Type = D3D12_HEAP_TYPE_UPLOAD; // GPU �ւ̓]���p
	D3D12_RESOURCE_DESC cbresdesc{}; // ���\�[�X�ݒ�
	cbresdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbresdesc.Width = (sizeof(ConstBufferData) + 0xff)&~0xff; // 256 �o�C�g�A���C�������g
	cbresdesc.Height = 1;
	cbresdesc.DepthOrArraySize = 1;
	cbresdesc.MipLevels = 1;
	cbresdesc.SampleDesc.Count = 1;
	cbresdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// GPU ���\�[�X�̐���
	ID3D12Resource* constBuff = nullptr;
	*result = dev->CreateCommittedResource(
		&cbheapprop, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&cbresdesc, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));

	////�萔�o�b�t�@�Ƀf�[�^��]������
	/// 
	ConstBufferData* constMap = nullptr;
	*result = constBuff->Map(0, nullptr, (void**)&constMap); // �}�b�s���O
	constMap->color = color; // RGBA �Ŕ������̐�
	constBuff->Unmap(0, nullptr); // �}�b�s���O����


	//�萔�o�b�t�@�r���[�̍쐬
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)constBuff->GetDesc().Width;
	dev->CreateConstantBufferView(
		&cbvDesc, basicDescHeap->GetCPUDescriptorHandleForHeapStart());
}

void Shape::ShapeInit(ID3D12Device* dev)
{
	//���_�o�b�t�@�̊m��
	//GPU ���̃��������m�ۂ��āA���_�f�[�^�p�̗̈��ݒ�B
	//���_�o�b�t�@�p GPU ���\�[�X�̐���
	D3D12_HEAP_PROPERTIES heapprop{}; //���_�q�[�v�ݒ�
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD; //GPU�ւ̓]���p

	D3D12_RESOURCE_DESC resdesc{}; //���\�[�X�ݒ�
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(GetVertices()); // ���_��񂪓��镪�̃T�C�Y
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// GPU ���\�[�X�̐���
	vertBuff = nullptr;
	*result = dev->CreateCommittedResource(
		&heapprop, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resdesc, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//���_�o�b�t�@�ւ̃f�[�^�]��
	// GPU ��̃o�b�t�@�ɑΉ��������z���������擾
	XMFLOAT3* vertMap = nullptr;
	*result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	// �S���_�ɑ΂���
	for (int i = 0; i < GetVertices().size(); i++)
	{
		vertMap[i] = GetVertices()[i]; // ���W���R�s�[
	}
	// �}�b�v������
	vertBuff->Unmap(0, nullptr);

	// ���_�o�b�t�@�r���[�̍쐬
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(GetVertices()[0]) * GetVertices().size();
	vbView.StrideInBytes = sizeof(GetVertices()[0]);

	//�C���f�b�N�X�o�b�t�@�̊m��
	ID3D12Resource* indexBuff = nullptr;
	resdesc.Width = sizeof(GetIndices()); // �C���f�b�N�X��񂪓��镪�̃T�C�Y
	// GPU ���\�[�X�̐���
	*result = dev->CreateCommittedResource(
		&heapprop, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resdesc, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));

	//�C���f�b�N�X�o�b�t�@�ւ̃f�[�^�]��
	// GPU ��̃o�b�t�@�ɑΉ��������z���������擾
	unsigned short* indexMap = nullptr;
	*result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	// �S�C���f�b�N�X�ɑ΂���
	for (int i = 0; i < GetIndices().size(); i++)
	{
		indexMap[i] = GetIndices()[i]; // �C���f�b�N�X���R�s�[
	}
	indexBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[�̍쐬�ƃZ�b�g
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(GetIndices()[0]) * GetIndices().size();
}

void Shape::UpdateVertMap()
{
	//���_�o�b�t�@�ւ̃f�[�^�]��
// GPU ��̃o�b�t�@�ɑΉ��������z���������擾
	XMFLOAT3* vertMap = nullptr;
	*result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	// �S���_�ɑ΂���
	for (int i = 0; i < vertices.size(); i++)
	{
		vertMap[i] = vertices[i]; // ���W���R�s�[
	}
	// �}�b�v������
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



