#pragma once
#include <Windows.h>
#include <cstdint>
class WinApp
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;
public:
	void Initialize();

	void Update();

	void Finalize();
	//getter
	HWND GetHwnd() const { return hwnd; }
	HINSTANCE GetHinstance() const { return wc.hInstance; }
	//メッセージの処理
	bool ProcessMessage();
private:
	//ウィンドウハンドル
	HWND hwnd = nullptr;
	//
	WNDCLASS wc{};
};

