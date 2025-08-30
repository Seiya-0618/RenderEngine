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

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
};

DXRenderer::DXRenderer(uint32_t width, uint32_t height)
	:m_pDevice(nullptr),
	m_pQueue(nullptr),
	m_pSwapChain(nullptr),
	m_pColorBuffer{},
	m_pCmdAllocator{},
	m_pCmdList(nullptr),
	m_pHeapRTV(nullptr),
	m_pFence(nullptr),
	m_pHeapCBV(nullptr),
	m_pVB(nullptr),
	m_pCB{},
	m_pRootSignature(nullptr),
	m_pPSO(nullptr),
	m_Width(width),
	m_Height(height)

{
	/* Do Nothing */
}

DXRenderer::~DXRenderer()
{
	/* Do Nothing */
}

bool DXRenderer::InitD3D(HWND hwnd)
{
	auto hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(m_pDevice.GetAddressOf()));
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
		hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_pQueue.GetAddressOf()));
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
		swapDesc.BufferDesc.Width = m_Width;
		swapDesc.BufferDesc.Height = m_Height;
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
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &swapDesc, &pSwapChain);
		if (FAILED(hr))
		{
			SafeRelease(pFactory);
			return false;
		}


		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(m_pSwapChain.GetAddressOf()));
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
				IID_PPV_ARGS(m_pCmdAllocator[i].GetAddressOf()));
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
			m_pCmdAllocator[m_FrameIndex].Get(),
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

		hr = m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pHeapRTV.GetAddressOf()));
		if (FAILED(hr))
		{
			std::cout << "Failed to create RTV descriptor heap." << std::abort;
			return false;
		}

		auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for (auto i = 0u; i < FrameCount; ++i)
		{
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pColorBuffer[i].GetAddressOf()));
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
				m_pColorBuffer[i].Get(), &viewDesc, handle
			);

			m_HandleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	{//Create Stencil and Depth Buffer
		D3D12_HEAP_PROPERTIES stencilProps = {};
		stencilProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		stencilProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		stencilProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		stencilProps.CreationNodeMask = 1;
		stencilProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC stencilDesc = {};
		stencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		stencilDesc.Alignment = 0;
		stencilDesc.Width = m_Width;
		stencilDesc.Height = m_Height;
		stencilDesc.DepthOrArraySize = 1;
		stencilDesc.MipLevels = 1;
		stencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		stencilDesc.SampleDesc.Count = 1;
		stencilDesc.SampleDesc.Quality = 0;
		stencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		stencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		hr = m_pDevice->CreateCommittedResource(
			&stencilProps,
			D3D12_HEAP_FLAG_NONE,
			&stencilDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(m_pDepthBuffer.GetAddressOf()));;
		if (FAILED(hr))
		{
			std::cout << "Failed to create depth stencil buffer." << std::endl;
			return false;
		}

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;

		hr = m_pDevice->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(m_pHeapDSV.GetAddressOf()));
		if (FAILED(hr))
		{
			std::cout << "Failed to create DSV descriptor heap." << std::endl;
			return false;
		}

		auto handle = m_pHeapDSV->GetCPUDescriptorHandleForHeapStart();
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
		viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_pDevice->CreateDepthStencilView(
			m_pDepthBuffer.Get(),
			&viewDesc,
			handle);
		m_HandleDSV = handle;
	}

	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			m_FenceCounter[i] = 0;
		}

		hr = m_pDevice->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(m_pFence.GetAddressOf()));
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

bool DXRenderer::OnInit()
{
	{
		Vertex vertices[] = {
			{DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
			{DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
			{DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
		};

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Alignment = 0;
		resDesc.Width = sizeof(vertices);
		resDesc.Height = 1;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;
		resDesc.Format = DXGI_FORMAT_UNKNOWN;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto hr = m_pDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pVB.GetAddressOf()));
		if (FAILED(hr))
		{
			std::cout << "Failed to create vertex buffer.HRESULT: 0x" << std::hex << hr << std::endl;
			return false;
		}

		void* ptr = nullptr;
		hr = m_pVB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			std::cout << "Failed to map vertex buffer." << std::endl;
			return false;
		}

		memcpy(ptr, vertices, sizeof(vertices));

		m_pVB->Unmap(0, nullptr);


		m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
		m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));
		m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1 * FrameCount;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.NodeMask = 0;

		auto hr = m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pHeapCBV.GetAddressOf()));
		if (FAILED(hr))
		{
			std::cout << "Failed to create CBV descriptor heap." << std::hex << hr <<  std::endl;
			return false;
		}
	}

	{
		D3D12_HEAP_PROPERTIES heapprops = {};
		heapprops.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapprops.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprops.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapprops.CreationNodeMask = 1;
		heapprops.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Alignment = 0;
		resDesc.Width = sizeof(Transform);
		resDesc.Height = 1;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;
		resDesc.Format = DXGI_FORMAT_UNKNOWN;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		for (auto i = 0; i < FrameCount; ++i)
		{
			auto hr = m_pDevice->CreateCommittedResource(
				&heapprops,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_pCB[i].GetAddressOf()));
			if (FAILED(hr))
			{
				std::cout << "Failed to create constant buffer. HResult : 0x" << std::endl;
				return false;
			}
			auto address = m_pCB[i]->GetGPUVirtualAddress();
			auto handleCPU = m_pHeapCBV->GetCPUDescriptorHandleForHeapStart();
			auto handleGPU = m_pHeapCBV->GetGPUDescriptorHandleForHeapStart();
			handleCPU.ptr += i * incrementSize;
			handleGPU.ptr += i * incrementSize;

			m_CBV[i].HandleCPU = handleCPU;
			m_CBV[i].HandleGPU = handleGPU;
			m_CBV[i].Desc.BufferLocation = address;
			m_CBV[i].Desc.SizeInBytes = sizeof(Transform);

			m_pDevice->CreateConstantBufferView(&m_CBV[i].Desc, handleCPU);
			hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBV[i].pBuffer));
			if (FAILED(hr))
			{
				std::cout << "Failed to map constant buffer." << std::endl;
				return false;
			}

			auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
			auto targetPos = DirectX::XMVectorZero();
			auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			auto fovY = DirectX::XMConvertToRadians(37.5f);
			auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);			std::cout << "Aspect: " << aspect << std::endl;

			m_CBV[i].pBuffer->World = DirectX::XMMatrixIdentity();
			m_CBV[i].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
			m_CBV[i].pBuffer->Projection = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 100.0f);
		}
	}

	
		//Create Depth Stencil State
		D3D12_DEPTH_STENCIL_DESC depthDesc = {};
		depthDesc.DepthEnable = TRUE;
		depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		depthDesc.StencilEnable = FALSE;
	

	{
		auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		D3D12_ROOT_PARAMETER rootParam = {};
		rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParam.Descriptor.ShaderRegister = 0;
		rootParam.Descriptor.RegisterSpace = 0;
		rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
		rootDesc.NumParameters = 1;
		rootDesc.NumStaticSamplers = 0;
		rootDesc.pParameters = &rootParam;
		rootDesc.pStaticSamplers = nullptr;
		rootDesc.Flags = flag;

		ComPtr<ID3DBlob> pBlob;
		ComPtr<ID3DBlob> pErrorBlob;

		auto hr = D3D12SerializeRootSignature(
			&rootDesc,
			D3D_ROOT_SIGNATURE_VERSION_1_0,
			pBlob.GetAddressOf(),
			pErrorBlob.GetAddressOf());
		if (FAILED(hr))
		{
			if (pErrorBlob != nullptr)
			{
				std::cout << static_cast<const char*>(pErrorBlob->GetBufferPointer()) << std::endl;
			}
			
			std::cout << "Failed to selialize root signature." << std::endl;
			return false;
		}

		hr = m_pDevice->CreateRootSignature(
			0,
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			IID_PPV_ARGS(m_pRootSignature.GetAddressOf()));
		if (FAILED(hr))
		{
			std::cout << "Failed to create Root Signature" << std::endl;
			return false;
		}
	}

	{
		//Create Pipeline State Object

		D3D12_INPUT_ELEMENT_DESC elements[2] = {};
		elements[0].SemanticName = "POSITION";
		elements[0].SemanticIndex = 0;
		elements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		elements[0].InputSlot = 0;
		elements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		elements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elements[0].InstanceDataStepRate = 0;

		elements[1].SemanticName = "COLOR";
		elements[1].SemanticIndex = 0;
		elements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elements[1].InputSlot = 0;
		elements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		elements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elements[1].InstanceDataStepRate = 0;

		D3D12_RASTERIZER_DESC rastDesc = {};
		rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rastDesc.CullMode = D3D12_CULL_MODE_NONE;
		rastDesc.FrontCounterClockwise = FALSE;
		rastDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rastDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rastDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rastDesc.DepthClipEnable = FALSE;
		rastDesc.MultisampleEnable = FALSE;
		rastDesc.AntialiasedLineEnable = FALSE;
		rastDesc.ForcedSampleCount = 0;
		rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {
			FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL };

		D3D12_BLEND_DESC descBS;
		descBS.AlphaToCoverageEnable = FALSE;
		descBS.IndependentBlendEnable = FALSE;
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		{
			descBS.RenderTarget[i] = blendDesc;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		auto hr = D3DReadFileToBlob(L"SimpleVS.cso", pVSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			std::cout << "Failed to load vertex shader." << std::endl;
			return false;
		}
		hr = D3DReadFileToBlob(L"SimplePS.cso", pPSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			std::cout << "Failed to load pixerl shader." << std::endl;
			return false;
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC descPSO = {};
		descPSO.InputLayout = { elements, _countof(elements) };
		descPSO.pRootSignature = m_pRootSignature.Get();
		descPSO.VS = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };
		descPSO.PS = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };
		descPSO.RasterizerState = rastDesc;
		descPSO.BlendState = descBS;
		descPSO.DepthStencilState = depthDesc;
		descPSO.DepthStencilState.DepthEnable = TRUE;
		descPSO.DepthStencilState.StencilEnable = FALSE;
		descPSO.SampleMask = UINT_MAX;
		descPSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		descPSO.NumRenderTargets = 1;
		descPSO.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		descPSO.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		descPSO.SampleDesc.Count = 1;
		descPSO.SampleDesc.Quality = 0;

		hr = m_pDevice->CreateGraphicsPipelineState(&descPSO, IID_PPV_ARGS(m_pPSO.GetAddressOf()));
		if (FAILED(hr))
		{
			std::cout << "Failed to create Pipeline State Object." << std::endl;
			return false;
		}
	}

	{
		//Viewport and ScissorRect
		m_Viewport.TopLeftX = 0;
		m_Viewport.TopLeftY = 0;
		m_Viewport.Width = static_cast<float>(m_Width);
		m_Viewport.Height = static_cast<float>(m_Height);
		m_Viewport.MinDepth = 0.0f;
		m_Viewport.MaxDepth = 1.0f;

		m_Scissor.left = 0;
		m_Scissor.right = m_Width;
		m_Scissor.top = 0;
		m_Scissor.bottom = m_Height;
	}

	std::cout << "Finish OnInit()" << std::endl;

	return true;
}

void DXRenderer::Render()
{
	m_RotateAngle += 0.025f;
	m_CBV[m_FrameIndex].pBuffer->World = DirectX::XMMatrixRotationY(m_RotateAngle);

	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex].Get(), nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_pCmdList->ResourceBarrier(1, &barrier);
	m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, &m_HandleDSV);

	float clearColor[] = { 0.25f, 0.25f, 0.75f, 1.0f };
	m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);
	m_pCmdList->ClearDepthStencilView(m_HandleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	{
		//Polygon lender process
		m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
		m_pCmdList->SetDescriptorHeaps(1, m_pHeapCBV.GetAddressOf());
		m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_FrameIndex].Desc.BufferLocation);
		m_pCmdList->SetPipelineState(m_pPSO.Get());

		m_pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pCmdList->IASetVertexBuffers(0, 1, &m_VBV);
		m_pCmdList->RSSetViewports(1, &m_Viewport);
		m_pCmdList->RSSetScissorRects(1, &m_Scissor);

		m_pCmdList->DrawInstanced(3, 1, 0, 0);
	}

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_pCmdList->ResourceBarrier(1, &barrier);
	m_pCmdList->Close();

	ID3D12CommandList* ppCmdLists[] = { m_pCmdList.Get()};
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	Present(1);
}

void DXRenderer::Present(uint32_t interval) {
	m_pSwapChain->Present(interval, 0);
	const auto currentValue = m_FenceCounter[m_FrameIndex];
	m_pQueue->Signal(m_pFence.Get(), currentValue);
	
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

	m_pQueue->Signal(m_pFence.Get(), m_FenceCounter[m_FrameIndex]);
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

	m_pFence.Reset();

	m_pHeapRTV.Reset();
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pColorBuffer[i].Reset();
	}

	m_pCmdList.Reset();

	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pCmdAllocator[i].Reset();
	}

	m_pSwapChain.Reset();
	m_pQueue.Reset();

	m_pDevice.Reset();
}

