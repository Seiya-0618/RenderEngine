#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class DXRenderer
{
public:
	DXRenderer();
	~DXRenderer();
	bool InitD3D(HWND hwnd, uint32_t width, uint32_t height);
	void Render();
	void TermD3D();
	void WaitGpu();
	void Present(uint32_t interval);


private:
	static const uint32_t FrameCount = 2;

	ID3D12Device* m_pDevice;
	ID3D12CommandQueue* m_pQueue;
	IDXGISwapChain3* m_pSwapChain;
	ID3D12Resource* m_pColorBuffer[FrameCount];
	ID3D12CommandAllocator* m_pCmdAllocator[FrameCount];
	ID3D12GraphicsCommandList* m_pCmdList;
	ID3D12DescriptorHeap* m_pHeapRTV;
	ID3D12Fence* m_pFence;
	HANDLE m_FenceEvent;
	uint64_t m_FenceCounter[FrameCount];
	uint32_t m_FrameIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];

};