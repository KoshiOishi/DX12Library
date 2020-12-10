#include "GamePlay.h"
#include "DX12Util.h"
#include "Input.h"
#include "Sound.h"
#include "DebugText.h"
#include "Collision.h"
#include "FPSManager.h"
#include "Object3D.h"

GamePlay::GamePlay()
{
	sceneName = "GamePlay";
}

GamePlay::~GamePlay()
{
}

void GamePlay::Initialize()
{
	Object3D::SetEye(DirectX::XMFLOAT3(0, 0, -100));

	model1.CreateSphere(10,10,30,1);
	model2.CreatePoll(20,10,10,2);

	model1.Initialize();
	model2.Initialize();

	obj1.SetModel(model1);
	obj1.Initialize();

	obj2.SetModel(model2);
	obj2.Initialize();
	obj2.SetPosition({ 6, 0, 0 });

	obj3.SetModel(model1);
	obj3.Initialize();
	obj3.SetPosition({ -30, 0, -10 });
	obj3.SetColorAs0To255(64, 255, 64, 255);

	sprite1.Initialize(1, L"Resources/haikeidayo.png");
	sprite2.Initialize(2, L"Resources/gazoudayo.png");

	sphere1.center = DirectX::XMVectorSet(obj1.GetPosition().x, obj1.GetPosition().y, obj1.GetPosition().z, 1);
	sphere1.radius = 2.5f;
	sphere2.center = DirectX::XMVectorSet(obj2.GetPosition().x, obj2.GetPosition().y, obj2.GetPosition().z, 1);
	sphere2.radius = 2.5f;

	ip.Start(3.0f);
}

void GamePlay::Update()
{
	// DirectX 毎フレーム処理 ここから

	//入力処理ここから

	ip.Update(600);

#pragma region 更新処理
	if (Input::Push(DIK_UP) || Input::Push(DIK_DOWN))
	{
		if (Input::Push(DIK_SPACE))
		{
			if (Input::Push(DIK_LSHIFT))
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
			else {
				//カメラの位置を移動する処理（Z）
				if (Input::Push(DIK_UP))
				{
					Object3D::MoveCamera(0, 0, 3.0f);
				}
				else if (Input::Push(DIK_DOWN))
				{
					Object3D::MoveCamera(0, 0, -3.0f);

				}
			}
		}
		else
		{
			if (Input::Push(DIK_LSHIFT))
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
			else {
				//カメラの位置を移動する処理（Y）
				if (Input::Push(DIK_UP))
				{
					Object3D::MoveCamera(0, 3.0f, 0);
				}
				else if (Input::Push(DIK_DOWN))
				{
					Object3D::MoveCamera(0, -3.0f, 0);

				}
			}
		}


	}

	if (Input::Push(DIK_LEFT) || Input::Push(DIK_RIGHT))
	{
		if (Input::Push(DIK_LSHIFT))
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
		else {

			//カメラの位置を移動する処理（X）
			if (Input::Push(DIK_LEFT))
			{
				Object3D::MoveCamera(-3.0f, 0, 0);
			}
			else if (Input::Push(DIK_RIGHT))
			{
				Object3D::MoveCamera(3.0f, 0, 0);
			}
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

	if (Input::Trigger(DIK_1))
	{
	 	obj1.SetIsWireFlame(!obj1.GetIsWireFlame());
		obj2.SetIsWireFlame(!obj2.GetIsWireFlame());
	}

	obj3.SetPosition(Interpolation::EaseInOut(-30, 30, ip.GetTimeRate()), 0, -10);

#pragma endregion


#pragma region 球当たり判定

	sphere1.center = DirectX::XMVectorSet(obj1.GetPosition().x, obj1.GetPosition().y, obj1.GetPosition().z, 1);
	sphere2.center = DirectX::XMVectorSet(obj2.GetPosition().x, obj2.GetPosition().y, obj2.GetPosition().z, 1);

	if (Collision::CheckSphere2Sphere(sphere1,sphere2))
		DebugText::Print("HIT", 0, 0);

#pragma endregion


#pragma region オブジェクトアップデート

	obj1.Update();
	
	obj2.Update();

	obj3.Update();

#pragma endregion

}

void GamePlay::Draw()
{
	//描画開始
	DX12Util::BeginDraw();

	//背景スプライト描画ここから
	sprite1.Draw();



	//背景描画ここまで
	DX12Util::ClearDepthBuffer();


	//オブジェクト描画ここから
	obj1.Draw();
	
	obj2.Draw();

	obj3.Draw();

	//オブジェクト描画ここまで

	//前景スプライト描画ここから

	//sprite2.Draw();
	DebugText::DrawAll();

	//前景スプライト描画ここまで

	//描画終了
	DX12Util::EndDraw();
}
