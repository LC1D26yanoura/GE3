#include "Input.h"
#include<cassert>
#include <wrl.h>
#include <dinput.h>
#define DIRCTINPUT_VERSION 0x0800
#pragma comment(lib, "dinput.lib")
#pragma comment(lib, "dxguid.lib")
using namespace Microsoft::WRL;

void Input::Initialize(HINSTANCE hInstance,HWND hwnd) {
	HRESULT result;

//DirectInputのインスタンス
	ComPtr<IDirectInput8> directInput = nullptr;
result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));
	//キーボードデバイス生成
	ComPtr<IDirectInputDevice8> Keyboard;
	result = directInput->CreateDevice(GUID_SysKeyboard, &Keyboard, NULL);
	assert(SUCCEEDED(result));
	//入力データ形式のリセット
	result = Keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	//排他制御レベルのセット
	result = Keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Update() {}
