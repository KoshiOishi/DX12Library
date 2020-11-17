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


ComPtr<ID3D12RootSignature> Sprite::spriteRootSignature = nullptr;	//���[�g�V�O�l�`��
ComPtr<ID3D12PipelineState> Sprite::spritePipelineState = nullptr;	//�p�C�v���C���X�e�[�g
ComPtr<ID3DBlob> Sprite::vsBlob = nullptr; // ���_�V�F�[�_�I�u�W�F�N�g
ComPtr<ID3DBlob> Sprite::psBlob = nullptr; // �s�N�Z���V�F�[�_�I�u�W�F�N�g
ComPtr<ID3DBlob> Sprite::errorBlob = nullptr; // �G���[�I�u�W�F�N�g
XMMATRIX Sprite::spriteMatProjection;		//�ˉe�s��
ComPtr <ID3D12DescriptorHeap> Sprite::spriteDescHeap = nullptr;
const int Sprite::spriteSRVCount = 512;
ComPtr <ID3D12Resource> Sprite::spriteTexbuff[Sprite::spriteSRVCount];	//�e�N�X�`���o�b�t�@


HRESULT Sprite::Initialize()
{
	HRESULT result = S_FALSE;

	//�e�V�F�[�_�t�@�C���̓ǂݍ��݂ƃR���p�C��
	vsBlob = nullptr; // ���_�V�F�[�_�I�u�W�F�N�g
	psBlob = nullptr; // �s�N�Z���V�F�[�_�I�u�W�F�N�g
	errorBlob = nullptr; // �G���[�I�u�W�F�N�g
	// ���_�V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"SpriteVertexShader.hlsl", // �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"VSmain", "vs_5_0", // �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&vsBlob, &errorBlob);

	if (FAILED(result)) {
		// errorBlob ����G���[���e�� string �^�ɃR�s�[
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// �s�N�Z���V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"SpritePixelShader.hlsl", // �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"PSmain", "ps_5_0", // �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&psBlob, &errorBlob);

	if (FAILED(result)) {
		// errorBlob ����G���[���e�� string �^�ɃR�s�[
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	//���_���C�A�E�g�z��̐錾�ƁA�e�퍀�ڂ̐ݒ�
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

	//�O���t�B�b�N�X�p�C�v���C���̊e�X�e�[�W�̐ݒ������\���̂�p�ӂ���B
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};

	//���_�V�F�[�_�A�s�N�Z���V�F�[�_���p�C�v���C���ɐݒ�
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());


	//�T���v���}�X�N�ƃ��X�^���C�U�X�e�[�g�̐ݒ�
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // �W���ݒ�
	//�W���I�Ȑݒ�(�w�ʃJ�����O�A�h��Ԃ��A�[�x�N���b�s���O�L��)
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);		//��U�W���l���Z�b�g
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;				//�J�����O���Ȃ�

	//�u�����h�X�e�[�g�̐ݒ�
	// �����_�[�^�[�Q�b�g�̃u�����h�ݒ�(8 ���邪���܂͈�����g��Ȃ�)
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // �W���ݒ�

	blenddesc.BlendEnable = true; // �u�����h��L���ɂ���
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; // ���Z
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE; // �\�[�X�̒l�� 100% �g��
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO; // �f�X�g�̒l�� 0% �g��

	blenddesc.BlendOp = D3D12_BLEND_OP_ADD; // ���Z
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA; // �\�[�X�̃A���t�@�l
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // 1.0f-�\�[�X�̃A���t�@�l

	// �u�����h�X�e�[�g�ɐݒ肷��
	gpipeline.BlendState.RenderTarget[0] = blenddesc;


	//���_���C�A�E�g�̐ݒ�
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	//�}�`�̌`����O�p�`�ɐݒ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//���̑��̐ݒ�
	gpipeline.NumRenderTargets = 1; // �`��Ώۂ� 1 ��
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0~255 �w��� RGBA
	gpipeline.SampleDesc.Count = 1; // 1 �s�N�Z���ɂ� 1 ��T���v�����O

	//�f�v�X�X�e���V���X�e�[�g�̐ݒ�
	//�W���I�Ȑݒ�(�[�x�e�X�g���s���A�������݋��A�[�x����������΍��i)
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);	//��U�W���l���Z�b�g
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;		//��ɏ㏑�����[��
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//�f�X�N���v�^�e�[�u���̐ݒ�
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); //t0 ���W�X�^

	//���[�g�p�����[�^�̐ݒ�
	CD3DX12_ROOT_PARAMETER rootparams[2];
	//�萔�p
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);	//�萔�o�b�t�@�r���[�Ƃ��ď�����(b0)
	//�e�N�X�`���p
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	//�T���v���[�̐ݒ�
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	//���[�g�V�O�l�`���̐���
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr <ID3DBlob> rootSigBlob;
	//�o�[�W������������ł̃V���A���C�Y
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);

	result = DX12Init::GetDevice()->CreateRootSignature(0, rootSigBlob.Get()->GetBufferPointer(), rootSigBlob.Get()->GetBufferSize(),
		IID_PPV_ARGS(&spriteRootSignature));

	// �p�C�v���C���Ƀ��[�g�V�O�l�`�����Z�b�g
	gpipeline.pRootSignature = spriteRootSignature.Get();

	//�p�C�v���C���X�e�[�g�̐���
	result = DX12Init::GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&spritePipelineState));


	//�X�v���C�g�p�f�X�N���v�^�q�[�v�̐���
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

	//WIC�e�N�X�`���̃��[�h
	TexMetadata metadata{};
	ScratchImage scratchimg{};

	result = LoadFromWICFile(
		filename,		//�uResources�v�t�H���_�́ugazoudayo.png�v
		WIC_FLAGS_NONE,
		&metadata,
		scratchimg
	);

	const Image* img = scratchimg.GetImage(0, 0, 0);	//���f�[�^���o

		//�e�N�X�`���o�b�t�@�̐���
	//GPU���Ƀe�N�X�`���o�b�t�@���쐬����
	D3D12_HEAP_PROPERTIES texHeapProp{};	//�e�N�X�`���q�[�v�ݒ�
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

	result = DX12Init::GetDevice()->CreateCommittedResource(	//GPU���\�[�X�̐���
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,	//�e�N�X�`���p�w��
		nullptr,
		IID_PPV_ARGS(&spriteTexbuff[texnumber])
	);

	//�f�[�^�]��
	//�e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	result = spriteTexbuff[texnumber]->WriteToSubresource(
		0,
		nullptr,	//�S�̈�ɃR�s�[
		img->pixels,	//���f�[�^�A�h���X
		(UINT)img->rowPitch,	//1���C���T�C�Y
		(UINT)img->slicePitch	//�S�T�C�Y
	);

	//���ۂɃV�F�[�_�[���\�[�X�r���[�𐶐�����
	//�V�F�[�_�[���\�[�X�r���[�쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};	//�ݒ�\����
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;	//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;

	DX12Init::GetDevice()->CreateShaderResourceView(
		spriteTexbuff[texnumber].Get(),	//�r���[�Ɗ֘A�t����o�b�t�@
		&srvDesc,	//�e�N�X�`���ݒ���
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

	//���_�f�[�^
	VertexPosUv vertices[] = {
		//	 x		y	  z		u	  v
		{{	0.0f,100.0f,0.0f},{0.0f,1.0f}},	//����
		{{	0.0f,  0.0f,0.0f},{0.0f,0.0f}},	//����
		{{100.0f,100.0f,0.0f},{1.0f,1.0f}},	//�E��
		{{100.0f,  0.0f,0.0f},{1.0f,0.0f}},	//�E��

	};

	//���_�o�b�t�@����
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&spriteVertBuff)
	);

	//�摜�̑傫������\���T�C�Y��ݒ�
	//�����A����A�E���A�E��
	enum { LB, LT, RB, RT };
	//�e�N�X�`�����擾
	D3D12_RESOURCE_DESC resDesc =
		spriteTexbuff[texNumber]->GetDesc();
	width = (float)resDesc.Width;		//�摜�̉���
	height = (float)resDesc.Height;		//�摜�̏c��

	float left = (0.0f - anchorpoint.x) * width;
	float right = (1.0f - anchorpoint.x) * width;
	float top = (0.0f - anchorpoint.y) * height;
	float bottom = (1.0f - anchorpoint.y) * height;

	if (isFlipX)
	{
		//���E����ւ�
		left = -left;
		right = -right;
	}

	if (isFlipY)
	{
		//�㉺����ւ�
		top = -top;
		bottom = -bottom;
	}

	vertices[LB].pos = { left,	bottom,	0.0f };	//����
	vertices[LT].pos = { left,	top,	0.0f };	//����
	vertices[RB].pos = { right,	bottom,	0.0f };	//�E��
	vertices[RT].pos = { right,	top,	0.0f };	//�E��

	//�e�N�X�`�����擾
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

		vertices[LB].uv = { tex_left,	tex_bottom };	//����
		vertices[LT].uv = { tex_left,	tex_top };	//����
		vertices[RB].uv = { tex_right,	tex_bottom };	//�E��
		vertices[RT].uv = { tex_right,	tex_top };	//�E��
	}


	//�o�b�t�@�ւ̃f�[�^�]��
	VertexPosUv* vertMap = nullptr;
	result = spriteVertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	spriteVertBuff->Unmap(0, nullptr);

	//���_�o�b�t�@�r���[�̍쐬
	spriteVBView.BufferLocation = spriteVertBuff->GetGPUVirtualAddress();
	spriteVBView.SizeInBytes = sizeof(vertices);
	spriteVBView.StrideInBytes = sizeof(vertices[0]);

	//�萔�o�b�t�@�̐���
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,
		IID_PPV_ARGS(&spriteConstBuff)
		);

	//�萔�o�b�t�@�Ƀf�[�^��]��
	ConstBufferData* constMap = nullptr;
	result = spriteConstBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = XMFLOAT4(1, 1, 1, 1);		//�F�w�� (RGBA)
	constMap->mat = XMMatrixOrthographicOffCenterLH(
		0.0f, DX12Init::GetWindowWidth(), DX12Init::GetWindowHeight(), 0.0f, 0.0f, 1.0f);	//���s���e�s��̍���
	spriteConstBuff->Unmap(0, nullptr);


	//�s�񏉊���
	spriteMatProjection = XMMatrixOrthographicOffCenterLH(
		0.0f, DX12Init::GetWindowWidth(), DX12Init::GetWindowHeight(), 0.0f, 0.0f, 1.0f
	);


	return result;
}

void Sprite::SpriteSetPipeline()
{
	//�p�C�v���C���X�e�[�g�̐ݒ�
	DX12Init::GetCmdList()->SetPipelineState(spritePipelineState.Get());
	//���[�g�V�O�l�`���̐ݒ�
	DX12Init::GetCmdList()->SetGraphicsRootSignature(spriteRootSignature.Get());
	//�v���~�e�B�u�`���ݒ�
	DX12Init::GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void Sprite::SpriteDraw()
{
	SpriteSetPipeline();

	if (!isDisplay)
		return;

	//���[���h�s��̍X�V
	spriteMatWorld = XMMatrixIdentity();
	spriteMatWorld *= XMMatrixRotationZ(XMConvertToRadians(spriteRotation));
	spriteMatWorld *= XMMatrixTranslationFromVector(spritePosition);
	//�s��̓]��
	ConstBufferData* constMap = nullptr;
	HRESULT result = spriteConstBuff->Map(0, nullptr, (void**)&constMap);
	constMap->color = spriteColor;
	constMap->mat = spriteMatWorld * spriteMatProjection;		//�s��̍���
	spriteConstBuff->Unmap(0, nullptr);


	//�f�X�N���v�^�q�[�v�̔z��
	ID3D12DescriptorHeap* ppHeaps[] = { spriteDescHeap.Get() };
	DX12Init::GetCmdList()->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//���_�o�b�t�@���Z�b�g
	DX12Init::GetCmdList()->IASetVertexBuffers(0, 1, &spriteVBView);

	//�萔�o�b�t�@���Z�b�g
	DX12Init::GetCmdList()->SetGraphicsRootConstantBufferView(0, spriteConstBuff->GetGPUVirtualAddress());


	//�V�F�[�_���\�[�X�r���[���Z�b�g
	DX12Init::GetCmdList()->SetGraphicsRootDescriptorTable(1,
		CD3DX12_GPU_DESCRIPTOR_HANDLE(
			spriteDescHeap->GetGPUDescriptorHandleForHeapStart(),
			texNumber,
			DX12Init::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		)
	);

	//�`��R�}���h
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

	//���_�f�[�^
	VertexPosUv vertices[] = {
		//	 x		y	  z		u	  v
		{{	0.0f,100.0f,0.0f},{0.0f,1.0f}},	//����
		{{	0.0f,  0.0f,0.0f},{0.0f,0.0f}},	//����
		{{100.0f,100.0f,0.0f},{1.0f,1.0f}},	//�E��
		{{100.0f,  0.0f,0.0f},{1.0f,0.0f}},	//�E��

	};

	//���_�o�b�t�@����
	result = DX12Init::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&spriteVertBuff)
	);

	//�摜�̑傫������\���T�C�Y��ݒ�
	//�����A����A�E���A�E��
	enum { LB, LT, RB, RT };
	float left = (0.0f - anchorpoint.x) * width;
	float right = (1.0f - anchorpoint.x) * width;
	float top = (0.0f - anchorpoint.y) * height;
	float bottom = (1.0f - anchorpoint.y) * height;

	if (isFlipX)
	{
		//���E����ւ�
		left = -left;
		right = -right;
	}

	if (isFlipY)
	{
		//�㉺����ւ�
		top = -top;
		bottom = -bottom;
	}

	vertices[LB].pos = { left,	bottom,	0.0f };	//����
	vertices[LT].pos = { left,	top,	0.0f };	//����
	vertices[RB].pos = { right,	bottom,	0.0f };	//�E��
	vertices[RT].pos = { right,	top,	0.0f };	//�E��

	//�e�N�X�`�����擾
	if (spriteTexbuff[texNumber])
	{
		D3D12_RESOURCE_DESC resDesc =
			spriteTexbuff[texNumber]->GetDesc();

		float tex_left = tex_x / resDesc.Width;
		float tex_right = (tex_x + tex_width) / resDesc.Width;
		float tex_top = tex_y / resDesc.Height;
		float tex_bottom = (tex_y + tex_height) / resDesc.Height;

		vertices[LB].uv = { tex_left,	tex_bottom};	//����
		vertices[LT].uv = { tex_left,	tex_top};	//����
		vertices[RB].uv = { tex_right,	tex_bottom};	//�E��
		vertices[RT].uv = { tex_right,	tex_top};	//�E��
	}


	//�o�b�t�@�ւ̃f�[�^�]��
	VertexPosUv* vertMap = nullptr;
	result = spriteVertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	spriteVertBuff->Unmap(0, nullptr);

}

