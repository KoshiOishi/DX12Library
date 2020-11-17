#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#pragma comment(lib, "d3d12.lib")

class Sprite
{
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

private:

	static ComPtr<ID3D12RootSignature> spriteRootSignature;	//���[�g�V�O�l�`��
	static ComPtr<ID3D12PipelineState> spritePipelineState;	//�p�C�v���C���X�e�[�g
	static XMMATRIX spriteMatProjection;		//�ˉe�s��
	static ComPtr<ID3DBlob> vsBlob; // ���_�V�F�[�_�I�u�W�F�N�g
	static ComPtr<ID3DBlob> psBlob; // �s�N�Z���V�F�[�_�I�u�W�F�N�g
	static ComPtr <ID3DBlob> errorBlob; // �G���[�I�u�W�F�N�g
	static ComPtr <ID3D12DescriptorHeap> spriteDescHeap;
	static const int spriteSRVCount;
	static ComPtr <ID3D12Resource> spriteTexbuff[];					//�e�N�X�`���o�b�t�@

	ComPtr<ID3D12Resource> spriteVertBuff;				//���_�o�b�t�@
	ComPtr<ID3D12Resource> spriteConstBuff;				//�萔�o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW spriteVBView{};			//���_�o�b�t�@�r���[

public:
	struct VertexPosUv
	{
		XMFLOAT3 pos;	//xyz���W
		XMFLOAT2 uv;	//uv���W
	};

	// �萔�o�b�t�@�p�f�[�^�\����
	struct ConstBufferData {
		XMFLOAT4 color; // �F (RGBA)
		XMMATRIX mat;	//3D�ϊ��s��
	};

	float spriteRotation = 0.0f;		//Z�����̉�]�p
	XMVECTOR spritePosition{};			//���W
	XMMATRIX spriteMatWorld{};			//���[���h�s��
	XMFLOAT4 spriteColor = { 1,1,1,1 };	//�F
	UINT texNumber;				//�e�N�X�`���ԍ�
	XMFLOAT2 anchorpoint{};		//�A���J�[�|�C���g
	float width;		//����
	float height;		//�c��
	bool isFlipX;		//���E����ւ�
	bool isFlipY;		//�㉺����ւ�
	float tex_x;		//�摜�؂�o��X���W
	float tex_y;		//�摜�؂�o��y���W
	float tex_width;	//�摜�؂�o������
	float tex_height;	//�摜�؂�o���c��
	bool isDisplay = true;		//�\�����

	/// <summary>
	/// ����������
	/// </summary>
	/// <returns></returns>
	static HRESULT Initialize();

	/// <summary>
	/// �X�v���C�g�̃e�N�X�`���ǂݍ��ݏ���
	/// </summary>
	/// <param name="texnumber">�e�N�X�`���ԍ�</param>
	/// <param name="filename">�t�@�C���p�X</param>
	/// <returns></returns>
	static HRESULT SpriteLoadTexture(UINT texnumber, const wchar_t* filename);

	/// <summary>
	/// �X�v���C�g�̐���
	/// </summary>
	/// <param name="anchorpoint">�A���J�[�|�C���g(�ȗ���)</param>
	/// <param name="isFlipX">����]�t���O(�ȗ���)</param>
	/// <param name="isFlipY">�c��]�t���O(�ȗ���)</param>
	/// <returns></returns>
	HRESULT GenerateSprite(XMFLOAT2 anchorpoint = { 0,0 }, bool isFlipX = false, bool isFlipY = false);

	/// <summary>
	/// �X�v���C�g�p�C�v���C�����Z�b�g �X�v���C�g�`��O�ɕK������
	/// </summary>
	static void SpriteSetPipeline();

	/// <summary>
	/// �`�揈��
	/// </summary>
	void SpriteDraw();

	/// <summary>
	/// �X�v���C�g�̑傫�����Z�b�g
	/// </summary>
	/// <param name="width">����(px)</param>
	/// <param name="height">�c��(px)</param>
	void SpriteSetSize(float width, float height);

	/// <summary>
	/// �X�v���C�g�̃A���J�[�|�C���g���Z�b�g
	/// </summary>
	/// <param name="anchorpoint">�A���J�[�|�C���g(XMFLOAT2)</param>
	void SpriteSetAnchorpoint(XMFLOAT2 anchorpoint);

	/// <summary>
	/// �X�v���C�g�̉����]�t���O���Z�b�g
	/// </summary>
	/// <param name="isFlipX">�����]�t���O</param>
	void SpriteSetIsFlipX(bool isFlipX);

	/// <summary>
	/// �X�v���C�g�̏c���]�t���O���Z�b�g
	/// </summary>
	/// <param name="isFlipX">�c���]�t���O</param>
	void SpriteSetIsFlipY(bool isFlipY);

	/// <summary>
	/// �X�v���C�g�̃e�N�X�`���ԍ����Z�b�g
	/// </summary>
	/// <param name="texnumber">�e�N�X�`���ԍ�</param>
	void SpriteSetTexNumber(UINT texnumber);

	/// <summary>
	/// ���_�o�b�t�@���X�V
	/// </summary>
	void SpriteUpdateVertBuff();

	/// <summary>
	/// �X�v���C�g�̕`��͈�(���N�^���O��)���Z�b�g
	/// </summary>
	/// <param name="tex_x">�؂���x���W</param>
	/// <param name="tex_y">�؂���y���W</param>
	/// <param name="tex_width">�؂��艡��(px)</param>
	/// <param name="tex_height">	�؂���c��(px)</param>
	void SpriteSetDrawRectangle(float tex_x, float tex_y, float tex_width, float tex_height);

};

/*
	ComPtr<ID3D12RootSignature> Sprite::spriteRootSignature = nullptr;	//���[�g�V�O�l�`��
	ComPtr<ID3D12PipelineState> Sprite::spritePipelineState = nullptr;	//�p�C�v���C���X�e�[�g
	ComPtr<ID3D12Resource> Sprite::texbuff = nullptr;					//�e�N�X�`���o�b�t�@
	ComPtr<ID3DBlob> Sprite::vsBlob = nullptr; // ���_�V�F�[�_�I�u�W�F�N�g
	ComPtr<ID3DBlob> Sprite::psBlob = nullptr; // �s�N�Z���V�F�[�_�I�u�W�F�N�g
	ComPtr<ID3DBlob> Sprite::errorBlob = nullptr; // �G���[�I�u�W�F�N�g
	ComPtr<ID3D12DescriptorHeap> Sprite::basicDescHeap = nullptr;
	XMMATRIX Sprite::spriteMatProjection;		//�ˉe�s��

*/

