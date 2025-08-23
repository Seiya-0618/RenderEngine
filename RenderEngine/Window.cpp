#include "Window.h"

const auto ClassName = TEXT("RenderWindowClass");

Window::Window(uint32_t width, uint32_t height)
	: m_hInst(nullptr),
	m_hwnd(nullptr),
	m_width(width),
	m_height(height)
{
	InitWnd();
	/* do nothing */
}

Window::~Window()
{
	TermWnd();
}

bool Window::InitWnd()
{

	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr)
	{
		return false;
	}
	
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	m_hInst = hInst;
	RECT rc = {};
	rc.right = static_cast<LONG>(m_width);
	rc.bottom = static_cast<LONG>(m_height);

	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	m_hwnd = CreateWindowEx(
		0,
		ClassName,
		TEXT("Renderer"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		m_hInst,
		nullptr);

	if (m_hwnd == nullptr)
	{
		return false;
	}
	msg = {};
		
}

void Window::Show()
{
	if (m_hwnd)
	{
		ShowWindow(m_hwnd, SW_SHOWNORMAL);
		UpdateWindow(m_hwnd);
		SetFocus(m_hwnd);
	}
}

bool Window::MessageLoop()
{
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		return true;
	}
	else {
		return false;
	}
}

bool Window::CheckMessage()
{
	bool result = WM_QUIT != msg.message;
	return result;
}


void Window::TermWnd()
{
	if (m_hInst != nullptr)
	{
		UnregisterClass(ClassName, m_hInst);
	}
	m_hInst = nullptr;
	m_hwnd = nullptr;
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}
	break;

	default:
	{/* Do Nothing */ }
	break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

HWND Window::GetHwnd()
{
	if (m_hwnd == nullptr)
	{
		if (!InitWnd())
		{
			return nullptr;
		}
	}

	return m_hwnd;
}