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
		
//サブクラス
public:

	// 定数バッファ用データ構造体
	struct ConstBufferDataB0 {
		XMFLOAT4 color;	//色(RGBA)
		XMMATRIX mat;	//3D変換行列
	};



//定数
private:


//メンバ変数
private:


	// スケーリング倍率
	XMFLOAT3 scale;

	// 回転角
	XMFLOAT3 rotation;

	// 座標
	XMFLOAT3 position;

	// ワールド座標
	XMMATRIX matWorld;

	//モデル
	Model model;

	//ワイヤフレーム表示
	bool isWireFlane;

	//色
	XMFLOAT4 color = { 1,1,1,1 };

#pragma region 初期化回りの変数


	// 定数バッファ
	ComPtr<ID3D12Resource> constBuffB0;




#pragma endregion

//静的メンバ変数
private:

	// 射影変換行列
	static XMMATRIX matProjection;

	// ビュー変換行列
	static XMMATRIX matView;

	// 視点座標
	static XMFLOAT3 eye;

	// 注視点座標
	static XMFLOAT3 target;

	// 上方向ベクトル
	static XMFLOAT3 up;


	// デスクリプタヒープ
	static ComPtr <ID3D12DescriptorHeap> basicDescHeap;

	// OBJ用ルートシグネチャ
	static ComPtr <ID3D12RootSignature> rootsignatureOBJ;

	// 数式用ルートシグネチャ
	static ComPtr <ID3D12RootSignature> rootsignatureMath;

	// OBJ用パイプラインステート
	static ComPtr <ID3D12PipelineState> pipelinestateOBJ;

	// OBJ用ワイヤフレームパイプラインステート
	static ComPtr <ID3D12PipelineState> pipelinestateOBJ_wire;

	// 数式オブジェクト用パイプラインステート
	static ComPtr <ID3D12PipelineState> pipelinestateMath;

	// 数式オブジェクト用ワイヤフレームパイプラインステート
	static ComPtr <ID3D12PipelineState> pipelinestateMath_wire;

//メンバ関数

public:

	/// <summary>
	/// 初期化処理 ※LoadOBJの後に記述！
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// モデルをセットする
	/// </summary>
	/// <param name="model">モデル</param>
	void SetModel(Model model);

#pragma region 便利関数
//メンバ関数
public:
	/// <summary>
	/// スケールを指定分加算する
	/// </summary>
	/// <param name="scaleX">スケールX</param>
	/// <param name="scaleY">スケールY</param>
	/// <param name="scaleZ">スケールZ</param>
	void AddScale(const float scaleX, const float scaleY, const float scaleZ)
	{
		this->scale.x += scaleX; this->scale.y += scaleY; this->scale.z += scaleZ;
	}

	/// <summary>
	/// 回転角を指定分加算する
	/// </summary>
	/// <param name="rotationX">回転角X</param>
	/// <param name="rotationY">回転角Y</param>
	/// <param name="rotationZ">回転角Z</param>
	void AddRotation(const float rotationX, const float rotationY, const float rotationZ)
	{
		this->rotation.x += rotationX; this->rotation.y += rotationY; this->rotation.z += rotationZ;
	}

	/// <summary>
	/// 座標を指定分加算する
	/// </summary>
	/// <param name="positionX">座標X</param>
	/// <param name="positionY">座標Y</param>
	/// <param name="positionZ">座標Z</param>
	void AddPosition(const float positionX, const float positionY, const float positionZ)
	{
		this->position.x += positionX; this->position.y += positionY; this->position.z += positionZ;
	}



//静的メンバ関数
public:
	static void AddEye(const float eyeX, const float eyeY, const float eyeZ);
	static void AddTarget(const float targetX, const float targetY, const float targetZ);
	static void AddUp(const float upX, const float upY, const float upZ);

	
#pragma endregion

//静的メンバ関数
public:
	
	/// <summary>
	/// Object3D自体の初期化。最初に書く。
	/// </summary>
	static void FirstInit();

	static void InitPipelineOBJ();

	static void InitPipelineMath();

#pragma region Getter
//メンバ関数
public:
	/// <summary>
	/// スケールの取得
	/// </summary>
	/// <returns>座標</returns>
	const XMFLOAT3& GetScale() { return scale; }

	/// <summary>
	/// 回転角の取得
	/// </summary>
	/// <returns>座標</returns>
	const XMFLOAT3& GetRotation() { return rotation; }


	/// <summary>
	/// 座標の取得
	/// </summary>
	/// <returns>座標</returns>
	const XMFLOAT3& GetPosition() { return position; }

	/// <summary>
	/// ワイヤフレーム描画フラグを取得
	/// </summary>
	/// <returns>ワイヤフレームフラグ</returns>
	const bool& GetIsWireFlame() { return isWireFlane; }

	/// <summary>
	/// 色(RGBA)を取得する
	/// </summary>
	/// <returns>色(RGBA)</returns>
	const XMFLOAT4& GetColor() { return color; }

	/// <summary>
	/// 色(RGBA)を0～255の数値で取得する
	/// </summary>
	/// <returns>色(RGBA)、0～255の範囲</returns>
	const XMFLOAT4& GetColorAs0To255(){
		XMFLOAT4 c = { color.x * 255,color.y * 255 ,color.z * 255 ,color.w * 255 };
		return c;
	}

//静的メンバ関数
public:
	/// <summary>
	/// 視点座標の取得
	/// </summary>
	/// <returns>座標</returns>
	static const XMFLOAT3& GetEye() { return eye; }

	/// <summary>
	/// 注視点座標の取得
	/// </summary>
	/// <returns>座標</returns>
	static const XMFLOAT3& GetTarget() { return target; }

	/// <summary>
	/// 上方向ベクトルの取得
	/// </summary>
	/// <returns>上方向ベクトル</returns>
	static const XMFLOAT3& GetUp() { return up; }

	/// <summary>
	/// ビュー行列の更新処理
	/// </summary>
	static void UpdateViewMatrix();

	/// <summary>
	/// デスクリプタヒープのポインタを取得
	/// </summary>
	/// <returns>デスクリプタヒープのポインタ</returns>
	static ID3D12DescriptorHeap* GetDescHeap();

#pragma endregion

#pragma region Setter
//メンバ関数
public:
	/// <summary>
	/// スケールをセット
	/// </summary>
	/// <param name="scale">スケール</param>
	void SetScale(const XMFLOAT3 scale) { this->scale = scale; }

	/// <summary>
	/// 回転角をセット
	/// </summary>
	/// <param name="rotation">回転角</param>
	void SetRotation(const XMFLOAT3 rotation) { this->rotation = rotation; }

	/// <summary>
	/// 座標をセット
	/// </summary>
	/// <param name="position">座標</param>
	void SetPosition(const XMFLOAT3 position) { this->position = position; }

	/// <summary>
	/// ワイヤフレームフラグをセット
	/// </summary>
	void SetIsWireFlame(const bool isWireFlame) { this->isWireFlane = isWireFlame; }

	/// <summary>
	/// 色(RGBA)をセット
	/// </summary>
	/// <param name="R">赤</param>
	/// <param name="G">緑</param>
	/// <param name="B">青</param>
	/// <param name="A">アルファ値</param>
	void SetColor(float R, float G, float B, float A) {
		XMFLOAT4 c = { R,G,B,A };
		SetColor(c);
	}

	/// <summary>
	/// 色(RGBA)をセット
	/// </summary>
	/// <param name="color">色(RGBA)</param>
	void SetColor(XMFLOAT4 color) { this->color = color; }

	/// <summary>
	/// 色(RGBA)を0～255の範囲でセット
	/// </summary>
	/// <param name="R">赤</param>
	/// <param name="G">緑</param>
	/// <param name="B">青</param>
	/// <param name="A">アルファ値</param>
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
	/// 色(RGBA)を0～255の範囲でセット
	/// </summary>
	/// <param name="color">色(RGBA)</param>
	void SetColorAs0To255(XMFLOAT4 color)
	{
		XMFLOAT4 c = { color.x / 255, color.y / 255, color.z / 255, color.w / 255 };
		SetColor(c);
	}
//静的メンバ関数
public:
	/// <summary>
	/// 視点座標をセット
	/// </summary>
	/// <param name="eye">視点座標</param>
	static void SetEye(const XMFLOAT3 eye);

	/// <summary>
	/// 注視点座標をセット
	/// </summary>
	/// <param name="target">注視点座標</param>
	static void SetTarSet(const XMFLOAT3 target);

	/// <summary>
	/// 上方向ベクトルのセット
	/// </summary>
	/// <param name="up"></param>
	static void SetUp(const XMFLOAT3 up);

#pragma endregion
};
