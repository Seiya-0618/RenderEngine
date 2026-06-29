#include "ResourceManager.h"
#include "FileUtil.h"
#include <iostream>
#include <DirectXTex.h>



#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#pragma comment (lib, "DirectXTex.lib")
#pragma comment (lib, "assimp-vc142-mtd.lib")

std::wstring GetFileNameFromPath(const std::wstring& path)
{
	size_t lastSlash = path.find_last_of(L"\\/");
	if (lastSlash != std::wstring::npos)
	{
		return path.substr(lastSlash + 1);
	}
	return path;
}

ResourceManager::ResourceManager(ID3D12Device* device, ID3D12DescriptorHeap* heap, ID3D12CommandQueue* queue, Scene* scene)
	:m_pDevice(device),
	m_pSrvHeap(heap),
	m_pCommandQueue(queue),
	m_pScene(scene)
{
	HRESULT hr = m_pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_pCommandAllocator.GetAddressOf())
	);
	if (FAILED(hr))
	{
		std::cout << "Failed to create command allocator for ResourceManager." << std::endl;
	}
	hr = m_pDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pCommandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(m_pCommandList.GetAddressOf())
		);
	if (FAILED(hr))
	{
		std::cout << "Failed to create command list for ResourceManager." << std::endl;
	}
}

ResourceManager::~ResourceManager()
{
	ClearResources();
}

Object* ResourceManager::LoadModel(const wchar_t* filepath)
{
	std::wstring filename = GetFileNameFromPath(filepath);
	std::wstring modelpath = std::wstring(MODEL_DIRECTORY) + filename;
	std::wstring fullpath;
	if (!SearchFilePath(modelpath.c_str(), fullpath))
	{
		std::cout << "Failed to find model file: " << filepath << std::endl;
		return nullptr;
	}
	std::string path = ToUTF8(fullpath);
	
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_GenSmoothNormals;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_RemoveRedundantMaterials;
	flag |= aiProcess_OptimizeMeshes;

	std::cout << "Loading model from: " << path << std::endl;

	const aiScene* pScene = importer.ReadFile(path, flag);
	if (pScene == nullptr)
	{
		std::cout << "Failed to load model(pScene is Null)" << importer.GetErrorString() << std::endl;
		return nullptr;
	}

	Object* rootObject = new Object();
	rootObject->isRoot = true;
	m_pScene->addObject(rootObject);

	size_t meshCount = pScene->mNumMeshes;
	std::vector<Mesh> meshes(meshCount);
	for (size_t i = 0; i < meshCount; ++i)
	{
		const aiMesh* pMesh = pScene->mMeshes[i];
		size_t vertexCount = pMesh->mNumVertices;
		for (size_t j = 0; j < vertexCount; ++j)
		{
			Vertex vertex;
			vertex.position = { pMesh->mVertices[j].x, pMesh->mVertices[j].y, pMesh->mVertices[j].z };
			if (pMesh->mTextureCoords[0]) {
				vertex.uv = { pMesh->mTextureCoords[0][j].x,1.0f - pMesh->mTextureCoords[0][j].y };
			}
			else {
				vertex.uv = { 0.0f, 0.0f };
			}
			if (pMesh->mNormals) {
				vertex.normal = { pMesh->mNormals[j].x, pMesh->mNormals[j].y, pMesh->mNormals[j].z };
			}
			else {
				vertex.normal = { 0.0f, 0.0f, 0.0f };
			}
			if (pMesh->mTangents) {
				vertex.tangent = { pMesh->mTangents[j].x, pMesh->mTangents[j].y, pMesh->mTangents[j].z };
			}
			else {
				vertex.tangent = { 0.0f, 0.0f, 0.0f };
			}
			meshes[i].vertices.push_back(vertex);
		}
		size_t faceCount = pMesh->mNumFaces;
		for (size_t j = 0; j < faceCount; ++j)
		{
			aiFace face = pMesh->mFaces[j];
			if (face.mNumIndices != 3)
			{
				std::cout << "Non-triangulated face detected. Skipping." << std::endl;
				continue;
			}
			meshes[i].indices.push_back(face.mIndices[0]);
			meshes[i].indices.push_back(face.mIndices[1]);
			meshes[i].indices.push_back(face.mIndices[2]);
		}
		Object* meshObject = new Object();
		meshObject->AddMesh(meshes[i], m_pDevice);
		meshObject->isRoot = false;

		unsigned int materialIndex = pMesh->mMaterialIndex;
		if (materialIndex < pScene->mNumMaterials)
		{
			const aiMaterial* material = pScene->mMaterials[materialIndex];
			aiString texturePath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
			{
				std::string texPath_str = texturePath.C_Str();
				std::wstring texturePathW(texPath_str.begin(), texPath_str.end());
				Texture* texture = LoadTexture(texturePathW.c_str());
				if (texture == nullptr)
				{
					std::cout << "Failed to load texture: " << texturePath.C_Str() << std::endl;
				}
				else
				{
					std::wstring textureFilename = GetFileNameFromPath(texturePathW);
					meshObject->SetTextureName(textureFilename);
				}
			}
			else
			{
				std::cout << "No diffuse texture found for material index: " << materialIndex << std::endl;
			}
			DXMaterial newMat(PipelineKey::Basic, meshObject->GetTextureName(), 0.5f, 0.5f);
			uint32_t materialID = m_pScene->AddMaterial(std::make_unique<DXMaterial>(newMat));
			meshObject->materialIndex = materialID;
				
		}

		m_pScene->addObject(meshObject);
		DXMaterial* mat = m_pScene->GetMaterial(meshObject->materialIndex);
		mat->InheritedObjectIDs.push_back(meshObject->objectID);
		rootObject->AddChild(meshObject);
	}

	UploadLoadedTextures();

	std::cout << "model loaded: " << path.c_str() << std::endl;
	std::unique_ptr<LoadedModel> loadedModel = std::make_unique<LoadedModel>();
	loadedModel->meshes = meshes;
	loadedModel->filepath = filepath;
	m_loadedModels[filepath] = std::move(loadedModel);
	return rootObject;
}

Texture* ResourceManager::LoadTexture(const wchar_t* filepath)
{

	std::wstring filename = GetFileNameFromPath(filepath);
	std::wstring texturepath = std::wstring(TEXTURE_DIRECTORY) + filename;
	std::wstring path;

	Texture* existingTex = m_pScene->GetTexture(filename);
	if (existingTex != nullptr)
	{
		std::cout << "Texture already loaded: " << filename.c_str() << std::endl;
		return existingTex;
	}

	if (!SearchFilePath(texturepath.c_str(), path))
	{
		std::wcout << L"Failed to find texture file: " << texturepath.c_str() << std::endl;
	}

	std::wcout << L" Loaded texture at : " << path.c_str() << std::endl;

	auto loadedtexture = std::make_unique<Texture>();
	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage image = {};
	HRESULT hr = DirectX::LoadFromWICFile(
		path.c_str(),
		DirectX::WIC_FLAGS_NONE,
		&metadata,
		image);
	if (FAILED(hr))
	{
		std::cout << "Failed to load texture file: " << path.c_str() << std::endl;
		return nullptr;
	}
	auto img = image.GetImage(0, 0, 0);
	D3D12_RESOURCE_DESC texDesc = {};
	D3D12_HEAP_PROPERTIES texProps = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = img->width;
	texDesc.Height = img->height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = metadata.format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	texProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	texProps.CreationNodeMask = 0;
	texProps.VisibleNodeMask = 0;

	hr = m_pDevice->CreateCommittedResource(
		&texProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(loadedtexture->resource.GetAddressOf())
	);
	if (FAILED(hr))
	{
		std::cout << "Failed to create texture resource." << std::endl;
		return nullptr;
	}
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = texDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	/*
	m_pDevice->CreateShaderResourceView(
		loadedtexture->resource.Get(),
		&srvDesc,
		loadedtexture->handleCPU
	);
	*/
	ComPtr<ID3D12Resource> uploadHeap;
	UINT64 uploadBufferSize = GetRequiredIntermediateSize(loadedtexture->resource.Get(), 0, 1);
	D3D12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	CD3DX12_HEAP_PROPERTIES uploadProps(D3D12_HEAP_TYPE_UPLOAD);
	hr = m_pDevice->CreateCommittedResource(
		&uploadProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeap)
	);
	if (FAILED(hr))
	{
		std::cout << "Failed to create upload heap for texture." << std::endl;
		return nullptr;
	}

	D3D12_SUBRESOURCE_DATA subresourcedata = {};
	subresourcedata.pData = img->pixels;
	subresourcedata.RowPitch = img->rowPitch;
	subresourcedata.SlicePitch = img->slicePitch;

	UINT64 result = UpdateSubresources(
		m_pCommandList.Get(),
		loadedtexture->resource.Get(),
		uploadHeap.Get(),
		0, 0, 1,
		&subresourcedata
	);
	if (result == 0)
	{
		std::cout << "Failed to update subresources for texture." << std::endl;
		return nullptr;
	}
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		loadedtexture->resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	m_pCommandList->ResourceBarrier(1, &barrier);
	UINT incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	auto srvHandleCPU = m_pSrvHeap->GetCPUDescriptorHandleForHeapStart();
	srvHandleCPU.ptr += incrementSize * (maxCBVCount + SRVDescriptorIndex);
	loadedtexture->handleCPU = srvHandleCPU;
	auto srvHandleGPU = m_pSrvHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandleGPU.ptr += incrementSize * (maxCBVCount + SRVDescriptorIndex);
	loadedtexture->handleGPU = srvHandleGPU;

	m_pDevice->CreateShaderResourceView(
		loadedtexture->resource.Get(),
		&srvDesc,
		loadedtexture->handleCPU
	);

	SRVDescriptorIndex++;


	PendingTextureUpload pending;
	pending.uploadBuffer = uploadHeap;
	pending.filepath = path;
	m_pendingTextureUploads.push_back(pending);

	Texture* texPtr = loadedtexture.get();
	m_pScene->AddTexture(filename, std::move(loadedtexture));

	return texPtr;
}

void ResourceManager::UploadLoadedTextures()
{
	HRESULT hr = m_pCommandList->Close();
	if (FAILED(hr))
	{
		std::cout << "Failed to close command list for texture uploads." << std::endl;
		return;
	}
	ID3D12CommandList* cmdLists[] = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(1, cmdLists);
	WaitGPU();

	m_pendingTextureUploads.clear();
	m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);
}


LoadedModel* ResourceManager::GetLoadedModel(const std::wstring& filepath)
{
	auto it = m_loadedModels.find(filepath);
	if (it != m_loadedModels.end())
	{
		return it->second.get();
	}
	return nullptr;
}

void ResourceManager::ClearResources()
{
	/* do nothing*/
	return;
}

std::string ResourceManager::ToUTF8(const std::wstring& value)
{
	if (value.empty()) return std::string();
	int length = WideCharToMultiByte(
		CP_UTF8,
		0,
		value.data(),
		static_cast<int>(value.size()),
		nullptr,
		0,
		nullptr,
		nullptr
	);
	if (length == 0) return std::string();

	std::string utf8str(length, 0);
	WideCharToMultiByte(
		CP_UTF8,
		0,
		value.data(),
		static_cast<int>(value.size()),
		&utf8str[0],
		length,
		nullptr,
		nullptr
	);

	return utf8str;
}

void ResourceManager::WaitGPU()
{
	ComPtr<ID3D12Fence> fence;
	m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_pCommandQueue->Signal(fence.Get(), 1);
	fence->SetEventOnCompletion(1, fenceEvent);
	WaitForSingleObject(fenceEvent, INFINITE);
	CloseHandle(fenceEvent);
}