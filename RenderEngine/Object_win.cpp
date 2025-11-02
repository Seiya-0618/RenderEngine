#include "Object_win.h"

Vertex square[] = {
	{DirectX::XMFLOAT3(-1.0f, 1.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)},
	{DirectX::XMFLOAT3(1.0f, 1.5f, 0.0f),  DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)},
	{DirectX::XMFLOAT3(1.0f, -1.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)},
	{DirectX::XMFLOAT3(-1.0f, -1.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)}
};

Mesh squareMesh = {
	std::vector<Vertex>(std::begin(square), std::end(square)),
	std::vector<uint32_t>{0, 1, 2, 0, 2, 3}
};

Object::Object(uint32_t R_width, uint32_t R_height, size_t id)
	:width(R_width),
	height(R_height),
	objectID(id)
{
	/* Do Nothing */
}

Object::~Object()
{
	for (auto vb : vertexBuffers)
	{
		vb.vertexBuffer->Release();
	}

	for (auto ib : indexBuffers)
	{
		ib.indexBuffer->Release();
	}
	/*
	for (auto material : materials)
	{
		delete material;
	}
	*/
}

bool Object::AddMesh(Mesh mesh, ID3D12Device* device)
{
	{
		//Vertex Buffer

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resDesc.Alignment = 0;
		resDesc.Width = sizeof(mesh.vertices[0]) * mesh.vertices.size();
		resDesc.Height = 1;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;
		resDesc.Format = DXGI_FORMAT_UNKNOWN;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ComPtr<ID3D12Resource> vertexbuffer;

		auto hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(vertexbuffer.GetAddressOf()));
		if (FAILED(hr))
		{
			std::cout << "Failed to create vertex buffer.HRESULT: 0x" << std::hex << hr << std::endl;
			return false;
		}
		//vertexBuffers.push_back(std::move(vertexbuffer)); //‚Ù‚ñ‚Æ‚É‚±‚ê‚Å‚¦‚¦‚ñ‚©H
		void* ptr = nullptr;
		hr = vertexbuffer->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			std::cout << "Failed to map vertex buffer.HRESULT: 0x" << std::hex << hr << std::endl;
			return false;
		}

		memcpy(ptr, mesh.vertices.data(), sizeof(mesh.vertices[0]) * mesh.vertices.size());
		vertexbuffer->Unmap(0, nullptr);

		D3D12_VERTEX_BUFFER_VIEW view;
		view.BufferLocation = vertexbuffer->GetGPUVirtualAddress();
		view.SizeInBytes = static_cast<UINT>(sizeof(mesh.vertices[0]) * mesh.vertices.size());
		view.StrideInBytes = sizeof(mesh.vertices[0]);

		VertexBuffer vb;
		vb.vertexBuffer = vertexbuffer;
		vb.view = view;
		vertexBuffers.push_back(vb);
	}

	{
		//Index Buffer

		D3D12_HEAP_PROPERTIES props = {};
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		props.CreationNodeMask = 1;
		props.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = sizeof(mesh.indices[0]) * mesh.indices.size();
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ComPtr<ID3D12Resource> indexbuffer;

		auto hr = device->CreateCommittedResource(
			&props,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(indexbuffer.GetAddressOf()));
		if (FAILED(hr))
		{
			std::cout << "Failed to create index buffer.HRESULT: 0x" << std::hex << hr << std::endl;
			return false;
		}
		void* ptr = nullptr;
		hr = indexbuffer->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			std::cout << "Failed to map index buffer.HRESULT: 0x" << std::hex << hr << std::endl;
			return false;
		}
		memcpy(ptr, mesh.indices.data(), sizeof(mesh.indices[0])*mesh.indices.size());
		indexbuffer->Unmap(0, nullptr);

		D3D12_INDEX_BUFFER_VIEW view;
		view.BufferLocation = indexbuffer->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<UINT>(sizeof(mesh.indices[0]) * mesh.indices.size());

		IndexBuffer ib;
		ib.indexBuffer = indexbuffer;
		ib.view = view;
		indexBuffers.push_back(ib);

	}

	{   /*
		//Constant Buffer
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 2;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.NodeMask = 0;

		auto hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pHeapCBV.GetAddressOf()));
		if (FAILED(hr))
		{
			std::cout << "Failed to create CBV descriptor heap.HRESULT: 0x" << std::hex << hr << std::endl;
			return false;
		}

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

		auto incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		for (auto i = 0; i < 2; ++i)
		{
			ComPtr<ID3D12Resource> buffer;
			auto hr = device->CreateCommittedResource(
				&heapprops,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(buffer.GetAddressOf()));
			if (FAILED(hr))
			{
				std::cout << "Failed to create constant buffer.HRESULT: 0x" << std::hex << hr << std::endl;
				return false;
			}
			cbv[i].buffer = buffer;
			auto address = (cbv[i].buffer)->GetGPUVirtualAddress();
			auto handleCPU = m_pHeapCBV->GetCPUDescriptorHandleForHeapStart();
			auto handleGPU = m_pHeapCBV->GetGPUDescriptorHandleForHeapStart();
			handleCPU.ptr += i * incrementSize;
			handleGPU.ptr += i * incrementSize;
			cbv[i].HandleCPU = handleCPU; 
			cbv[i].HandleGPU = handleGPU;
			cbv[i].Desc.BufferLocation = address;
			cbv[i].Desc.SizeInBytes = sizeof(Transform);

			device->CreateConstantBufferView(&cbv[i].Desc, handleCPU);
			hr = cbv[i].buffer->Map(0, nullptr, reinterpret_cast<void**>(&cbv[i].pBuffer));
			if (FAILED(hr))
			{
				std::cout << "Failed to map constant buffer.HRESULT: 0x" << std::hex << hr << std::endl;
				return false;
			}
			auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
			auto targetPos = DirectX::XMVectorZero();
			auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			auto fovY = DirectX::XMConvertToRadians(37.5f);
			auto aspect = static_cast<float>(width) / static_cast<float>(height);
			
			cbv[i].pBuffer->World = DirectX::XMMatrixIdentity();
			cbv[i].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
			cbv[i].pBuffer->Projection = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 100.0f);
		}
		*/
	}

}


void Object::AddChild(Object* child)
{
	childrenIDs.push_back(child->objectID);
	child->parentID = this->objectID;
}


size_t Object::GetParentID()
{
	return parentID;
}