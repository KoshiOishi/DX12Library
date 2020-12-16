#include <d3dx12.h>
#include "DX12Util.h"
#include "Light.h"

using namespace DirectX;

void Light::Initialize()
{
	HRESULT result;
	//�萔�o�b�t�@�̐���
	result = DX12Util::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff)
	);
	if (FAILED(result)) { assert(0); }

	//�萔�o�b�t�@�փf�[�^�]��
	TransferConstBuffer();
}

void Light::Update()
{
	//�l�̕ύX���������������萔�o�b�t�@�ɓ]������
	if (dirty) {
		TransferConstBuffer();
		dirty = false;
	}
}

void Light::Draw(UINT rootParameterIndex)
{
	//�萔�o�b�t�@�r���[���Z�b�g
	DX12Util::GetCmdList()->SetGraphicsRootConstantBufferView(rootParameterIndex,constBuff->GetGPUVirtualAddress());
}

void Light::TransferConstBuffer()
{
	HRESULT result;
	//�萔�o�b�t�@�փf�[�^�]��
	ConstBufferData* constMap = nullptr;
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(result)) {
		constMap->lightv = -lightdir;
		constMap->lightcolor = lightcolor;
		constBuff->Unmap(0, nullptr);
	}
}

void Light::SetLightDir(const XMVECTOR & lightdir)
{
	//���K�����ăZ�b�g
	this->lightdir = XMVector3Normalize(lightdir);
	dirty = true;
}

void Light::SetLightColor(const XMFLOAT3 & lightcolor)
{
	this->lightcolor = lightcolor;
	dirty = true;
}
