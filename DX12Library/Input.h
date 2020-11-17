#pragma once
#include <Windows.h>

#define DIRECTINPUT_VERSION	0x0800	//DirectInputのバージョン指定
#include <dinput.h>

class Input
{
private:
	static BYTE key[256];
	static BYTE prevKey[256];
	static IDirectInputDevice8* devkeyboard;

public:
	//メンバ関数の宣言
	Input();
	~Input();
	//初期化
	static void Initialize(HWND hwnd);
	//毎フレーム処理
	static void Update();

	static bool Trigger(int keynum);
	static bool Push(int keynum);
};