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

	//���C�g������
	light.Initialize();
	light.SetLightColor({ 1,1,1 });
	Object3D::SetLight(light);

	model1.CreateBox(10,8,6,0,true);
	model2.CreateSphere(50, 50, 15, 1);

	model1.Initialize();
	model2.Initialize();

	obj1.SetModel(model1);
	obj1.Initialize();
	obj1.SetPosition(0,-15,0);
	obj1.SetColorAs0To255(227,185,83);

	obj2.SetModel(model2);
	obj2.Initialize();
	obj2.SetPosition({ 6, 0, 0 });
	obj2.SetColorAs0To255(255,128,128);

	sprite1.Initialize(1, L"Resources/haikeidayo.png");
	sprite2.Initialize(2, L"Resources/gazoudayo.png");

	sphere1.center = DirectX::XMVectorSet(obj1.GetPosition().x, obj1.GetPosition().y, obj1.GetPosition().z, 0);
	sphere1.radius = 30.0f;
	sphere2.center = DirectX::XMVectorSet(obj2.GetPosition().x, obj2.GetPosition().y, obj2.GetPosition().z, 0);
	sphere2.radius = 15.0f;

	ip.Start(3.0f);
}

void GamePlay::Update()
{
	// DirectX ���t���[������ ��������

	//���͏�����������

	ip.Update(600);

#pragma region �X�V����
	if (Input::Push(DIK_UP) || Input::Push(DIK_DOWN))
	{
		if (Input::Push(DIK_SPACE))
		{
			if (Input::Push(DIK_LSHIFT))
			{
				//���_���W�����Z���鏈���iZ�j
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
				//�J�����̈ʒu���ړ����鏈���iZ�j
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
				//���_���W�����Z���鏈���iY�j
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
				//�J�����̈ʒu���ړ����鏈���iY�j
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
			//���_���W�����Z���鏈���iX�j
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

			//�J�����̈ʒu���ړ����鏈���iX�j
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

	if (!Input::Push(DIK_LCONTROL))
	{

		if (Input::Push(DIK_W) || Input::Push(DIK_S))
		{
			//���W���ړ����鏈��(Y���W)
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
			//���W���ړ����鏈��(X���W)
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
			//���W���ړ����鏈��(Z���W)
			if (Input::Push(DIK_Z))
			{
				obj1.AddPosition(0, 0, -1.0f);
			}
			else if (Input::Push(DIK_X))
			{
				obj1.AddPosition(0, 0, 1.0f);

			}

		}
	}

	if (Input::Push(DIK_C) || Input::Push(DIK_V))
	{
		//���W���ړ����鏈��(X���W)
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
		//���W���ړ����鏈��(X���W)
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

	obj2.SetPosition(Interpolation::EaseInOut(-30, 30, ip.GetTimeRate()), 0, -10);

	obj1.AddRotation(0,1,0);
	obj2.AddRotation(0, 1, 0);

#pragma endregion


#pragma region �������蔻��

	sphere1.center = DirectX::XMVectorSet(obj1.GetPosition().x, obj1.GetPosition().y, obj1.GetPosition().z, 0);
	sphere2.center = DirectX::XMVectorSet(obj2.GetPosition().x, obj2.GetPosition().y, obj2.GetPosition().z, 0);

	if (Collision::CheckSphere2Sphere(sphere1,sphere2))
		DebugText::Print("ATATTAYO", 0, 0);

#pragma endregion

#pragma region ���C�g
	{
		//�������������l			  �� ��
		static XMVECTOR lightDir = { 0, 1, 5, 0 };

		if (Input::Push(DIK_LCONTROL))
		{
			if (Input::Push(DIK_W)) { lightDir.m128_f32[1] += 1.0f; }
			else if (Input::Push(DIK_S)) { lightDir.m128_f32[1] -= 1.0f; }
			if (Input::Push(DIK_D)) { lightDir.m128_f32[0] += 1.0f; }
			else if (Input::Push(DIK_A)) { lightDir.m128_f32[0] -= 1.0f; }
		}

		light.SetLightDir(lightDir);

	}
#pragma endregion

#pragma region �I�u�W�F�N�g�A�b�v�f�[�g
	light.Update();

	obj1.Update();
	
	obj2.Update();

#pragma endregion

}

void GamePlay::Draw()
{
	//�`��J�n
	DX12Util::BeginDraw();

	//�w�i�X�v���C�g�`�悱������
	//sprite1.Draw();



	//�w�i�`�悱���܂�
	DX12Util::ClearDepthBuffer();


	//�I�u�W�F�N�g�`�悱������
	obj1.Draw();
	
	//obj2.Draw();

	//�I�u�W�F�N�g�`�悱���܂�

	//�O�i�X�v���C�g�`�悱������

	//sprite2.Draw();
	DebugText::DrawAll();

	//�O�i�X�v���C�g�`�悱���܂�

	//�`��I��
	DX12Util::EndDraw();
}
