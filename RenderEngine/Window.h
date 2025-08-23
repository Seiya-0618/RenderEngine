#pragma once

#include <Windows.h>
#include <cstdint>

class Window
{
public:
	//method
	Window(uint32_t width, uint32_t height);
	~Window();
	void Show();
	bool MessageLoop();
	bool CheckMessage();
	HWND GetHwnd();

private:
	HINSTANCE m_hInst;
	HWND m_hwnd;
	uint32_t m_width;
	uint32_t m_height;
	MSG msg;


	bool InitWnd();
	void TermWnd();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
};