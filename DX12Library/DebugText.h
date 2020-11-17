#pragma once
#include "Sprite.h"
#include <string>

class DebugText
{
public:
	static const int maxCharCount = 256;	//最大文字数
	static const int fontWidth = 9;			//フォント画像内1文字分の横幅
	static const int fontHeight = 18;		//フォント画像内1文字分の縦幅
	static const int fontLineCount = 14;	//フォント画像内1行分の文字列
	static UINT debugTextTexNumber;

	static void Initialize();
	static void SetDebugTextTexNumber(const int debugTextTexNumber);

	/// <summary>
	/// デバッグテキストを出力
	/// </summary>
	/// <param name="text">テキスト</param>
	/// <param name="x">x座標</param>
	/// <param name="y">y座標</param>
	/// <param name="scale">スケール</param>
	static void Print(const std::string& text, float x, float y, float scale = 1.0f);

	/// <summary>
	/// デバッグテキストを出力
	/// </summary>
	/// <param name="text">整数</param>
	/// <param name="x">x座標</param>
	/// <param name="y">y座標</param>
	/// <param name="scale">スケール</param>
	static void Print(int text, float x, float y, float scale = 1.0f);

	/// <summary>
	/// デバッグテキストを出力
	/// </summary>
	/// <param name="text">数値(float)</param>
	/// <param name="x">x座標</param>
	/// <param name="y">y座標</param>
	/// <param name="scale">スケール</param>
	static void Print(float text, float x, float y, float scale = 1.0f);

	/// <summary>
	/// デバッグテキストを描画
	/// </summary>
	static void DrawAll();

	static UINT GetDebugTextTexNumber() { return debugTextTexNumber; }

private:
	static Sprite sprites[maxCharCount];
	static int spriteIndex;
};

