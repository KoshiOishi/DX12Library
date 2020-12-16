#include "Model.h"
#include <fstream>
#include <sstream>
#include "DX12Util.h"
#include <DirectXTex.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include "Object3D.h"

using namespace DirectX;

void Model::Initialize()
{
	HRESULT result;

	//�萔�o�b�t�@�̍쐬
	result = DX12Util::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// �A�b�v���[�h�\
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB1) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffB1));
}

void Model::Update()
{
	HRESULT result;

	// �萔�o�b�t�@�փf�[�^�]��
	ConstBufferDataB1* constMap1 = nullptr;
	result = constBuffB1->Map(0, nullptr, (void**)&constMap1);
	constMap1->ambient = material.ambient;
	constMap1->diffuse = material.diffuse;
	constMap1->specular = material.specular;
	constMap1->alpha = material.alpha;
	constBuffB1->Unmap(0, nullptr);
}

void Model::Draw()
{
	//�C���f�b�N�X�o�b�t�@�̃Z�b�g�R�}���h
	DX12Util::GetCmdList()->IASetIndexBuffer(&ibView);

	//�v���~�e�B�u�`��̐ݒ�R�}���h(�O�p�`���X�g)
	DX12Util::GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//���_�o�b�t�@�̐ݒ�R�}���h
	DX12Util::GetCmdList()->IASetVertexBuffers(0, 1, &vbView);

	DX12Util::GetCmdList()->SetGraphicsRootConstantBufferView(1, constBuffB1->GetGPUVirtualAddress());
	
	// �V�F�[�_���\�[�X�r���[���Z�b�g
	DX12Util::GetCmdList()->SetGraphicsRootDescriptorTable(2, gpuDescHandleSRV);

	//�`��R�}���h
	DX12Util::GetCmdList()->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);
}

void Model::CreateFromOBJ(const std::string & modelname, int index, bool smoothing)
{
	HRESULT result;

	//�t�@�C���X�g���[��
	std::fstream file;
	//.obj�t�@�C�����J��
	const string filename = modelname + ".obj";	//"triangle_mat.obj"
	const string directoryPath = "Resources/" + modelname + "/";	//"Resources/triangle_mat/"
	file.open(directoryPath + filename);	//"Resources/triangle_mat/triangle_mat.obj"
	//�t�@�C���I�[�v�����s���`�F�b�N
	if (file.fail()) {
		assert(0);
	}

	vector<XMFLOAT3> positions;	//���_���W
	vector<XMFLOAT3> normals;	//�@���x�N�g��
	vector<XMFLOAT2> texcoords;	//�e�N�X�`��UV
	//1�s���ǂݍ���
	string line;
	while (getline(file, line)) {

		//1�s���̕�������X�g���[���ɕϊ����ĉ�͂��₷������
		std::istringstream line_stream(line);

		//���p�X�y�[�X��؂�ōs�̐擪��������擾
		string key;
		getline(line_stream, key, ' ');

		//�擪������mtllib�Ȃ�}�e���A��
		if (key == "mtllib")
		{
			//�}�e���A���̃t�@�C�����ǂݍ���
			string filename;
			line_stream >> filename;
			//�}�e���A���ǂݍ���
			LoadMaterial(directoryPath, filename, index);
		}

		//�擪������v�Ȃ璸�_���W
		if (key == "v") {
			//X,Y,Z���W�ǂݍ���
			XMFLOAT3 position{};
			line_stream >> position.x;
			line_stream >> position.y;
			line_stream >> position.z;
			//���W�f�[�^�ɒǉ�
			positions.emplace_back(position);
			////���_�f�[�^�ɒǉ�
			//VertexPosNormalUv vertex{};
			//vertex.pos = position;
			//vertices.emplace_back(vertex);
		}

		//�擪������vt�Ȃ�e�N�X�`��
		if (key == "vt") {
			//U,V�����ǂݍ���
			XMFLOAT2 texcoord{};
			line_stream >> texcoord.x;
			line_stream >> texcoord.y;
			//V�������]
			texcoord.y = 1.0f - texcoord.y;
			//�e�N�X�`�����W�f�[�^�ɒǉ�
			texcoords.emplace_back(texcoord);
		}

		//�擪������vn�Ȃ�@���x�N�g��
		if (key == "vn") {
			//X,Y,Z�����ǂݍ���
			XMFLOAT3 normal{};
			line_stream >> normal.x;
			line_stream >> normal.y;
			line_stream >> normal.z;
			//�@���x�N�g���f�[�^�ɒǉ�
			normals.emplace_back(normal);
		}

		//�擪������f�Ȃ�|���S��(�O�p�`)
		if (key == "f")
		{
			//���p�X�y�[�X��؂�ōs�̑�����ǂݍ���
			string index_string;
			while (getline(line_stream, index_string, ' ')) {

				//���_�C���f�b�N�X1���̕�������X�g���[���ɕϊ����ĉ�͂��₷������
				std::istringstream index_stream(index_string);
				unsigned short indexPosition, indexNormal, indexTexcoord;
				index_stream >> indexPosition;
				index_stream.seekg(1, std::ios_base::cur);	//�X���b�V�����΂�
				index_stream >> indexTexcoord;
				index_stream.seekg(1, std::ios_base::cur);	//�X���b�V�����΂�
				index_stream >> indexNormal;

				//���_�f�[�^�̒ǉ�
				Vertex vertex{};
				vertex.pos = positions[indexPosition - 1];
				vertex.normal = normals[indexNormal - 1];
				vertex.uv = texcoords[indexTexcoord - 1];
				vertices.emplace_back(vertex);
				//�C���f�b�N�X�f�[�^�̒ǉ�
				indices.emplace_back(indices.size());
				//�G�b�W�������p�̃f�[�^��ǉ�
				if (smoothing) {
					//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
					AddSmoothData(indexPosition, (unsigned short)vertices.size() - 1);
				}
			}
		}
	}

	file.close();

	if (smoothing) {
		CalculateSmoothedVertexNormals();
	}

	CreateBuffer();
}

void Model::LoadMaterial(const std::string & directoryPath, const std::string & filename, int index)
{
	bool isLoadTexture = false;

	//�t�@�C���X�g���[��
	std::ifstream file;
	//�}�e���A���t�@�C�����J��
	file.open(directoryPath + filename);
	//�t�@�C���I�[�v�����s���`�F�b�N
	if (file.fail()) {
		assert(0);
	}

	//1�s���ǂݍ���
	string line;
	while (getline(file, line)) {

		//1�s���̕�������X�g���[���ɕϊ�
		std::istringstream line_stream(line);
		//���p�X�y�[�X��؂�ōs�̐擪��������擾
		string key;
		getline(line_stream, key, ' ');

		//�擪�̃^�u�����͖�������
		if (key[0] == '\t') {
			key.erase(key.begin());//�擪���������폜
		}

		//�擪������newmtl�Ȃ�}�e���A����
		if (key == "newmtl") {
			//�}�e���A�����ǂݍ���
			line_stream >> material.name;
		}

		//�擪������Ka�Ȃ�A���r�G���g�F
		if (key == "Ka") {
			line_stream >> material.ambient.x;
			line_stream >> material.ambient.y;
			line_stream >> material.ambient.z;
		}

		//�擪������Kd�Ȃ�f�B�t���[�Y�F
		if (key == "Kd") {
			line_stream >> material.diffuse.x;
			line_stream >> material.diffuse.y;
			line_stream >> material.diffuse.z;
		}

		//�擪������Ks�Ȃ�X�y�L�����[�F
		if (key == "Ks") {
			line_stream >> material.specular.x;
			line_stream >> material.specular.y;
			line_stream >> material.specular.z;
		}

		//�擪������map_Kd�Ȃ�e�N�X�`���t�@�C����
		if (key == "map_Kd") {
			//�e�N�X�`���̃t�@�C�����ǂݍ���
			line_stream >> material.textureFilename;
			//�e�N�X�`���ǂݍ���
			LoadTexture(directoryPath, material.textureFilename, index);
			isLoadTexture = true;
		}
	}

	//�t�@�C�������
	file.close();

	if (!isLoadTexture)
	{
		//�e�N�X�`�����Ȃ��Ƃ��͔��e�N�X�`����ǂݍ���
		LoadTexture("Resources/", "white1x1.png", index);
	}
}

bool Model::LoadTexture(const std::string & directoryPath, const std::string & filename, int index)
{
	HRESULT result = S_FALSE;

	// WIC�e�N�X�`���̃��[�h
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	//�t�@�C���p�X������
	string filepath = directoryPath + filename;

	//���j�R�[�h������ɕϊ�����
	wchar_t wfilepath[128];
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, filepath.c_str(), -1, wfilepath, _countof(wfilepath));

	result = LoadFromWICFile(
		wfilepath, WIC_FLAGS_NONE,
		&metadata, scratchImg);
	if (FAILED(result)) {
		return result;
	}

	const Image* img = scratchImg.GetImage(0, 0, 0); // ���f�[�^���o

	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	// �e�N�X�`���p�o�b�t�@�̐���
	result = DX12Util::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // �e�N�X�`���p�w��
		nullptr,
		IID_PPV_ARGS(&texbuff));
	if (FAILED(result)) {
		return result;
	}

	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	result = texbuff->WriteToSubresource(
		0,
		nullptr, // �S�̈�փR�s�[
		img->pixels,    // ���f�[�^�A�h���X
		(UINT)img->rowPitch,  // 1���C���T�C�Y
		(UINT)img->slicePitch // 1���T�C�Y
	);
	if (FAILED(result)) {
		return result;
	}

	// �f�X�N���v�^�T�C�Y���擾
	UINT descriptorHandleIncrementSize = DX12Util::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// �V�F�[�_���\�[�X�r���[�쐬
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(Object3D::GetDescHeap()->GetCPUDescriptorHandleForHeapStart(), index, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(Object3D::GetDescHeap()->GetGPUDescriptorHandleForHeapStart(), index, descriptorHandleIncrementSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // �ݒ�\����
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;

	DX12Util::GetDevice()->CreateShaderResourceView(texbuff.Get(), //�r���[�Ɗ֘A�t����o�b�t�@
		&srvDesc, //�e�N�X�`���ݒ���
		cpuDescHandleSRV
	);

	return true;

}

bool Model::LoadTextureReturnTexSize(const std::string & directoryPath, const std::string & filename, int index, float* texWidth, float* texHeight)
{
	HRESULT result = S_FALSE;

	// WIC�e�N�X�`���̃��[�h
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	//�t�@�C���p�X������
	string filepath = directoryPath + filename;

	//���j�R�[�h������ɕϊ�����
	wchar_t wfilepath[128];
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, filepath.c_str(), -1, wfilepath, _countof(wfilepath));

	result = LoadFromWICFile(
		wfilepath, WIC_FLAGS_NONE,
		&metadata, scratchImg);
	if (FAILED(result)) {
		return false;
	}

	const Image* img = scratchImg.GetImage(0, 0, 0); // ���f�[�^���o

	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	// �e�N�X�`���p�o�b�t�@�̐���
	result = DX12Util::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // �e�N�X�`���p�w��
		nullptr,
		IID_PPV_ARGS(&texbuff));
	if (FAILED(result)) {
		return false;
	}

	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	result = texbuff->WriteToSubresource(
		0,
		nullptr, // �S�̈�փR�s�[
		img->pixels,    // ���f�[�^�A�h���X
		(UINT)img->rowPitch,  // 1���C���T�C�Y
		(UINT)img->slicePitch // 1���T�C�Y
	);
	if (FAILED(result)) {
		return false;
	}

	// �f�X�N���v�^�T�C�Y���擾
	UINT descriptorHandleIncrementSize = DX12Util::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// �V�F�[�_���\�[�X�r���[�쐬
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(Object3D::GetDescHeap()->GetCPUDescriptorHandleForHeapStart(), index, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(Object3D::GetDescHeap()->GetGPUDescriptorHandleForHeapStart(), index, descriptorHandleIncrementSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // �ݒ�\����
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;

	DX12Util::GetDevice()->CreateShaderResourceView(texbuff.Get(), //�r���[�Ɗ֘A�t����o�b�t�@
		&srvDesc, //�e�N�X�`���ݒ���
		cpuDescHandleSRV
	);

	if (texWidth != nullptr)
		*texWidth = metadata.width;
	if (texHeight != nullptr)
		*texHeight = metadata.height;
	return false;
}

void Model::CreateBuffer()
{
	HRESULT result;

	UINT sizeVB = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());


	//���_�o�b�t�@�̊m��
	//GPU ���̃��������m�ۂ��āA���_�f�[�^�p�̗̈��ݒ�B
	//���_�o�b�t�@�p GPU ���\�[�X�̐���

		// ���_�o�b�t�@�̐���
	result = DX12Util::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB), // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//�C���f�b�N�X�o�b�t�@�̐���
	result = DX12Util::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB), // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));


	//���_�o�b�t�@�ւ̃f�[�^�]��
	// GPU ��̃o�b�t�@�ɑΉ��������z���������擾
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	// �S���_�ɑ΂���
	for (int i = 0; i < vertices.size(); i++)
	{
		vertMap[i] = vertices[i]; // ���W���R�s�[
	}
	// �}�b�v������
	vertBuff->Unmap(0, nullptr);

	// ���_�o�b�t�@�r���[�̍쐬
	vbView.BufferLocation = vertBuff.Get()->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	vbView.StrideInBytes = sizeof(vertices[0]);



	//�C���f�b�N�X�o�b�t�@�ւ̃f�[�^�]��
	// GPU ��̃o�b�t�@�ɑΉ��������z���������擾
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	// �S�C���f�b�N�X�ɑ΂���
	for (int i = 0; i < indices.size(); i++)
	{
		indexMap[i] = indices[i]; // �C���f�b�N�X���R�s�[
	}
	indexBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[�̍쐬�ƃZ�b�g
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices[0]) * indices.size();
}

void Model::AddSmoothData(unsigned short indexPosition, unsigned short indexVertex)
{
	smoothData[indexPosition].emplace_back(indexVertex);
}

void Model::CalculateSmoothedVertexNormals()
{
	auto itr = smoothData.begin();
	for (; itr != smoothData.end(); ++itr)
	{
		//�e�ʗp�̋��ʒ��_�R���N�V����
		std::vector<unsigned short>& v = itr->second;
		//�S���_�̖@���𕽋ς���
		XMVECTOR normal = {};
		for (unsigned short index : v) {
			normal += XMVectorSet(vertices[index].normal.x, vertices[index].normal.y, vertices[index].normal.z, 0);
		}
		normal = XMVector3Normalize(normal / (float)v.size());
		//���ʍ��W���g�p����S�Ă̒��_�f�[�^�ɏ�������
		for (unsigned short index : v) {
			vertices[index].normal = { normal.m128_f32[0],normal.m128_f32[1], normal.m128_f32[2] };
		}
	}

}

void Model::CreateBox(float width, float height, float depth, int index, bool smoothing)
{
	isLoadFromOBJFile = false;

	material.ambient = { 1,1,1 };
	material.diffuse = { 0.5f,0.5f,0.5f };

	material.name = "default_box";

	material.textureFilename = "white1x1.png";
	LoadTexture("Resources/" , material.textureFilename, index);
	
	float x = width / 2;
	float y = height / 2;
	float z = depth / 2;

	//���_�v�Z
	Vertex vertex{};
	XMFLOAT3 samplePos[8];
	XMFLOAT2 sampleUV[4];
	//��O
	samplePos[0] = { -x, -y, -z };//����
	samplePos[1] = { -x,  y, -z };//����
	samplePos[2] = {  x, -y, -z };//�E��
	samplePos[3] = {  x,  y, -z };//�E��
	//��
	samplePos[4] = { -x, -y,  z };//����
	samplePos[5] = { -x,  y,  z };//����
	samplePos[6] = {  x, -y,  z };//�E��
	samplePos[7] = {  x,  y,  z };//�E��

	sampleUV[0] = { 0,1 };//����
	sampleUV[1] = { 0,0 };//����
	sampleUV[2] = { 1,1 };//�E��
	sampleUV[3] = { 1,0 };//�E��

	int nUV[6]{ 0,1,2,2,1,3 };

	//��1 0123
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 0,1,2,3 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}
	//��2 2367
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 2,3,6,7 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}
	//��3 6745
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 6,7,4,5 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}
	//��4 4501
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 4,5,0,1 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}
	//��5 1537
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 1,5,3,7 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}
	//��6 4062
	for (int i = 0; i < 6; i++)
	{
		int t[4]{ 4,0,6,2 };
		int n[6]{ t[0],t[1],t[2],t[2],t[1],t[3] };
		vertex.pos = samplePos[n[i]];
		vertex.uv = sampleUV[nUV[i]];
		vertices.emplace_back(vertex);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[i], (unsigned short)vertices.size() - 1);
		}
	}

	//�C���f�b�N�X�z��
	for (int i = 0; i < 36; i++)
	{
		indices.emplace_back(i);
	}

	
	//�@���̌v�Z
	for (int i = 0; i < vertices.size() / 3; i++)
	{
		//�O�p�`1���ƂɌv�Z���Ă���
		//�O�p�`�̃C���f�b�N�X�����o���āA�ꎞ�I�ȕϐ��ɓ����
		unsigned short index0 = indices[i * 3 + 0];
		unsigned short index1 = indices[i * 3 + 1];
		unsigned short index2 = indices[i * 3 + 2];
		//�O�p�`���\�����钸�_���W���x�N�g���ɑ��
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//p0��p1�x�N�g���Ap0��p2�x�N�g�����v�Z�i�x�N�g���̌��Z�j
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//�O�ς͗������琂���ȃx�N�g��
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//���K���i������1�ɂ���j
		normal = XMVector3Normalize(normal);
		//���߂��@���𒸓_�f�[�^�ɑ��
		XMStoreFloat3(&vertices[index0].normal, normal);
		XMStoreFloat3(&vertices[index1].normal, normal);
		XMStoreFloat3(&vertices[index2].normal, normal);

	}

	if (smoothing) {
		CalculateSmoothedVertexNormals();
	}
	
	CreateBuffer();
}

void Model::CreateSphere(int vertexX, int vertexY, float radius, int index, bool smoothing)
{
	isLoadFromOBJFile = false;

	material.ambient = { 0.5f,0.5f,0.5f };
	material.diffuse = { 0.5f,0.5f,0.5f };

	material.name = "default_sphere";

	material.textureFilename = "white1x1.png";
	LoadTexture("Resources/", material.textureFilename, index);


	if (vertexX < 3 || vertexY < 3) {
		assert(0);
		return;
	}

	Vertex upper, downer;
	//upper...0 downer...1 side...side + 2
	upper.pos = { 0,radius,0 };
	downer.pos = { 0,-radius,0 };

	vector<vector<Vertex>> side;

	for (int i = 1; i < vertexY; i++)
	{
		float fY = 180.0f / vertexY;
		float radY = XMConvertToRadians(fY * i);
		vector<Vertex> v;
		side.emplace_back(v);
		for (int j = 0; j < vertexX; j++)
		{
			Vertex vert;
			float fX = 360.0f / vertexX;
			float radXZ = XMConvertToRadians(fX * j);
			vert.pos = { std::cos(radXZ) * std::sin(radY) * radius, std::cos(radY) * radius, std::sin(radXZ) * std::sin(radY) * radius };
			side[i - 1].emplace_back(vert);
		}
	}

	//��
	for (int i = 0; i < vertexX; i++)
	{
		Vertex v[3];
		unsigned short indice[3];
		if (i < vertexX - 1)
		{
			v[0] = upper;
			v[1] = side[0][i + 1];
			v[2] = side[0][i];
			indice[0] = 0;
			indice[1] = i + 1 + 2;
			indice[2] = i + 2;
		}
		else
		{
			v[0] = upper;
			v[1] = side[0][0];
			v[2] = side[0][i];
			indice[0] = 0;
			indice[1] = 2;
			indice[2] = i + 2;
		}
		for (int j = 0; j < 3; j++) 
		{
			vertices.emplace_back(v[j]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(indice[j], (unsigned short)vertices.size() - 1);
			}
		}
	}

	//��
	for (int i = 0; i < vertexX; i++)
	{
		Vertex v[3];
		unsigned short indice[3];
		if (i < vertexX - 1)
		{
			v[0] = downer;
			v[1] = side[vertexY - 2][i];
			v[2] = side[vertexY - 2][i + 1];
			indice[0] = 1;
			indice[1] = (vertexY - 2) * vertexX + i + 2;
			indice[2] = (vertexY - 2) * vertexX + i + 1 + 2;
		}
		else
		{
			v[0] = downer;
			v[1] = side[vertexY - 2][i];
			v[2] = side[vertexY - 2][0];
			indice[0] = 1;
			indice[1] = (vertexY - 2) * vertexX + i + 2;
			indice[2] = (vertexY - 2) * vertexX + 2;
		}
		for (int j = 0; j < 3; j++)
		{
			vertices.emplace_back(v[j]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(indice[j], (unsigned short)vertices.size() - 1);
			}
		}
	}

	//��
	for (int i = 0; i < vertexY - 2; i++)
	{
		for (int j = 0; j < vertexX; j++)
		{
			Vertex v[6];
			unsigned short indice[6];
			if (j < vertexX - 1)
			{
				v[0] = side[i + 1][j];	//����
				v[1] = side[i][j];		//����
				v[2] = side[i + 1][j + 1];	//�E��
				v[3] = side[i + 1][j + 1];	//�E��
				v[4] = side[i][j];		//����
				v[5] = side[i][j + 1];		//�E��
				indice[0] = (i + 1) * vertexX + j + 2;
				indice[1] = (i) * vertexX + j + 2;
				indice[2] = (i + 1) * vertexX + j + 1 + 2;
				indice[3] = (i + 1) * vertexX + j + 1 + 2;
				indice[4] = (i) * vertexX + j + 2;
				indice[5] = (i) * vertexX + j + 1 + 2;

			}
			else
			{
				v[0] = side[i + 1][j];	//����
				v[1] = side[i][j];		//����
				v[2] = side[i + 1][0];	//�E��
				v[3] = side[i + 1][0];	//�E��
				v[4] = side[i][j];		//����
				v[5] = side[i][0];		//�E��
				indice[0] = (i + 1) * vertexX + j + 2;
				indice[1] = (i)* vertexX + j + 2;
				indice[2] = (i + 1) * vertexX + 2;
				indice[3] = (i + 1) * vertexX + 2;
				indice[4] = (i)* vertexX + j + 2;
				indice[5] = (i)* vertexX + 2;
			}
			for (int k = 0; k < 6; k++)
			{
				vertices.emplace_back(v[k]);
				//�G�b�W�������p�̃f�[�^��ǉ�
				if (smoothing) {
					//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
					AddSmoothData(indice[k], (unsigned short)vertices.size() - 1);
				}
			}
		}
	}


	for (int i = 0; i < vertexX * 6 + vertexX * 6 * (vertexY - 2); i++)
	{
		indices.emplace_back(i);
	}

	//�@���̌v�Z
	for (int i = 0; i < vertices.size() / 3; i++)
	{
		//�O�p�`1���ƂɌv�Z���Ă���
		//�O�p�`�̃C���f�b�N�X�����o���āA�ꎞ�I�ȕϐ��ɓ����
		unsigned short index0 = indices[i * 3 + 0];
		unsigned short index1 = indices[i * 3 + 1];
		unsigned short index2 = indices[i * 3 + 2];
		//�O�p�`���\�����钸�_���W���x�N�g���ɑ��
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//p0��p1�x�N�g���Ap0��p2�x�N�g�����v�Z�i�x�N�g���̌��Z�j
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//�O�ς͗������琂���ȃx�N�g��
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//���K���i������1�ɂ���j
		normal = XMVector3Normalize(normal);
		//���߂��@���𒸓_�f�[�^�ɑ��
		XMStoreFloat3(&vertices[index0].normal, normal);
		XMStoreFloat3(&vertices[index1].normal, normal);
		XMStoreFloat3(&vertices[index2].normal, normal);

	}

	if (smoothing) {
		CalculateSmoothedVertexNormals();
	}

	CreateBuffer();
}

void Model::CreatePoll(int vertex, float radius, float height, int index, bool smoothing)
{
	isLoadFromOBJFile = false;

	material.ambient = { 0.5f,0.5f,0.5f };
	material.diffuse = { 0.5f,0.5f,0.5f };

	material.name = "default_poll";

	material.textureFilename = "white1x1.png";
	LoadTexture("Resources/", material.textureFilename, index);

	vector<Vertex> v;
	Vertex v0_up;
	v0_up.pos = { 0,height / 2,0 };
	v.emplace_back(v0_up);

	//�V��̍��W��
	for (int i = 0; i < vertex; i++)
	{
		Vertex vert;
		float f = 360.0f / vertex;
		float rad = XMConvertToRadians(f * i);
		vert.pos = { std::cos(rad) * radius, height / 2, std::sin(rad) * radius };
		v.emplace_back(vert);
	}

	Vertex v0_down;
	v0_down.pos = { 0,-height / 2,0 };
	v.emplace_back(v0_down);

	//��ʂ̒��_���W�����߂�
	for (int i = 0; i < vertex; i++)
	{
		Vertex vert;
		float f = 360.0f / vertex;
		float rad = XMConvertToRadians(f * i);
		vert.pos = { std::cos(rad) * radius, -height / 2, std::sin(rad) * radius };
		v.emplace_back(vert);
	}

	//for (int i = 0; i < vertex * 2 + 2; i++)
	//	v[i].pos.x = i;

	//�i�[
	//���
	for (int i = 0; i < vertex; i++)
	{
		if (i + 1 < vertex)
		{
			vertices.emplace_back(v[i + 1]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(i + 1, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[0]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(0, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[i + 2]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(i + 2, (unsigned short)vertices.size() - 1);
			}
		}
		else
		{
			vertices.emplace_back(v[i + 1]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(i + 1, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[0]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(0, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[vertex - i]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(vertex - i, (unsigned short)vertices.size() - 1);
			}
		}
	}

	//����
	for (int i = vertex; i < vertex * 2; i++)
	{
		if (i + 1 < vertex * 2)
		{
			vertices.emplace_back(v[i + 2]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(i + 2, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[i + 3]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(i + 3, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[vertex + 1]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(vertex + 1, (unsigned short)vertices.size() - 1);
			}
		}
		else
		{
			vertices.emplace_back(v[i + 2]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(i + 2, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[vertex * 2 - i + vertex + 1]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(vertex * 2 - i + vertex + 1, (unsigned short)vertices.size() - 1);
			}
			vertices.emplace_back(v[vertex + 1]);
			//�G�b�W�������p�̃f�[�^��ǉ�
			if (smoothing) {
				//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
				AddSmoothData(vertex + 1, (unsigned short)vertices.size() - 1);
			}
		}
	}


	for (int i = 0; i < vertex; i++)
	{
		int n[4];
		if (i + 1 < vertex)
		{
			n[0] = i + 1;
			n[1] = i + 2;
			n[2] = vertex + 2 + i;
			n[3] = vertex + 3 + i;
		}
		else
		{
			n[0] = i + 1;
			n[1] = 1;
			n[2] = vertex + 2 + i;
			n[3] = vertex + 2;
		}
		vertices.emplace_back(v[n[2]]);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[2], (unsigned short)vertices.size() - 1);
		}
		vertices.emplace_back(v[n[0]]);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[0], (unsigned short)vertices.size() - 1);
		}
		vertices.emplace_back(v[n[3]]);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[3], (unsigned short)vertices.size() - 1);
		}
		vertices.emplace_back(v[n[3]]);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[3], (unsigned short)vertices.size() - 1);
		}
		vertices.emplace_back(v[n[0]]);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[0], (unsigned short)vertices.size() - 1);
		}
		vertices.emplace_back(v[n[1]]);
		//�G�b�W�������p�̃f�[�^��ǉ�
		if (smoothing) {
			//v�L�[(���W�f�[�^)�̔ԍ��ƁA�S�č����������_�̃C���f�b�N�X���Z�b�g�œo�^����
			AddSmoothData(n[1], (unsigned short)vertices.size() - 1);
		}

	}

	//�C���f�b�N�X
	for (int i = 0; i < vertex * 3 * 2 + 6 * vertex; i++)
	{
		indices.emplace_back(i);
	}

	//for (int i = vertices.size(); i > vertex * 6; i--)
	//{
	//	vertices.pop_back();
	//	indices.pop_back();
	//}

	//�@���̌v�Z
	for (int i = 0; i < vertices.size() / 3; i++)
	{
		//�O�p�`1���ƂɌv�Z���Ă���
		//�O�p�`�̃C���f�b�N�X�����o���āA�ꎞ�I�ȕϐ��ɓ����
		unsigned short index0 = indices[i * 3 + 0];
		unsigned short index1 = indices[i * 3 + 1];
		unsigned short index2 = indices[i * 3 + 2];
		//�O�p�`���\�����钸�_���W���x�N�g���ɑ��
		XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
		//p0��p1�x�N�g���Ap0��p2�x�N�g�����v�Z�i�x�N�g���̌��Z�j
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//�O�ς͗������琂���ȃx�N�g��
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//���K���i������1�ɂ���j
		normal = XMVector3Normalize(normal);
		//���߂��@���𒸓_�f�[�^�ɑ��
		XMStoreFloat3(&vertices[index0].normal, normal);
		XMStoreFloat3(&vertices[index1].normal, normal);
		XMStoreFloat3(&vertices[index2].normal, normal);

	}

	if (smoothing) {
		CalculateSmoothedVertexNormals();
	}

	CreateBuffer();
}

void Model::CreateSquare(float width, float height, int index)
{
	isLoadFromOBJFile = false;

	material.ambient = { 0.5f,0.5f,0.5f };
	material.diffuse = { 0.5f,0.5f,0.5f };

	material.name = "default_square";

	material.textureFilename = "white1x1.png";
	LoadTexture("Resources/", material.textureFilename, index);

	float div2Width = width / 2.0f;
	float div2Height = height / 2.0f;

	vertices.push_back({ {-div2Width,-div2Height,0.0f},{0,0,1},{0,1} });	//����
	vertices.push_back({ {-div2Width,+div2Height,0.0f},{0,0,1},{0,0} });	//����
	vertices.push_back({ {+div2Width,-div2Height,0.0f},{0,0,1},{1,1} });	//�E��
	vertices.push_back({ {+div2Width,+div2Height,0.0f},{0,0,1},{1,0} });	//�E��

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);//�O�p�`1

	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);//�O�p�`2

	CreateBuffer();

}

void Model::CreateSquareTex(float standardLength, std::string texName, int index)
{
	isLoadFromOBJFile = false;

	material.ambient = { 0.5f,0.5f,0.5f };
	material.diffuse = { 0.5f,0.5f,0.5f };

	material.name = "square_tex";

	material.textureFilename = texName;
	float* texWidth = new float();
	float* texHeight = new float();
	LoadTextureReturnTexSize("Resources/", material.textureFilename, index, texWidth, texHeight);

	float than = 0.0f;
	float div2Width = 0.0f;
	float div2Height = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	if (*texWidth >= *texHeight)//����
	{
		than = *texWidth / *texHeight;
		width = standardLength * than;
		height = standardLength;
	}
	else
	{
		than = *texHeight / *texWidth;
		width = standardLength;
		height = standardLength * than;
	}
	div2Width = width / 2.0f;
	div2Height = height / 2.0f;

	vertices.push_back({ {-div2Width,-div2Height,0.0f},{0,0,1},{0,1} });	//����
	vertices.push_back({ {-div2Width,+div2Height,0.0f},{0,0,1},{0,0} });	//����
	vertices.push_back({ {+div2Width,-div2Height,0.0f},{0,0,1},{1,1} });	//�E��
	vertices.push_back({ {+div2Width,+div2Height,0.0f},{0,0,1},{1,0} });	//�E��

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);//�O�p�`1

	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);//�O�p�`2

	delete texWidth;
	delete texHeight;

	CreateBuffer();
}
