#include "Input.h"

BYTE Input::key[256];
BYTE Input::prevKey[256];
IDirectInputDevice8* Input::devkeyboard;

Input::Input()
{

}

Input::~Input()
{

}

void Input::Initialize(HWND hwnd)
{
	HRESULT result;

	//DirectInput オブジェクトの生成
	IDirectInput8* dinput = nullptr;
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);

	//キーボードデバイスの生成
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	//入力データ形式のセット
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard); //標準形式

	//排他制御レベルのセット
	result = devkeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

}

void Input::Update()
{
	HRESULT result;
	//キーボード情報の取得開始
	result = devkeyboard->Acquire();

	//前回のキー入力情報をコピー
	for (int i = 0; i < 256; i++)
	{
		prevKey[i] = key[i];
	}

	//全キーの入力状態を取得する
	result = devkeyboard->GetDeviceState(sizeof(key), key);
}

bool Input::Trigger(int keynum)
{
	if (!prevKey[keynum] && key[keynum])
		return true;

	return false;
}

bool Input::Push(int keynum)
{
	if (keynum < 0x00) return false;
	if (keynum > 0xff) return false;

	if (key[keynum])
		return true;

	return false;
}