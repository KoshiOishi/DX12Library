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

	//DirectInput �I�u�W�F�N�g�̐���
	IDirectInput8* dinput = nullptr;
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);

	//�L�[�{�[�h�f�o�C�X�̐���
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	//���̓f�[�^�`���̃Z�b�g
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard); //�W���`��

	//�r�����䃌�x���̃Z�b�g
	result = devkeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

}

void Input::Update()
{
	HRESULT result;
	//�L�[�{�[�h���̎擾�J�n
	result = devkeyboard->Acquire();

	//�O��̃L�[���͏����R�s�[
	for (int i = 0; i < 256; i++)
	{
		prevKey[i] = key[i];
	}

	//�S�L�[�̓��͏�Ԃ��擾����
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