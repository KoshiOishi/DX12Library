#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#pragma comment(lib, "d3d12.lib")

class Sprite
{
public:
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

//�T�u�N���X
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

//�萔
private:
	static const int spriteSRVCount;

//�ÓI�����o�ϐ�
private:

	static ComPtr<ID3D12RootSignature> spriteRootSignature;	//���[�g�V�O�l�`��
	static ComPtr<ID3D12PipelineState> spritePipelineState;	//�p�C�v���C���X�e�[�g
	static XMMATRIX spriteMatProjection;		//�ˉe�s��
	static ComPtr<ID3DBlob> vsBlob; // ���_�V�F�[�_�I�u�W�F�N�g
	static ComPtr<ID3DBlob> psBlob; // �s�N�Z���V�F�[�_�I�u�W�F�N�g
	static ComPtr <ID3DBlob> errorBlob; // �G���[�I�u�W�F�N�g
	static ComPtr <ID3D12DescriptorHeap> spriteDescHeap;
	static ComPtr <ID3D12Resource> spriteTexbuff[];					//�e�N�X�`���o�b�t�@

	ComPtr<ID3D12Resource> spriteVertBuff;				//���_�o�b�t�@
	ComPtr<ID3D12Resource> spriteConstBuff;				//�萔�o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW spriteVBView{};			//���_�o�b�t�@�r���[

//�ÓI�����o�֐�
public:

	/// <summary>
	/// ����������
	/// </summary>
	static void FirstInit();


	/// <summary>
	/// �X�v���C�g�̃e�N�X�`���ǂݍ��ݏ���
	/// </summary>
	/// <param name="texnumber">�e�N�X�`���ԍ�</param>
	/// <param name="filename">�t�@�C���p�X</param>
	static void LoadTexture(UINT texnumber, const wchar_t* filename);

	/// <summary>
	/// �X�v���C�g�p�C�v���C�����Z�b�g �X�v���C�g�`��O�ɕK������
	/// </summary>
	static void SpriteSetPipeline();

//�����o�֐�
public:

	/// <summary>
	/// �������������܂Ƃ߂����́B
	/// </summary>
	/// <param name="texnumber">�e�N�X�`���ԍ�</param>
	/// <param name="filename">�e�N�X�`���t�@�C���p�X</param>
	/// <param name="anchorpoint">�A���J�[�|�C���g(�ȗ���)</param>
	/// <param name="isFlipX">�����]�t���O(�ȗ���)</param>
	/// <param name="isFlipY">�c���]�t���O(�ȗ���)</param>
	void Initialize(UINT texnumber, const wchar_t* filename, XMFLOAT2 anchorpoint = { 0,0 }, bool isFlipX = false, bool isFlipY = false);

	/// <summary>
	/// �X�v���C�g�̐���
	/// </summary>
	/// <param name="anchorpoint">�A���J�[�|�C���g(�ȗ���)</param>
	/// <param name="isFlipX">����]�t���O(�ȗ���)</param>
	/// <param name="isFlipY">�c��]�t���O(�ȗ���)</param>
	void GenerateSprite(XMFLOAT2 anchorpoint = { 0,0 }, bool isFlipX = false, bool isFlipY = false);


	/// <summary>
	/// �`�揈��
	/// </summary>
	void Draw();

	/// <summary>
	/// ���_�o�b�t�@���X�V
	/// </summary>
	void UpdateVertBuff();


//�����o�ϐ�
private:
	//1126 �Q�b�^�[�Z�b�^�[�����܂��傤
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


public:
#pragma region �Q�b�^�[
	/// <summary>
	/// ���W���擾����
	/// </summary>
	/// <returns>���W</returns>
	XMFLOAT2 GetPosition()
	{
		return { DirectX::XMVectorGetX(spritePosition), DirectX::XMVectorGetY(spritePosition) };
	}

	/// <summary>
	/// �F�����擾����
	/// </summary>
	/// <returns>�F���</returns>
	XMFLOAT4 GetColor() { return spriteColor; }

	/// <summary>
	/// Z������̉�]�p���擾����
	/// </summary>
	/// <returns>Z������̉�]�p</returns>
	float GetRotation() { return spriteRotation; }

	/// <summary>
	/// �X�P�[�����擾����
	/// </summary>
	/// <returns>�X�P�[��</returns>
	XMFLOAT2 GetScale() { return { width,height }; }

	/// <summary>
	/// �����]�t���O���擾����
	/// </summary>
	/// <returns>�����]�t���O</returns>
	bool GetIsFlipX() { return isFlipX; }

	/// <summary>
	/// �c���]�t���O���擾����
	/// </summary>
	/// <returns>�c���]�t���O</returns>
	bool GetIsFlipY() { return isFlipY; }

	/// <summary>
	/// �\���t���O���擾����
	/// </summary>
	/// <returns>�\���t���O</returns>
	bool GetIsDisplay() { return isDisplay; }

#pragma endregion

#pragma region �Z�b�^�[
	/// <summary>
	/// ���W���Z�b�g����
	/// </summary>
	/// <param name="pos">���W</param>
	void SetPosition(XMFLOAT2 pos);

	/// <summary>
	/// �F�����Z�b�g����
	/// </summary>
	/// <param name="color">�F���</param>
	void SetColor(XMFLOAT4 color);

	/// <summary>
	/// Z������̉�]�p���Z�b�g����
	/// </summary>
	/// <param name="rotation">Z������̉�]�p</param>
	void SetRotation(float rotation);

	/// <summary>
	/// �X�P�[�����Z�b�g����
	/// </summary>
	/// <param name="scale">�X�P�[��</param>
	void SetScale(XMFLOAT2 scale);

	/// <summary>
	/// �����]�t���O���Z�b�g����
	/// </summary>
	/// <param name="isFlipX">�����]�t���O</param>
	void SetIsFlipX(bool isFlipX);

	/// <summary>
	/// �c���]�t���O���Z�b�g����
	/// </summary>
	/// <param name="isFlipY">�c���]�t���O</param>
	void SetIsFlipY(bool isFlipY);

	/// <summary>
	/// �\���t���O���Z�b�g����
	/// </summary>
	/// <param name="isDisplay">�\���t���O</param>
	void SetIsDisplay(bool isDisplay);

	/// <summary>
	/// �e�N�X�`���ԍ����Z�b�g
	/// </summary>
	/// <param name="texnumber">�e�N�X�`���ԍ�</param>
	void SetTexNumber(UINT texnumber);

	/// <summary>
	/// �`��͈�(���N�^���O��)���Z�b�g
	/// </summary>
	/// <param name="tex_x">�؂���x���W</param>
	/// <param name="tex_y">�؂���y���W</param>
	/// <param name="tex_width">�؂��艡��(px)</param>
	/// <param name="tex_height">	�؂���c��(px)</param>
	void SetDrawRectangle(float tex_x, float tex_y, float tex_width, float tex_height);

	/// <summary>
	/// �A���J�[�|�C���g���Z�b�g����
	/// </summary>
	/// <param name="anchorpoint">�A���J�[�|�C���g</param>
	void SetAnchorpoint(XMFLOAT2 anchorpoint);
#pragma endregion
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

