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
	// DirectX ���t���[������ ��������

	//�L�[�{�[�h���̎擾�J�n
	Input::Update();
	//���͏�����������

#pragma region �X�V����
	if (Input::Push(DIK_UP) || Input::Push(DIK_DOWN))
	{
		if (Input::Push(DIK_SPACE))
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
		else
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
	}

	if (Input::Push(DIK_LEFT) || Input::Push(DIK_RIGHT))
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

	//if (Input::Trigger(DIK_SPACE))
	//{
	//	Sound::PlayWave("Resources/Alarm01.wav");
	//}

#pragma endregion


#pragma region �������蔻��

		////�e�I�u�W�F�N�g�̔��a
		//const float radius1 = 15;
		//const float radius2 = 15;

		////���[���h���W���s�񂩂�ǂݎ��
		//XMFLOAT3 position1 = { obj1.matWorld.r[3].m128_f32[0],
		//	obj1.matWorld.r[3].m128_f32[1],obj1.matWorld.r[3].m128_f32[2] };
		//XMFLOAT3 position2 = { obj2.matWorld.r[3].m128_f32[0],
		//	obj2.matWorld.r[3].m128_f32[1],obj2.matWorld.r[3].m128_f32[2] };

		////2�̍��W�̍�
		//XMVECTOR position_sub = XMVectorSet(
		//	position1.x - position2.x,
		//	position1.y - position2.y,
		//	position1.z - position2.z,
		//	0
		//);

		////2�̍��W�̋���
		//position_sub = XMVector3Length(position_sub);
		//float distance = position_sub.m128_f32[0];

		////���������a�̘a�ȉ��Ȃ瓖�����Ă���
		//if (distance <= radius1 + radius2) {
		//	debugText.Print("Hit", 100, 100, 10.0f);
		//}



#pragma endregion

	DebugText::Print(100, 0, 0);

#pragma region �I�u�W�F�N�g�A�b�v�f�[�g

	obj1.Update();
	
	obj2.Update();

#pragma endregion

}

void GamePlay::Draw()
{
	//�`��J�n
	DX12Init::BeginDraw();

	//�w�i�X�v���C�g�`�悱������
	sprite1.Draw();



	//�w�i�`�悱���܂�
	DX12Init::ClearDepthBuffer();


	//�I�u�W�F�N�g�`�悱������
	obj1.Draw();
	
	obj2.Draw();

	//�I�u�W�F�N�g�`�悱���܂�

	//�O�i�X�v���C�g�`�悱������

	sprite2.Draw();
	DebugText::DrawAll();

	//�O�i�X�v���C�g�`�悱���܂�

	//�`��I��
	DX12Init::EndDraw();
}
