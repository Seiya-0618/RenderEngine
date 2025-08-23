#include "App.h"


App::App(uint32_t width, uint32_t height)
	:m_window(nullptr),
	m_renderer(nullptr),
	m_width(width),
	m_height(height)
{
	m_window = new Window(m_width, m_height);
	m_renderer = new DXRenderer();
}

App::~App()
{
	delete m_renderer;
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
	m_renderer->InitD3D(m_window->GetHwnd(), m_width, m_height);

	return true;
}

void App::TermApp()
{
	//Do Nothing
}

void App::MainLoop()
{
	
	while (m_window->CheckMessage())
	{
		if (!m_window->MessageLoop())
		{
			m_renderer->Render();
		}
	}
}