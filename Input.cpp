#include "Input.h"
#include<cassert>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(WinApp* winApp) {
	HRESULT result;

	//DirectInputのインスタンス
	directInput;
	result = DirectInput8Create(winApp->GetInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));
	//キーボードデバイス生成

	result = directInput->CreateDevice(GUID_SysKeyboard, &Keyboard, NULL);
	assert(SUCCEEDED(result));
	//入力データ形式のリセット
	result = Keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	//排他制御レベルのセット
	result = Keyboard->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	//借りてきたWinAppのインスタンスを記録
	this->winApp = winApp;
}

void Input::Update() {
	HRESULT result;

	// 前回のキー入力を保存
	memcpy(KeyPre, Key, sizeof(Key));
	//キーボード情報の取得開始
	result = Keyboard->Acquire();
	//全キーの入力情報を取得する

	result = Keyboard->GetDeviceState(sizeof(Key), Key);
}

bool Input::PushKey(BYTE KeyNumber) {
	if (Key[KeyNumber]) {
		return true;
	}return false;
}

bool Input::TriggerKey(BYTE keyNumber) {
	if (KeyPre[keyNumber] == 0 && Key[keyNumber] != 0) {
		return true;
	}
	return false;
}
