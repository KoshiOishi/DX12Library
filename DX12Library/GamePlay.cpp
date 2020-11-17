#include "GamePlay.h"
#include "DX12Init.h"
#include "Input.h"
#include "Sound.h"
#include "DebugText.h"

GamePlay::GamePlay()
{
	nextScene = "";
	sceneName = "GamePlay";
}

void GamePlay::Initialize()
{

	for (int i = 0; i < 5; i++)
	{
		obj1[i].LoadOBJ("vending_machine");
		obj1[i].Initialize();
		obj1[i].SetPosition({ (float)(rand() % 20 - 10), (float)(rand() % 20 - 10), (float)(rand() % 20 - 10) });
	}

	obj2.LoadOBJ("post");
	obj2.Initialize();
	obj2.SetPosition({ 30, 0, 0 });


	Sprite::SpriteLoadTexture(1, L"Resources/haikeidayo.png");
	Sprite::SpriteLoadTexture(2, L"Resources/gazoudayo.png");
	sprite1.SpriteSetTexNumber(1);
	sprite1.GenerateSprite();
	sprite2.SpriteSetTexNumber(2);
	sprite2.GenerateSprite();

	sprite2.SpriteSetSize(600.0f,300.0f);

	//デバッグテキスト用のテクスチャ番号を指定
	DebugText::SetDebugTextTexNumber(0);
	//デバッグテキスト用のテクスチャ読み込み
	Sprite::SpriteLoadTexture(DebugText::GetDebugTextTexNumber(), L"Resources/debugfont.png");
	//デバッグテキスト初期化
	DebugText::Initialize();

}

void GamePlay::Update()
{
	// DirectX 毎フレーム処理 ここから

	//キーボード情報の取得開始
	Input::Update();
	//入力処理ここから

#pragma region 更新処理
	if (Input::Push(DIK_UP) || Input::Push(DIK_DOWN))
	{
		if (Input::Push(DIK_SPACE))
		{
			//視点座標を加算する処理（Z）
			if (Input::Push(DIK_UP))
			{
				Object3D::AddEye(0, 0, 3.0f);
			}
			else if (Input::Push(DIK_DOWN))
			{
				Object3D::AddEye(0, 0, -3.0f);

			}
		}
		else
		{
			//視点座標を加算する処理（Y）
			if (Input::Push(DIK_UP))
			{
				Object3D::AddEye(0, 3.0f, 0);
			}
			else if (Input::Push(DIK_DOWN))
			{
				Object3D::AddEye(0, -3.0f, 0);

			}
		}
	}

	if (Input::Push(DIK_LEFT) || Input::Push(DIK_RIGHT))
	{
		//視点座標を加算する処理（X）
		if (Input::Push(DIK_LEFT))
		{
			Object3D::AddEye(-3.0f, 0, 0);
		}
		else if (Input::Push(DIK_RIGHT))
		{
			Object3D::AddEye(3.0f, 0, 0);
		}
	}


	if (Input::Push(DIK_W) || Input::Push(DIK_S))
	{
		//座標を移動する処理(Y座標)
		if (Input::Push(DIK_W))
		{
			obj1[0].AddPosition(0, 1.0f, 0);

		}
		else if (Input::Push(DIK_S))
		{
			obj1[0].AddPosition(0, -1.0f, 0);

		}
	}
	if (Input::Push(DIK_A) || Input::Push(DIK_D))
	{
		//座標を移動する処理(X座標)
		if (Input::Push(DIK_A))
		{
			obj1[0].AddPosition(-1.0f, 0, 0);
		}
		else if (Input::Push(DIK_D))
		{
			obj1[0].AddPosition(1.0f, 0, 0);

		}

	}

	if (Input::Push(DIK_Z) || Input::Push(DIK_X))
	{
		//座標を移動する処理(Z座標)
		if (Input::Push(DIK_Z))
		{
			obj1[0].AddPosition(0, 0, -1.0f);
		}
		else if (Input::Push(DIK_X))
		{
			obj1[0].AddPosition(0, 0, 1.0f);

		}

	}

	//if (Input::Trigger(DIK_SPACE))
	//{
	//	Sound::PlayWave("Resources/Alarm01.wav");
	//}

#pragma endregion


#pragma region 球当たり判定

		////各オブジェクトの半径
		//const float radius1 = 15;
		//const float radius2 = 15;

		////ワールド座標を行列から読み取る
		//XMFLOAT3 position1 = { obj1.matWorld.r[3].m128_f32[0],
		//	obj1.matWorld.r[3].m128_f32[1],obj1.matWorld.r[3].m128_f32[2] };
		//XMFLOAT3 position2 = { obj2.matWorld.r[3].m128_f32[0],
		//	obj2.matWorld.r[3].m128_f32[1],obj2.matWorld.r[3].m128_f32[2] };

		////2つの座標の差
		//XMVECTOR position_sub = XMVectorSet(
		//	position1.x - position2.x,
		//	position1.y - position2.y,
		//	position1.z - position2.z,
		//	0
		//);

		////2つの座標の距離
		//position_sub = XMVector3Length(position_sub);
		//float distance = position_sub.m128_f32[0];

		////距離が半径の和以下なら当たっている
		//if (distance <= radius1 + radius2) {
		//	debugText.Print("Hit", 100, 100, 10.0f);
		//}



#pragma endregion

	DebugText::Print(100, 0, 0);

#pragma region オブジェクトアップデート

	for (int i = 0; i < 5; i++)
	{
		obj1[i].Update();
	}
	obj2.Update();

#pragma endregion

}

void GamePlay::Draw()
{
	//描画開始
	DX12Init::BeginDraw();

	//背景スプライト描画ここから
	sprite1.SpriteDraw();



	//背景描画ここまで
	DX12Init::ClearDepthBuffer();


	//オブジェクト描画ここから
	for (int i = 0; i < 5; i++)
	{
		obj1[i].Draw();
	}
	obj2.Draw();

	//オブジェクト描画ここまで

	//前景スプライト描画ここから

	sprite2.SpriteDraw();
	DebugText::DrawAll();

	//前景スプライト描画ここまで

	//描画終了
	DX12Init::EndDraw();
}
