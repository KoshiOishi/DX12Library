#include "GamePlay.h"
#include "DX12Init.h"
#include "Input.h"
#include "Sound.h"
#include "DebugText.h"

GamePlay::GamePlay()
{
	sceneName = "GamePlay";
}

GamePlay::~GamePlay()
{
}

void GamePlay::Initialize()
{
	obj1.Initialize();

	model1.LoadOBJ("Sphere", 1);
	model2.CreatePoll(6, 1, 30, 0);

	model1.Initialize();
	model2.Initialize();

	obj1.SetModel(model1);
	obj1.SetColorAs0To255(255, 255, 64, 255);

	obj2.SetModel(model2);
	obj2.Initialize();
	obj2.SetPosition({ 30, 0, 0 });

	sprite1.Initialize(1, L"Resources/haikeidayo.png");
	sprite2.Initialize(2, L"Resources/gazoudayo.png");

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
			obj1.AddPosition(0, 1.0f, 0);

		}
		else if (Input::Push(DIK_S))
		{
			obj1.AddPosition(0, -1.0f, 0);

		}
	}
	if (Input::Push(DIK_A) || Input::Push(DIK_D))
	{
		//座標を移動する処理(X座標)
		if (Input::Push(DIK_A))
		{
			obj1.AddPosition(-1.0f, 0, 0);
		}
		else if (Input::Push(DIK_D))
		{
			obj1.AddPosition(1.0f, 0, 0);

		}

	}

	if (Input::Push(DIK_Z) || Input::Push(DIK_X))
	{
		//座標を移動する処理(Z座標)
		if (Input::Push(DIK_Z))
		{
			obj1.AddPosition(0, 0, -1.0f);
		}
		else if (Input::Push(DIK_X))
		{
			obj1.AddPosition(0, 0, 1.0f);

		}

	}

	if (Input::Push(DIK_C) || Input::Push(DIK_V))
	{
		//座標を移動する処理(X座標)
		if (Input::Push(DIK_C))
		{
			obj1.AddRotation(0, -1.0f, 0);
		}
		else if (Input::Push(DIK_V))
		{
			obj1.AddRotation(0, 1.0f, 0);

		}

	}

	if (Input::Push(DIK_F) || Input::Push(DIK_R))
	{
		//座標を移動する処理(X座標)
		if (Input::Push(DIK_F))
		{
			obj1.AddRotation(1.0f, 0, 0);
		}
		else if (Input::Push(DIK_R))
		{
			obj1.AddRotation(-1.0f, 0, 0);

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

	obj1.Update();
	
	obj2.Update();

#pragma endregion

}

void GamePlay::Draw()
{
	//描画開始
	DX12Init::BeginDraw();

	//背景スプライト描画ここから
	sprite1.Draw();



	//背景描画ここまで
	DX12Init::ClearDepthBuffer();


	//オブジェクト描画ここから
	obj1.Draw();
	
	obj2.Draw();

	//オブジェクト描画ここまで

	//前景スプライト描画ここから

	sprite2.Draw();
	DebugText::DrawAll();

	//前景スプライト描画ここまで

	//描画終了
	DX12Init::EndDraw();
}
