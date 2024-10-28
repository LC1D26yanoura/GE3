#pragma once
#include <Windows.h>
#include <wrl.h>
#define DIRCTINPUT_VERSION 0x0800
#include <dinput.h>
class Input {
public:
	//namespace省略
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	//初期化
	void Initialize(HINSTANCE hInstance,HWND hwnd);
	//更新
	void Update();

private:
	//キーボードのデバイス
	ComPtr<IDirectInput8> directInput;
	ComPtr<IDirectInputDevice8> Keyboard;
};
