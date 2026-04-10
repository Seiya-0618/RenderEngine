#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>
#include "Object_win.h"
#include "FileUtil.h"
#include "Scene.h"

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

struct LoadedModel
{
	std::vector<Mesh> meshes;
	std::wstring filepath;
};


struct PendingTextureUpload
{
	ComPtr<ID3D12Resource> uploadBuffer;
	std::wstring filepath;
};

class ResourceManager
{
public:
	ResourceManager(ID3D12Device* device, ID3D12DescriptorHeap* heap, ID3D12CommandQueue* queue, Scene* scene);
	~ResourceManager();

	Object* LoadModel(const wchar_t* filepath);
	Texture* LoadTexture(const wchar_t* filepath);
	void UploadLoadedTextures();

	LoadedModel* GetLoadedModel(const std::wstring& filepath);
	Texture* GetLoadedTexture(const std::wstring& filepath);

	size_t GetLoadedModelCount() const {
		return m_loadedModels.size();
	}
	/*
	size_t GetLoadedTextureCoount() const {
		return m_loadedTextures.size();
	}
	*/

	void ClearResources();

private:
	ID3D12Device* m_pDevice;
	Scene* m_pScene;
	ID3D12DescriptorHeap* m_pSrvHeap;
	ID3D12CommandQueue* m_pCommandQueue;

	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;

	const size_t maxCBVCount = 1000;
	static constexpr const wchar_t* MODEL_DIRECTORY = L"res/SampleObj/";
	static constexpr const wchar_t* TEXTURE_DIRECTORY = L"res/SampleTex/";
	size_t CBVDescriptorIndex = 0;
	size_t SRVDescriptorIndex = 0;

	std::unordered_map<std::wstring, std::unique_ptr<LoadedModel>> m_loadedModels;
	//std::unordered_map<std::wstring, std::unique_ptr<Texture>> m_loadedTextures;

	std::vector<PendingTextureUpload> m_pendingTextureUploads;

	LoadedModel* LoadModelInternal(const wchar_t* filepath);
	Texture* LoadTextureInternal(const wchar_t* filepath);

	std::string ToUTF8(const std::wstring& value);
	void WaitGPU();

};