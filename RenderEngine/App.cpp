#include "App.h"

App::App(uint32_t width, uint32_t height)
	:m_window(nullptr)
{
	m_window = new Window(width, height);
}

App::~App()
{
    delete m_window;
}

void App::Run()
{
	if (InitApp())
	{
		MainLoop();
	}
	TermApp();
}

bool App::InitApp()
{
	if (!m_window || !m_window->GetHwnd())
	{
		return false;
	}
	m_window->Show();

	return true;
}

void App::TermApp()
{
	//Do Nothing
}

void App::MainLoop()
{
	m_window->MessageLoop();
}