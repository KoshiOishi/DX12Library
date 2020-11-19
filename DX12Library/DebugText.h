#pragma once
#include "Sprite.h"
#include <string>

class DebugText
{
public:
	static const int maxCharCount = 256;	//�ő啶����
	static const int fontWidth = 9;			//�t�H���g�摜��1�������̉���
	static const int fontHeight = 18;		//�t�H���g�摜��1�������̏c��
	static const int fontLineCount = 14;	//�t�H���g�摜��1�s���̕�����
	static UINT debugTextTexNumber;

	/// <summary>
	/// �f�o�b�O�e�L�X�g�̏���������
	/// </summary>
	/// <param name="debugTextTexNumber">�f�o�b�O�e�L�X�g�Ɏg�p����X�v���C�g�e�N�X�`���ԍ�</param>
	/// <param name="texfilename">�f�o�b�O�e�L�X�g�Ɏg�p����e�N�X�`���t�@�C���p�X</param>
	static void Initialize(UINT debugTextTexNumber, const wchar_t* texfilename);
	static void SetDebugTextTexNumber(const int debugTextTexNumber);

	/// <summary>
	/// �f�o�b�O�e�L�X�g���o��
	/// </summary>
	/// <param name="text">�e�L�X�g</param>
	/// <param name="x">x���W</param>
	/// <param name="y">y���W</param>
	/// <param name="scale">�X�P�[��</param>
	static void Print(const std::string& text, float x, float y, float scale = 1.0f);

	/// <summary>
	/// �f�o�b�O�e�L�X�g���o��
	/// </summary>
	/// <param name="text">����</param>
	/// <param name="x">x���W</param>
	/// <param name="y">y���W</param>
	/// <param name="scale">�X�P�[��</param>
	static void Print(int text, float x, float y, float scale = 1.0f);

	/// <summary>
	/// �f�o�b�O�e�L�X�g���o��
	/// </summary>
	/// <param name="text">���l(float)</param>
	/// <param name="x">x���W</param>
	/// <param name="y">y���W</param>
	/// <param name="scale">�X�P�[��</param>
	static void Print(float text, float x, float y, float scale = 1.0f);

	/// <summary>
	/// �f�o�b�O�e�L�X�g��`��
	/// </summary>
	static void DrawAll();

	static UINT GetDebugTextTexNumber() { return debugTextTexNumber; }

private:
	static Sprite sprites[maxCharCount];
	static int spriteIndex;
};

