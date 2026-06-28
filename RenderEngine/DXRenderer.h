#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "Object_win.h"
#include "Scene.h"
#include "FileUtil.h"
#include "ResourceUploadBatch.h"
#include "DDSTextureLoader.h"
#include "VertexTypes.h"
#include <cassert>
#include <DirectXTex.h>
#include "PipelineKey.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

static const uint32_t FrameCount = 2;

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

struct CBV_data
{
	ComPtr<ID3D12Resource> Buffer;
	void* mappedBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;

};


class DXRenderer
{
public:
	DXRenderer(uint32_t width, uint32_t height, Scene* scene);
	~DXRenderer();
	bool InitD3D(HWND hwnd);
	bool OnInit();
	bool CreatePipelineStateObject();
	bool CreateBasicPSO();
	bool CreateLambertPSO();
	bool CreatePhongPSO();
	bool CreateConstantBuffer(UINT CBSize, uint32_t slotIdx, CBV_data(&cbvData)[FrameCount]);
	bool CreateObjectConstantBuffer(Object* obj, UINT cbvSlotIndex);
	void Render();
	void TermD3D();
	void WaitGpu();
	void Present(uint32_t interval);
	void UpdateObjectConstants();
	Scene* m_Scene;

	ID3D12Device* GetDevice() const { return m_pDevice.Get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return m_pQueue.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return m_pCmdList.Get(); }
	ID3D12DescriptorHeap* GetCBV_SRV_UAVHeap() const { return m_pHeapCBV_SRV_UAV.Get(); }

private:
	//static const uint32_t FrameCount = 2;
	static const uint32_t maxCBVCount = 1000;
	static const uint32_t maxTextureCount = 100;



	ComPtr<ID3D12Device> m_pDevice;
	ComPtr<ID3D12CommandQueue> m_pQueue;
	ComPtr<IDXGISwapChain3> m_pSwapChain;
	ComPtr<ID3D12Resource> m_pColorBuffer[FrameCount];
	ComPtr<ID3D12Resource> m_pDepthBuffer;
	ComPtr<ID3D12DescriptorHeap> m_pHeapDSV;
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;
	ComPtr<ID3D12Fence> m_pFence;

	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV_SRV_UAV;
	ComPtr<ID3D12Resource> m_pVB;              //頂点バッファ
	ComPtr<ID3D12Resource> m_pCB[FrameCount];  //定数バッファ
	ComPtr<ID3D12RootSignature> m_pRootSignature;
	ComPtr<ID3D12PipelineState> m_pPSO;
	std::unordered_map<PipelineKey ,ComPtr<ID3D12PipelineState>> m_PSOMap;

	HANDLE m_FenceEvent;
	uint64_t m_FenceCounter[FrameCount];
	uint32_t m_FrameIndex = 0;
	UINT m_cbvSlotIndex = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleDSV;
	D3D12_VERTEX_BUFFER_VIEW m_VBV;
	D3D12_DEPTH_STENCIL_DESC m_DSState;
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_Scissor;
	//ConstantBufferView<ObjectConstants> m_CBV[FrameCount];
	float m_RotateAngle;
	uint32_t m_Width;
	uint32_t m_Height;

	Texture m_Texture;

};