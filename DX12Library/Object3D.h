#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include <vector>
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
	struct Vertex
	{
		XMFLOAT3 pos;	 //xyz���W
		XMFLOAT3 normal; //�@���x�N�g��
		XMFLOAT2 uv;	//uv���W
	};

	// �萔�o�b�t�@�p�f�[�^�\����
	struct ConstBufferDataB0 {
		XMMATRIX mat;	//3D�ϊ��s��
	};

	// �萔�o�b�t�@�p�f�[�^�\����B1
	struct ConstBufferDataB1
	{
		XMFLOAT3 ambient;	//�A���r�G���g�W��
		float pad1;			//�p�f�B���O
		XMFLOAT3 diffuse;	//�f�B�t���[�Y�W��
		float pad2;			//�p�f�B���O
		XMFLOAT3 specular;	//�X�y�L�����[�W��
		float alpha;		//�A���t�@
	};

	//�}�e���A��
	struct Material
	{
		std::string name;	//�}�e���A����
		XMFLOAT3 ambient;	//�A���r�G���g�e���x
		XMFLOAT3 diffuse;	//�f�B�t���[�Y�e���x
		XMFLOAT3 specular;	//�X�y�L�����[�e���x
		float alpha;		//�A���t�@
		std::string textureFilename;	//�e�N�X�`���t�@�C����
		//�R���X�g���N�^
		Material() {
			ambient = { 0.3f, 0.3f, 0.3f };
			diffuse = { 0.0f, 0.0f, 0.0f };
			specular = { 0.0f,0.0f,0.0f };
			alpha = 1.0f;
		}
	};


//�萔
private:


//�����o�ϐ�
private:

	//�}�e���A��
	Material material;

	//���_�f�[�^
	vector<Vertex> vertices;

	//�C���f�b�N�X�f�[�^
	vector<unsigned short> indices;

	// �X�P�[�����O�{��
	XMFLOAT3 scale;

	// ��]�p
	XMFLOAT3 rotation;

	// ���W
	XMFLOAT3 position;

	// ���[���h���W
	XMMATRIX matWorld;

#pragma region ���������̕ϐ�
	// ���_�o�b�t�@
	ComPtr < ID3D12Resource> vertBuff = nullptr;

	// ���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	// �C���f�b�N�X�o�b�t�@
	ComPtr <ID3D12Resource> indexBuff = nullptr;

	//�C���f�b�N�X�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW ibView{};


	// �e�N�X�`���o�b�t�@
	ComPtr <ID3D12Resource> texbuff = nullptr;

	// �萔�o�b�t�@
	ComPtr<ID3D12Resource> constBuffB0;
	ComPtr<ID3D12Resource> constBuffB1;

	// �V�F�[�_���\�[�X�r���[�̃n���h��(CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;

	// �V�F�[�_���\�[�X�r���[�̃n���h��(GPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;



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

	// ���[�g�V�O�l�`��
	static ComPtr <ID3D12RootSignature> rootsignature;

	// �p�C�v���C���X�e�[�g
	static ComPtr <ID3D12PipelineState> pipelinestate;

	// �f�X�N���v�^�T�C�Y
	static UINT descriptorHandleIncrementSize;

//�����o�֐�
private:
	/// <summary>
	/// �}�e���A���ǂݍ���
	/// </summary>
	void LoadMaterial(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// �e�N�X�`���ǂݍ���
	/// </summary>
	/// <returns>����</returns>
	bool LoadTexture(const std::string& directoryPath, const std::string& filename);


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
	/// OBJ�t�@�C���ǂݍ��� ��Initialize�̑O�ɋL�q�I
	/// </summary>
	/// <param name="modelname"></param>
	void LoadOBJ(const std::string& modelname);

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
