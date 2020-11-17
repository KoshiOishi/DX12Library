#include "DebugText.h"
#include <string>

//静的メンバ変数の実体
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
	//すべての文字について
	for (int i = 0; i < text.size(); i++)
	{
		//最大文字数超過
		if (spriteIndex >= maxCharCount) {
			break;
		}

		//1文字取り出す(※ASCIIコードでしか成り立たない)
		const unsigned char& character = text[i];

		//ASCIIコードの2段分飛ばした番号を計算
		int fontIndex = character - 32;
		if (character >= 0x7f) {
			fontIndex = 0;
		}

		int fontIndexY = fontIndex / fontLineCount;
		int fontIndexX = fontIndex % fontLineCount;

		//座標計算
		sprites[spriteIndex].spritePosition = { x + fontWidth * scale * i, y };
		sprites[spriteIndex].SpriteSetDrawRectangle(fontIndexX * fontWidth, fontIndexY * fontHeight, fontWidth, fontHeight);
		sprites[spriteIndex].SpriteSetSize(fontWidth * scale, fontHeight * scale);

		//文字を1つ進める
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
	//すべての文字のスプライトについて
	for (int i = 0; i < spriteIndex; i++)
	{
		//スプライト描画
		sprites[i].SpriteDraw();
	}

	spriteIndex = 0;
}