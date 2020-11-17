#include "DebugText.h"
#include <string>

//�ÓI�����o�ϐ��̎���
Sprite DebugText::sprites[maxCharCount];
int DebugText::spriteIndex = 0;
UINT DebugText::debugTextTexNumber;

void DebugText::Initialize()
{
	for (int i = 0; i < _countof(sprites); i++)
	{
		sprites[i].SpriteSetTexNumber(debugTextTexNumber);
		sprites[i].GenerateSprite();
	}
}

void DebugText::SetDebugTextTexNumber(const int debugTextTexNumber)
{
	DebugText::debugTextTexNumber = (UINT)debugTextTexNumber;
}

void DebugText::Print(const std::string& text, float x, float y, float scale)
{
	//���ׂĂ̕����ɂ���
	for (int i = 0; i < text.size(); i++)
	{
		//�ő啶��������
		if (spriteIndex >= maxCharCount) {
			break;
		}

		//1�������o��(��ASCII�R�[�h�ł������藧���Ȃ�)
		const unsigned char& character = text[i];

		//ASCII�R�[�h��2�i����΂����ԍ����v�Z
		int fontIndex = character - 32;
		if (character >= 0x7f) {
			fontIndex = 0;
		}

		int fontIndexY = fontIndex / fontLineCount;
		int fontIndexX = fontIndex % fontLineCount;

		//���W�v�Z
		sprites[spriteIndex].spritePosition = { x + fontWidth * scale * i, y };
		sprites[spriteIndex].SpriteSetDrawRectangle(fontIndexX * fontWidth, fontIndexY * fontHeight, fontWidth, fontHeight);
		sprites[spriteIndex].SpriteSetSize(fontWidth * scale, fontHeight * scale);

		//������1�i�߂�
		spriteIndex++;

	}
}

void DebugText::Print(int text, float x, float y, float scale)
{
	Print(std::to_string(text), x, y, scale);
}

void DebugText::Print(float text, float x, float y, float scale)
{
	Print(std::to_string(text), x, y, scale);
}

void DebugText::DrawAll()
{
	//���ׂĂ̕����̃X�v���C�g�ɂ���
	for (int i = 0; i < spriteIndex; i++)
	{
		//�X�v���C�g�`��
		sprites[i].SpriteDraw();
	}

	spriteIndex = 0;
}