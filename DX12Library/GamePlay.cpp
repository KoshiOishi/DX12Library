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

	//�f�o�b�O�e�L�X�g�p�̃e�N�X�`���ԍ����w��
	DebugText::SetDebugTextTexNumber(0);
	//�f�o�b�O�e�L�X�g�p�̃e�N�X�`���ǂݍ���
	Sprite::SpriteLoadTexture(DebugText::GetDebugTextTexNumber(), L"Resources/debugfont.png");
	//�f�o�b�O�e�L�X�g������
	DebugText::Initialize();

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
			obj1[0].AddPosition(0, 1.0f, 0);

		}
		else if (Input::Push(DIK_S))
		{
			obj1[0].AddPosition(0, -1.0f, 0);

		}
	}
	if (Input::Push(DIK_A) || Input::Push(DIK_D))
	{
		//���W���ړ����鏈��(X���W)
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
		//���W���ړ����鏈��(Z���W)
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

	for (int i = 0; i < 5; i++)
	{
		obj1[i].Update();
	}
	obj2.Update();

#pragma endregion

}

void GamePlay::Draw()
{
	//�`��J�n
	DX12Init::BeginDraw();

	//�w�i�X�v���C�g�`�悱������
	sprite1.SpriteDraw();



	//�w�i�`�悱���܂�
	DX12Init::ClearDepthBuffer();


	//�I�u�W�F�N�g�`�悱������
	for (int i = 0; i < 5; i++)
	{
		obj1[i].Draw();
	}
	obj2.Draw();

	//�I�u�W�F�N�g�`�悱���܂�

	//�O�i�X�v���C�g�`�悱������

	sprite2.SpriteDraw();
	DebugText::DrawAll();

	//�O�i�X�v���C�g�`�悱���܂�

	//�`��I��
	DX12Init::EndDraw();
}
