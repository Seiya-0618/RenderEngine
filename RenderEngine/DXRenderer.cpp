#include "DXRenderer.h"
#include <iostream>
#include <cassert>


template<typename T>
void SafeRelease(T*& ptr)
{
	if (ptr != nullptr)
	{
		ptr->Release();
		ptr = nullptr;
	}
}

DXRenderer::DXRenderer()
	:m_pDevice(nullptr),
	m_pQueue(nullptr),
	m_pSwapChain(nullptr),
	m_pColorBuffer{},
	m_pCmdAllocator{},
	m_pCmdList(nullptr),
	m_pHeapRTV(nullptr),
	m_pFence(nullptr)
{
	/* Do Nothing */
}

DXRenderer::~DXRenderer()
{
	/* Do Nothing */
}

bool DXRenderer::InitD3D(HWND hwnd, uint32_t width, uint32_t height)
{
	auto hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_pDevice));
	if (FAILED(hr))
	{
		return false;
	}

	{//Command Queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.NodeMask = 0;
		hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pQueue));
		if (FAILED(hr))
		{
			return false;
		}
	}

	{//SwapChain
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
		{
			return false;
		}

		DXGI_SWAP_CHAIN_DESC swapDesc = {};
		swapDesc.BufferDesc.Width = width;
		swapDesc.BufferDesc.Height = height;
		swapDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.BufferCount = FrameCount;
		swapDesc.OutputWindow = hwnd;
		swapDesc.Windowed = TRUE;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue, &swapDesc, &pSwapChain);
		if (FAILED(hr))
		{
			SafeRelease(pFactory);
			return false;
		}


		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		if (FAILED(hr))
		{
			SafeRelease(pFactory);
			SafeRelease(pSwapChain);
			std::cout << "Failed to query IDXGISwapChain3 interface." <<  std::endl;
			return false;
		}

		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
		SafeRelease(pFactory);
		SafeRelease(pSwapChain);
	}
	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			hr = m_pDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&m_pCmdAllocator[i]));
			if (FAILED(hr))
			{
				return false;
			}
		}
	}
	{
		hr = m_pDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_pCmdAllocator[m_FrameIndex],
			nullptr,
			IID_PPV_ARGS(&m_pCmdList));
		if (FAILED(hr))
		{
			return false;
		}
	}

	{
		//Create RTV
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = FrameCount;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;

		hr = m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pHeapRTV));
		if (FAILED(hr))
		{
			std::cout << "Failed to create RTV descriptor heap." << std::abort;
			return false;
		}

		auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for (auto i = 0u; i < FrameCount; ++i)
		{
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffer[i]));
			if (FAILED(hr))
			{
				return false;
			}
			D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = 0;
			viewDesc.Texture2D.PlaneSlice = 0;

			m_pDevice->CreateRenderTargetView(
				m_pColorBuffer[i], &viewDesc, handle
			);

			m_HandleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			m_FenceCounter[i] = 0;
		}

		hr = m_pDevice->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_pFence));
		if (FAILED(hr))
		{
			return false;
		}
		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_FenceEvent == nullptr)
		{
			return false;
		}
	}

	m_pCmdList->Close();

	return true;

}

void DXRenderer::Render()
{
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex], nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_pCmdList->ResourceBarrier(1, &barrier);
	m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, nullptr);

	float clearColor[] = { 0.25f, 0.25f, 0.75f, 1.0f };
	m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);
	{
		//Polygon lender process
	}

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_pCmdList->ResourceBarrier(1, &barrier);
	m_pCmdList->Close();

	ID3D12CommandList* ppCmdLists[] = { m_pCmdList };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	Present(1);
}

void DXRenderer::Present(uint32_t interval) {
	m_pSwapChain->Present(interval, 0);
	const auto currentValue = m_FenceCounter[m_FrameIndex];
	m_pQueue->Signal(m_pFence, currentValue);
	
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex])
	{
		m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}

	m_FenceCounter[m_FrameIndex] = currentValue + 1;
}

void DXRenderer::WaitGpu()
{
	assert(m_pQueue != nullptr);
	assert(m_pFence != nullptr);
	assert(m_FenceEvent != nullptr);

	m_pQueue->Signal(m_pFence, m_FenceCounter[m_FrameIndex]);
	m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);

	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	m_FenceCounter[m_FrameIndex]++;
}

void DXRenderer::TermD3D()
{
	WaitGpu();
	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}

	SafeRelease(m_pFence);

	SafeRelease(m_pHeapRTV);
	for (auto i = 0u; i < FrameCount; ++i)
	{
		SafeRelease(m_pColorBuffer[i]);
	}

	SafeRelease(m_pCmdList);

	for (auto i = 0u; i < FrameCount; ++i)
	{
		SafeRelease(m_pCmdAllocator[i]);
	}

	SafeRelease(m_pSwapChain);
	SafeRelease(m_pQueue);

	SafeRelease(m_pDevice);
}

