#pragma once
#include <Windows.h>
#include <wrl.h>
#define DIRCTINPUT_VERSION 0x0800
#include <dinput.h>
#include "WinApp.h"
class Input {
public:
	// namespace省略
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// 初期化
	void Initialize(WinApp*winApp);
	// 更新
	void Update();
	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	///< param name="keyNumber">キー番号(DIK_0等)</param>
	///< returns>押されているか</returns>
	bool PushKey(BYTE KeyNumber);
	/// <summary>
	/// キーのトリガーチェック
	/// </summary>
	/// <param name=<"KeyNumber">キー番号(DIK_0等)</param>
	/// <returns>トリガーか</returns>
	bool TriggerKey(BYTE keyNumber);

private:
	// キーボードのデバイス
	ComPtr<IDirectInputDevice8> Keyboard;
	// 全キーの状態
	BYTE Key[256] = {};
	// 前回のキーの状態
	BYTE KeyPre[256] = {};
	//WindowsAPI
	WinApp* winApp = nullptr;
	ComPtr<IDirectInput8> directInput;
};
