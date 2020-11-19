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

	static ComPtr<ID3D12RootSignature> spriteRootSignature;	//ルートシグネチャ
	static ComPtr<ID3D12PipelineState> spritePipelineState;	//パイプラインステート
	static XMMATRIX spriteMatProjection;		//射影行列
	static ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト
	static ComPtr<ID3DBlob> psBlob; // ピクセルシェーダオブジェクト
	static ComPtr <ID3DBlob> errorBlob; // エラーオブジェクト
	static ComPtr <ID3D12DescriptorHeap> spriteDescHeap;
	static const int spriteSRVCount;
	static ComPtr <ID3D12Resource> spriteTexbuff[];					//テクスチャバッファ

	ComPtr<ID3D12Resource> spriteVertBuff;				//頂点バッファ
	ComPtr<ID3D12Resource> spriteConstBuff;				//定数バッファ
	D3D12_VERTEX_BUFFER_VIEW spriteVBView{};			//頂点バッファビュー

public:
	struct VertexPosUv
	{
		XMFLOAT3 pos;	//xyz座標
		XMFLOAT2 uv;	//uv座標
	};

	// 定数バッファ用データ構造体
	struct ConstBufferData {
		XMFLOAT4 color; // 色 (RGBA)
		XMMATRIX mat;	//3D変換行列
	};

	float spriteRotation = 0.0f;		//Z軸回りの回転角
	XMVECTOR spritePosition{};			//座標
	XMMATRIX spriteMatWorld{};			//ワールド行列
	XMFLOAT4 spriteColor = { 1,1,1,1 };	//色
	UINT texNumber;				//テクスチャ番号
	XMFLOAT2 anchorpoint{};		//アンカーポイント
	float width;		//横幅
	float height;		//縦幅
	bool isFlipX;		//左右入れ替え
	bool isFlipY;		//上下入れ替え
	float tex_x;		//画像切り出しX座標
	float tex_y;		//画像切り出しy座標
	float tex_width;	//画像切り出し横幅
	float tex_height;	//画像切り出し縦幅
	bool isDisplay = true;		//表示状態

	/// <summary>
	/// 初期化処理
	/// </summary>
	static void FirstInit();

	/// <summary>
	/// 初期化処理をまとめたもの。
	/// </summary>
	/// <param name="texnumber">テクスチャ番号</param>
	/// <param name="filename">テクスチャファイルパス</param>
	/// <param name="anchorpoint">アンカーポイント(省略可)</param>
	/// <param name="isFlipX">横反転フラグ(省略可)</param>
	/// <param name="isFlipY">縦反転フラグ(省略可)</param>
	void Initialize(UINT texnumber, const wchar_t* filename, XMFLOAT2 anchorpoint = { 0,0 }, bool isFlipX = false, bool isFlipY = false);

	/// <summary>
	/// スプライトのテクスチャ読み込み処理
	/// </summary>
	/// <param name="texnumber">テクスチャ番号</param>
	/// <param name="filename">ファイルパス</param>
	static void SpriteLoadTexture(UINT texnumber, const wchar_t* filename);

	/// <summary>
	/// スプライトの生成
	/// </summary>
	/// <param name="anchorpoint">アンカーポイント(省略可)</param>
	/// <param name="isFlipX">横回転フラグ(省略可)</param>
	/// <param name="isFlipY">縦回転フラグ(省略可)</param>
	void GenerateSprite(XMFLOAT2 anchorpoint = { 0,0 }, bool isFlipX = false, bool isFlipY = false);



	/// <summary>
	/// スプライトパイプラインをセット スプライト描画前に必ず書く
	/// </summary>
	static void SpriteSetPipeline();

	/// <summary>
	/// 描画処理
	/// </summary>
	void SpriteDraw();

	/// <summary>
	/// スプライトの大きさをセット
	/// </summary>
	/// <param name="width">横幅(px)</param>
	/// <param name="height">縦幅(px)</param>
	void SpriteSetSize(float width, float height);

	/// <summary>
	/// スプライトのアンカーポイントをセット
	/// </summary>
	/// <param name="anchorpoint">アンカーポイント(XMFLOAT2)</param>
	void SpriteSetAnchorpoint(XMFLOAT2 anchorpoint);

	/// <summary>
	/// スプライトの横反転フラグをセット
	/// </summary>
	/// <param name="isFlipX">横反転フラグ</param>
	void SpriteSetIsFlipX(bool isFlipX);

	/// <summary>
	/// スプライトの縦反転フラグをセット
	/// </summary>
	/// <param name="isFlipX">縦反転フラグ</param>
	void SpriteSetIsFlipY(bool isFlipY);

	/// <summary>
	/// スプライトのテクスチャ番号をセット
	/// </summary>
	/// <param name="texnumber">テクスチャ番号</param>
	void SpriteSetTexNumber(UINT texnumber);

	/// <summary>
	/// 頂点バッファを更新
	/// </summary>
	void SpriteUpdateVertBuff();

	/// <summary>
	/// スプライトの描画範囲(レクタングル)をセット
	/// </summary>
	/// <param name="tex_x">切り取りx座標</param>
	/// <param name="tex_y">切り取りy座標</param>
	/// <param name="tex_width">切り取り横幅(px)</param>
	/// <param name="tex_height">	切り取り縦幅(px)</param>
	void SpriteSetDrawRectangle(float tex_x, float tex_y, float tex_width, float tex_height);

};

/*
	ComPtr<ID3D12RootSignature> Sprite::spriteRootSignature = nullptr;	//ルートシグネチャ
	ComPtr<ID3D12PipelineState> Sprite::spritePipelineState = nullptr;	//パイプラインステート
	ComPtr<ID3D12Resource> Sprite::texbuff = nullptr;					//テクスチャバッファ
	ComPtr<ID3DBlob> Sprite::vsBlob = nullptr; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> Sprite::psBlob = nullptr; // ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> Sprite::errorBlob = nullptr; // エラーオブジェクト
	ComPtr<ID3D12DescriptorHeap> Sprite::basicDescHeap = nullptr;
	XMMATRIX Sprite::spriteMatProjection;		//射影行列

*/

