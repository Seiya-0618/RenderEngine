#include "App.h"


App::App(uint32_t width, uint32_t height)
	:m_window(nullptr),
	m_renderer(nullptr),
	m_width(width),
	m_height(height),
	mainScene(nullptr)
{
	m_window = new Window(m_width, m_height);
	mainScene = new Scene(static_cast<float>(m_width), static_cast<float>(m_height), 0.1f, 100.0f);
	scenes.push_back(mainScene);
	m_renderer = new DXRenderer(m_width, m_height, mainScene);
	m_resourceManager = nullptr;
}

App::~App()
{
	delete mainScene;
	delete m_resourceManager;
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
	m_renderer->InitD3D(m_window->GetHwnd());
	m_resourceManager = new ResourceManager(
		m_renderer->GetDevice(),
		m_renderer->GetCBV_SRV_UAVHeap(),
		m_renderer->GetCommandQueue(),
		mainScene
	);
	const wchar_t* modelPath = L"SampleObj/Sample.obj";
	m_resourceManager->LoadModel(modelPath);
	m_renderer->OnInit();


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
			m_renderer->UpdateObjects();
			m_renderer->Render();
		}
	}
}