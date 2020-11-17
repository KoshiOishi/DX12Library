#pragma once
#include <Windows.h>

#define DIRECTINPUT_VERSION	0x0800	//DirectInput�̃o�[�W�����w��
#include <dinput.h>

class Input
{
private:
	static BYTE key[256];
	static BYTE prevKey[256];
	static IDirectInputDevice8* devkeyboard;

public:
	//�����o�֐��̐錾
	Input();
	~Input();
	//������
	static void Initialize(HWND hwnd);
	//���t���[������
	static void Update();

	static bool Trigger(int keynum);
	static bool Push(int keynum);
};