#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include <vector>
#include "Model.h"
#pragma comment(lib, "d3d12.lib")

class Object3D
{
public:

	using XMMATRIX = DirectX::XMMATRIX;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMVECTOR = DirectX::XMVECTOR;
	using string = std::string;

	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	template <typename T>
	using vector = std::vector<T>;
		
//�T�u�N���X
public:

	// �萔�o�b�t�@�p�f�[�^�\����
	struct ConstBufferDataB0 {
		XMFLOAT4 color;	//�F(RGBA)
		XMMATRIX mat;	//3D�ϊ��s��
	};



//�萔
private:


//�����o�ϐ�
private:


	// �X�P�[�����O�{��
	XMFLOAT3 scale;

	// ��]�p
	XMFLOAT3 rotation;

	// ���W
	XMFLOAT3 position;

	// ���[���h���W
	XMMATRIX matWorld;

	//���f��
	Model model;

	//���C���t���[���\��
	bool isWireFlane;

	//�F
	XMFLOAT4 color = { 1,1,1,1 };

#pragma region ���������̕ϐ�


	// �萔�o�b�t�@
	ComPtr<ID3D12Resource> constBuffB0;




#pragma endregion

//�ÓI�����o�ϐ�
private:

	// �ˉe�ϊ��s��
	static XMMATRIX matProjection;

	// �r���[�ϊ��s��
	static XMMATRIX matView;

	// ���_���W
	static XMFLOAT3 eye;

	// �����_���W
	static XMFLOAT3 target;

	// ������x�N�g��
	static XMFLOAT3 up;


	// �f�X�N���v�^�q�[�v
	static ComPtr <ID3D12DescriptorHeap> basicDescHeap;

	// OBJ�p���[�g�V�O�l�`��
	static ComPtr <ID3D12RootSignature> rootsignatureOBJ;

	// �����p���[�g�V�O�l�`��
	static ComPtr <ID3D12RootSignature> rootsignatureMath;

	// OBJ�p�p�C�v���C���X�e�[�g
	static ComPtr <ID3D12PipelineState> pipelinestateOBJ;

	// OBJ�p���C���t���[���p�C�v���C���X�e�[�g
	static ComPtr <ID3D12PipelineState> pipelinestateOBJ_wire;

	// �����I�u�W�F�N�g�p�p�C�v���C���X�e�[�g
	static ComPtr <ID3D12PipelineState> pipelinestateMath;

	// �����I�u�W�F�N�g�p���C���t���[���p�C�v���C���X�e�[�g
	static ComPtr <ID3D12PipelineState> pipelinestateMath_wire;

//�����o�֐�

public:

	/// <summary>
	/// ���������� ��LoadOBJ�̌�ɋL�q�I
	/// </summary>
	void Initialize();

	/// <summary>
	/// �X�V����
	/// </summary>
	void Update();

	/// <summary>
	/// �`�揈��
	/// </summary>
	void Draw();

	/// <summary>
	/// ���f�����Z�b�g����
	/// </summary>
	/// <param name="model">���f��</param>
	void SetModel(Model model);

#pragma region �֗��֐�
//�����o�֐�
public:
	/// <summary>
	/// �X�P�[�����w�蕪���Z����
	/// </summary>
	/// <param name="scaleX">�X�P�[��X</param>
	/// <param name="scaleY">�X�P�[��Y</param>
	/// <param name="scaleZ">�X�P�[��Z</param>
	void AddScale(const float scaleX, const float scaleY, const float scaleZ)
	{
		this->scale.x += scaleX; this->scale.y += scaleY; this->scale.z += scaleZ;
	}

	/// <summary>
	/// ��]�p���w�蕪���Z����
	/// </summary>
	/// <param name="rotationX">��]�pX</param>
	/// <param name="rotationY">��]�pY</param>
	/// <param name="rotationZ">��]�pZ</param>
	void AddRotation(const float rotationX, const float rotationY, const float rotationZ)
	{
		this->rotation.x += rotationX; this->rotation.y += rotationY; this->rotation.z += rotationZ;
	}

	/// <summary>
	/// ���W���w�蕪���Z����
	/// </summary>
	/// <param name="positionX">���WX</param>
	/// <param name="positionY">���WY</param>
	/// <param name="positionZ">���WZ</param>
	void AddPosition(const float positionX, const float positionY, const float positionZ)
	{
		this->position.x += positionX; this->position.y += positionY; this->position.z += positionZ;
	}



//�ÓI�����o�֐�
public:
	static void AddEye(const float eyeX, const float eyeY, const float eyeZ);
	static void AddTarget(const float targetX, const float targetY, const float targetZ);
	static void AddUp(const float upX, const float upY, const float upZ);

	
#pragma endregion

//�ÓI�����o�֐�
public:
	
	/// <summary>
	/// Object3D���̂̏������B�ŏ��ɏ����B
	/// </summary>
	static void FirstInit();

	static void InitPipelineOBJ();

	static void InitPipelineMath();

#pragma region Getter
//�����o�֐�
public:
	/// <summary>
	/// �X�P�[���̎擾
	/// </summary>
	/// <returns>���W</returns>
	const XMFLOAT3& GetScale() { return scale; }

	/// <summary>
	/// ��]�p�̎擾
	/// </summary>
	/// <returns>���W</returns>
	const XMFLOAT3& GetRotation() { return rotation; }


	/// <summary>
	/// ���W�̎擾
	/// </summary>
	/// <returns>���W</returns>
	const XMFLOAT3& GetPosition() { return position; }

	/// <summary>
	/// ���C���t���[���`��t���O���擾
	/// </summary>
	/// <returns>���C���t���[���t���O</returns>
	const bool& GetIsWireFlame() { return isWireFlane; }

	/// <summary>
	/// �F(RGBA)���擾����
	/// </summary>
	/// <returns>�F(RGBA)</returns>
	const XMFLOAT4& GetColor() { return color; }

	/// <summary>
	/// �F(RGBA)��0�`255�̐��l�Ŏ擾����
	/// </summary>
	/// <returns>�F(RGBA)�A0�`255�͈̔�</returns>
	const XMFLOAT4& GetColorAs0To255(){
		XMFLOAT4 c = { color.x * 255,color.y * 255 ,color.z * 255 ,color.w * 255 };
		return c;
	}

//�ÓI�����o�֐�
public:
	/// <summary>
	/// ���_���W�̎擾
	/// </summary>
	/// <returns>���W</returns>
	static const XMFLOAT3& GetEye() { return eye; }

	/// <summary>
	/// �����_���W�̎擾
	/// </summary>
	/// <returns>���W</returns>
	static const XMFLOAT3& GetTarget() { return target; }

	/// <summary>
	/// ������x�N�g���̎擾
	/// </summary>
	/// <returns>������x�N�g��</returns>
	static const XMFLOAT3& GetUp() { return up; }

	/// <summary>
	/// �r���[�s��̍X�V����
	/// </summary>
	static void UpdateViewMatrix();

	/// <summary>
	/// �f�X�N���v�^�q�[�v�̃|�C���^���擾
	/// </summary>
	/// <returns>�f�X�N���v�^�q�[�v�̃|�C���^</returns>
	static ID3D12DescriptorHeap* GetDescHeap();

#pragma endregion

#pragma region Setter
//�����o�֐�
public:
	/// <summary>
	/// �X�P�[�����Z�b�g
	/// </summary>
	/// <param name="scale">�X�P�[��</param>
	void SetScale(const XMFLOAT3 scale) { this->scale = scale; }

	/// <summary>
	/// ��]�p���Z�b�g
	/// </summary>
	/// <param name="rotation">��]�p</param>
	void SetRotation(const XMFLOAT3 rotation) { this->rotation = rotation; }

	/// <summary>
	/// ���W���Z�b�g
	/// </summary>
	/// <param name="position">���W</param>
	void SetPosition(const XMFLOAT3 position) { this->position = position; }

	/// <summary>
	/// ���C���t���[���t���O���Z�b�g
	/// </summary>
	void SetIsWireFlame(const bool isWireFlame) { this->isWireFlane = isWireFlame; }

	/// <summary>
	/// �F(RGBA)���Z�b�g
	/// </summary>
	/// <param name="R">��</param>
	/// <param name="G">��</param>
	/// <param name="B">��</param>
	/// <param name="A">�A���t�@�l</param>
	void SetColor(float R, float G, float B, float A) {
		XMFLOAT4 c = { R,G,B,A };
		SetColor(c);
	}

	/// <summary>
	/// �F(RGBA)���Z�b�g
	/// </summary>
	/// <param name="color">�F(RGBA)</param>
	void SetColor(XMFLOAT4 color) { this->color = color; }

	/// <summary>
	/// �F(RGBA)��0�`255�͈̔͂ŃZ�b�g
	/// </summary>
	/// <param name="R">��</param>
	/// <param name="G">��</param>
	/// <param name="B">��</param>
	/// <param name="A">�A���t�@�l</param>
	void SetColorAs0To255(float R, float G, float B, float A)
	{
		R = R / 255;
		G = G / 255;
		B = B / 255;
		A = A / 255;
		XMFLOAT4 c = { R,G,B,A };
		SetColor(c);
	}

	/// <summary>
	/// �F(RGBA)��0�`255�͈̔͂ŃZ�b�g
	/// </summary>
	/// <param name="color">�F(RGBA)</param>
	void SetColorAs0To255(XMFLOAT4 color)
	{
		XMFLOAT4 c = { color.x / 255, color.y / 255, color.z / 255, color.w / 255 };
		SetColor(c);
	}
//�ÓI�����o�֐�
public:
	/// <summary>
	/// ���_���W���Z�b�g
	/// </summary>
	/// <param name="eye">���_���W</param>
	static void SetEye(const XMFLOAT3 eye);

	/// <summary>
	/// �����_���W���Z�b�g
	/// </summary>
	/// <param name="target">�����_���W</param>
	static void SetTarSet(const XMFLOAT3 target);

	/// <summary>
	/// ������x�N�g���̃Z�b�g
	/// </summary>
	/// <param name="up"></param>
	static void SetUp(const XMFLOAT3 up);

#pragma endregion
};
