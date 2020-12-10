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

//サブクラス
public:
	struct Vertex
	{
		XMFLOAT3 pos;	 //xyz座標
		XMFLOAT3 normal; //法線ベクトル
		XMFLOAT2 uv;	//uv座標
	};

	// 定数バッファ用データ構造体B1
	struct ConstBufferDataB1
	{
		XMFLOAT3 ambient;	//アンビエント係数
		float pad1;			//パディング
		XMFLOAT3 diffuse;	//ディフューズ係数
		float pad2;			//パディング
		XMFLOAT3 specular;	//スペキュラー係数
		float alpha;		//アルファ
	};

	//マテリアル
	struct Material
	{
		std::string name;	//マテリアル名
		XMFLOAT3 ambient;	//アンビエント影響度
		XMFLOAT3 diffuse;	//ディフューズ影響度
		XMFLOAT3 specular;	//スペキュラー影響度
		float alpha;		//アルファ
		std::string textureFilename;	//テクスチャファイル名
		//コンストラクタ
		Material() {
			ambient = { 0.5f, 0.5f, 0.5f };
			diffuse = { 0.0f, 0.0f, 0.0f };
			specular = { 0.0f,0.0f,0.0f };
			alpha = 1.0f;
		}
	};

private:
	// 頂点バッファ
	ComPtr < ID3D12Resource> vertBuff = nullptr;

	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	// インデックスバッファ
	ComPtr <ID3D12Resource> indexBuff = nullptr;

	//インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW ibView{};

	//頂点データ
	vector<Vertex> vertices;

	//インデックスデータ
	vector<unsigned short> indices;

	//マテリアル
	Material material;

	// テクスチャバッファ
	ComPtr <ID3D12Resource> texbuff = nullptr;

	// シェーダリソースビューのハンドル(CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;

	// シェーダリソースビューのハンドル(GPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;

	//定数バッファ
	ComPtr<ID3D12Resource> constBuffB1;

	//OBJから読み込まれたか否か
	bool isLoadFromOBJFile = true;

//メンバ関数
public:
	void Initialize();
	void Update();
	void Draw();

	/// <summary>
	/// OBJファイルからモデルを作成する（フォルダ名と中身の名前は同一のものにしてください。）
	/// </summary>
	/// <param name="modelname">objファイルを格納してるフォルダ名</param>
	/// <param name="index">インデックス（重複しないように指定）</param>
	void LoadOBJ(const std::string & modelname, int index);
	void LoadMaterial(const std::string & directoryPath, const std::string & filename, int index);
	bool LoadTexture(const std::string & directoryPath, const std::string & filename, int index);
	bool LoadTextureReturnTexSize(const std::string & directoryPath, const std::string & filename, int index, float* texWidth = nullptr, float* texHeight = nullptr);
	void CreateBuffer();

	/// <summary>
	/// OBJファイルから作られたモデルであるかどうか返す
	/// </summary>
	/// <returns></returns>
	bool GetIsOBJ() { return isLoadFromOBJFile; }

	//以下ポリゴン作成
	/// <summary>
	/// 四角形ポリゴンを作成。中心基準。
	/// </summary>
	/// <param name="width">横幅</param>
	/// <param name="height">縦幅</param>
	/// <param name="depth">奥行</param>
	/// <param name="index">インデックス（重複しないように指定）</param>
	void CreateBox(float width, float height, float depth, int index);

	/// <summary>
	/// 球ポリゴンを作成
	/// </summary>
	/// <param name="vertexX">軸の分割数 (3以上)</param>
	/// <param name="vertexY">高さの分割数 (3以上)</param>
	/// <param name="radius">半径</param>
	/// <param name="index">インデックス（重複しないように指定）</param>
	void CreateSphere(int vertexX, int vertexY, float radius, int index);

	/// <summary>
	/// 円柱ポリゴンを作成
	/// </summary>
	/// <param name="vertex">底面の頂点数</param>
	/// <param name="radius">半径</param>
	/// <param name="height">高さ</param>
	/// <param name="index">インデックス（重複しないように指定）</param>
	void CreatePoll(int vertex, float radius, float height, int index);

	/// <summary>
	/// 四角形板ポリゴンを作成
	/// </summary>
	/// <param name="width">横幅</param>
	/// <param name="height">縦幅</param>
	/// <param name="index">インデックス（重複しないように指定）</param>
	void CreateSquare(float width, float height, int index);

	/// <summary>
	/// テクスチャあり四角形板ポリゴンを作成
	/// </summary>
	/// <param name="standardLength">ポリゴンの基準の長さ テクスチャが横長の場合は縦幅に、縦長の場合は横幅に適応され、もう片方は自動で合わせる。</param>
	/// <param name="texName">テクスチャの名前</param>
	/// <param name="index">インデックス（重複しないように指定）</param>
	void CreateSquareTex(float standardLength, string texName, int index);

};

