#pragma once

//#include <Windows.h>
#include <cstdint>
#include "Window.h"

class App
{
public:
	//method
	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	Window* m_window;

	bool InitApp();
	void TermApp();
	void MainLoop();
};