#pragma once

//#include <Windows.h>
#include <cstdint>
#include "Window.h"
#include "DXRenderer.h"
#include "Scene.h"

class App
{
public:
	//method
	App(uint32_t width, uint32_t height);
	~App();
	void Run();
	uint32_t m_width;
	uint32_t m_height;

	std::vector<Scene*> scenes;

private:
	Window* m_window;
	DXRenderer* m_renderer;
	Scene* mainScene;
	bool InitApp();
	void TermApp();
	void MainLoop();
};