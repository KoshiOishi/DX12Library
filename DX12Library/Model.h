#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include <vector>
#include <string>
#pragma comment(lib, "d3d12.lib")

class Model
{
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
			ambient = { 0.5f, 0.5f, 0.5f };
			diffuse = { 0.0f, 0.0f, 0.0f };
			specular = { 0.0f,0.0f,0.0f };
			alpha = 1.0f;
		}
	};

private:
	// ���_�o�b�t�@
	ComPtr < ID3D12Resource> vertBuff = nullptr;

	// ���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	// �C���f�b�N�X�o�b�t�@
	ComPtr <ID3D12Resource> indexBuff = nullptr;

	//�C���f�b�N�X�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW ibView{};

	//���_�f�[�^
	vector<Vertex> vertices;

	//�C���f�b�N�X�f�[�^
	vector<unsigned short> indices;

	//�}�e���A��
	Material material;

	// �e�N�X�`���o�b�t�@
	ComPtr <ID3D12Resource> texbuff = nullptr;

	// �V�F�[�_���\�[�X�r���[�̃n���h��(CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;

	// �V�F�[�_���\�[�X�r���[�̃n���h��(GPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;

	//�萔�o�b�t�@
	ComPtr<ID3D12Resource> constBuffB1;

	//OBJ����ǂݍ��܂ꂽ���ۂ�
	bool isLoadFromOBJFile = true;

//�����o�֐�
public:
	void Initialize();
	void Update();
	void Draw();

	/// <summary>
	/// OBJ�t�@�C�����烂�f�����쐬����i�t�H���_���ƒ��g�̖��O�͓���̂��̂ɂ��Ă��������B�j
	/// </summary>
	/// <param name="modelname">obj�t�@�C�����i�[���Ă�t�H���_��</param>
	/// <param name="index">�C���f�b�N�X�i�d�����Ȃ��悤�Ɏw��j</param>
	void LoadOBJ(const std::string & modelname, int index);
	void LoadMaterial(const std::string & directoryPath, const std::string & filename, int index);
	bool LoadTexture(const std::string & directoryPath, const std::string & filename, int index);
	bool LoadTextureReturnTexSize(const std::string & directoryPath, const std::string & filename, int index, float* texWidth = nullptr, float* texHeight = nullptr);
	void CreateBuffer();

	/// <summary>
	/// OBJ�t�@�C��������ꂽ���f���ł��邩�ǂ����Ԃ�
	/// </summary>
	/// <returns></returns>
	bool GetIsOBJ() { return isLoadFromOBJFile; }

	//�ȉ��|���S���쐬
	/// <summary>
	/// �l�p�`�|���S�����쐬�B���S��B
	/// </summary>
	/// <param name="width">����</param>
	/// <param name="height">�c��</param>
	/// <param name="depth">���s</param>
	/// <param name="index">�C���f�b�N�X�i�d�����Ȃ��悤�Ɏw��j</param>
	void CreateBox(float width, float height, float depth, int index);

	/// <summary>
	/// ���|���S�����쐬
	/// </summary>
	/// <param name="vertexX">���̕����� (3�ȏ�)</param>
	/// <param name="vertexY">�����̕����� (3�ȏ�)</param>
	/// <param name="radius">���a</param>
	/// <param name="index">�C���f�b�N�X�i�d�����Ȃ��悤�Ɏw��j</param>
	void CreateSphere(int vertexX, int vertexY, float radius, int index);

	/// <summary>
	/// �~���|���S�����쐬
	/// </summary>
	/// <param name="vertex">��ʂ̒��_��</param>
	/// <param name="radius">���a</param>
	/// <param name="height">����</param>
	/// <param name="index">�C���f�b�N�X�i�d�����Ȃ��悤�Ɏw��j</param>
	void CreatePoll(int vertex, float radius, float height, int index);

	/// <summary>
	/// �l�p�`�|���S�����쐬
	/// </summary>
	/// <param name="width">����</param>
	/// <param name="height">�c��</param>
	/// <param name="index">�C���f�b�N�X�i�d�����Ȃ��悤�Ɏw��j</param>
	void CreateSquare(float width, float height, int index);

	/// <summary>
	/// �e�N�X�`������l�p�`�|���S�����쐬
	/// </summary>
	/// <param name="standardLength">�|���S���̊�̒��� �e�N�X�`���������̏ꍇ�͏c���ɁA�c���̏ꍇ�͉����ɓK������A�����Е��͎����ō��킹��B</param>
	/// <param name="texName">�e�N�X�`���̖��O</param>
	/// <param name="index">�C���f�b�N�X�i�d�����Ȃ��悤�Ɏw��j</param>
	void CreateSquareTex(float standardLength, string texName, int index);

};

