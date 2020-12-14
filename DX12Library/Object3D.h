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

	enum BillboardType {
		NONE,
		Billboard,
		BillboardY,
	};

//�萔
private:


//�����o�ϐ�
private:


	// �X�P�[�����O�{��
	XMFLOAT3 scale{};

	// ��]�p
	XMFLOAT3 rotation{};

	// ���W
	XMFLOAT3 position{};

	// ���[���h���W
	XMMATRIX matWorld{};

	//���f��
	Model model;

	//���C���t���[���\��
	bool isWireFlame = false;

	//�F
	XMFLOAT4 color = { 1,1,1,1 };

	//�r���{�[�h�^�C�v
	BillboardType billboardType = BillboardType::NONE;

	//���C�g���ʂ̗L��
	bool isLight = true;

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

	//�r���{�[�h�s��
	static XMMATRIX matBillboard;

	//Y�����r���{�[�h�s��
	static XMMATRIX matBillboardY;

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

	// OBJ�p�p�C�v���C���X�e�[�g
	static ComPtr <ID3D12PipelineState> pipelinestateOBJ;

	// �����p���[�g�V�O�l�`��
	static ComPtr <ID3D12RootSignature> rootsignatureMath;

	// �����I�u�W�F�N�g�p�p�C�v���C���X�e�[�g
	static ComPtr <ID3D12PipelineState> pipelinestateMath;

	// �e�����I�u�W�F�N�g�p���[�g�V�O�l�`��
	static ComPtr <ID3D12RootSignature> rootsignatureNoShade;

	// �e�����I�u�W�F�N�g�p�p�C�v���C���X�e�[�g
	static ComPtr <ID3D12PipelineState> pipelinestateNoShade;

	// ���C���t���[���p�p�C�v���C���X�e�[�g
	static ComPtr <ID3D12PipelineState> pipelinestateWire;


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
	/// <param name="positionX">X���W</param>
	/// <param name="positionY">Y���W</param>
	/// <param name="positionZ">Z���W</param>
	void AddPosition(const float positionX, const float positionY, const float positionZ)
	{
		this->position.x += positionX; this->position.y += positionY; this->position.z += positionZ;
	}



//�ÓI�����o�֐�
public:
	/// <summary>
	/// ���_���W���w�蕪���Z����
	/// </summary>
	/// <param name="eyeX">X���W</param>
	/// <param name="eyeY">Y���W</param>
	/// <param name="eyeZ">Z���W</param>
	static void AddEye(const float eyeX, const float eyeY, const float eyeZ);

	/// <summary>
	/// �����_���W���w�蕪���Z����
	/// </summary>
	/// <param name="targetX">X���W</param>
	/// <param name="targetY">Y���W</param>
	/// <param name="targetZ">Z���W</param>
	static void AddTarget(const float targetX, const float targetY, const float targetZ);
	
	/// <summary>
	/// ������x�N�g�����w�蕪���Z����
	/// </summary>
	/// <param name="upX">x����</param>
	/// <param name="upY">y����</param>
	/// <param name="upZ">z����</param>
	static void AddUp(const float upX, const float upY, const float upZ);

	/// <summary>
	/// �J�������ړ������鏈��
	/// </summary>
	/// <param name="vector">�ړ��ʃx�N�g��</param>
	static void MoveCamera(const XMFLOAT3& vector);

	/// <summary>
	/// �J�������ړ������鏈��
	/// </summary>
	/// <param name="x">�ړ���x</param>
	/// <param name="y">�ړ���y</param>
	/// <param name="z">�ړ���z</param>
	static void MoveCamera(const float x, const float y, const float z);
		
#pragma endregion

//�ÓI�����o�֐�
public:
	
	/// <summary>
	/// Object3D���̂̏������B�ŏ��ɏ����B
	/// </summary>
	static void FirstInit();

	static void InitPipelineOBJ();

	static void InitPipelineMath();

	static void InitPipelineNoShade();

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
	const bool& GetIsWireFlame() { return isWireFlame; }

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

	/// <summary>
	/// �r���{�[�h�̎�ނ�Ԃ�
	/// </summary>
	/// <returns>�r���{�[�h�̎��</returns>
	const BillboardType GetBillboardType() { return billboardType; }

	/// <summary>
	/// ���C�g�̗L����Ԃ�
	/// </summary>
	/// <returns>���C�g�̗L��</returns>
	const bool GetIsLight() { return isLight; }

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
	/// �X�P�[�����Z�b�g
	/// </summary>
	/// <param name="x">x</param>
	/// <param name="y">y</param>
	/// <param name="z">z</param>
	void SetScale(const float x, const float y, const float z) { this->scale = {x,y,z}; }

	/// <summary>
	/// ��]�p���Z�b�g
	/// </summary>
	/// <param name="rotation">��]�p</param>
	void SetRotation(const XMFLOAT3 rotation) { this->rotation = rotation; }

	/// <summary>
	/// ��]�p���Z�b�g
	/// </summary>
	/// <param name="x">x</param>
	/// <param name="y">y</param>
	/// <param name="z">z</param>
	void SetRotation(const float x, const float y, const float z) { this->rotation = { x,y,z }; }


	/// <summary>
	/// ���W���Z�b�g
	/// </summary>
	/// <param name="position">���W</param>
	void SetPosition(const XMFLOAT3 position) { this->position = position; }

	/// <summary>
	/// ���W���Z�b�g
	/// </summary>
	/// <param name="x">x</param>
	/// <param name="y">y</param>
	/// <param name="z">z</param>
	void SetPosition(const float x, const float y, const float z) { this->position = { x,y,z }; }

	/// <summary>
	/// ���C���t���[���t���O���Z�b�g
	/// </summary>
	void SetIsWireFlame(const bool isWireFlame) { this->isWireFlame = isWireFlame; }

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

	/// <summary>
	/// �r���{�[�h�̎�ނ��Z�b�g���� ������Object3D::BillboardType�œn��
	/// </summary>
	/// <param name="billboardType">�r���{�[�h�̎��</param>
	void SetBillboardType(BillboardType billboardType) { this->billboardType = billboardType; }

	/// <summary>
	/// ���C�g�̗L���t���O���Z�b�g
	/// </summary>
	/// <param name="isLight">���C�g�̗L��</param>
	void SetIsLight(bool isLight) { this->isLight = isLight; }

//�ÓI�����o�֐�
public:
	/// <summary>
	/// ���_���W���Z�b�g
	/// </summary>
	/// <param name="eye">���_���W</param>
	static void SetEye(const XMFLOAT3 eye);

	/// <summary>
	/// ���_���W���Z�b�g
	/// </summary>
	/// <param name="x">x���W</param>
	/// <param name="y">y���W</param>
	/// <param name="z">z���W</param>
	static void SetEye(const float x, const float y, const float z);

	/// <summary>
	/// �����_���W���Z�b�g
	/// </summary>
	/// <param name="target">�����_���W</param>
	static void SetTarget(const XMFLOAT3 target);

	/// <summary>
	/// �����_���W���Z�b�g
	/// </summary>
	/// <param name="x">x���W</param>
	/// <param name="y">y���W</param>
	/// <param name="z">z���W</param>
	static void SetTarget(const float x, const float y, const float z);

	/// <summary>
	/// ������x�N�g���̃Z�b�g
	/// </summary>
	/// <param name="up"></param>
	static void SetUp(const XMFLOAT3 up);

	/// <summary>
	/// ������x�N�g���̃Z�b�g
	/// </summary>
	/// <param name="x">x���W</param>
	/// <param name="y">y���W</param>
	/// <param name="z">z���W</param>
	static void SetUp(const float x, const float y, const float z);

#pragma endregion
};
