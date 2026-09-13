#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>
#include "Object_win.h"
#include "DXMaterial.h"
#include "FileUtil.h"
#include "Scene.h"
#include "RenderConfig.h"
#include <DirectXTex.h>

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

struct LoadedModel
{
	std::vector<Mesh> meshes;
	std::wstring filepath;
};


struct PendingTextureUpload
{
	ComPtr<ID3D12Resource> uploadBuffer;
	std::wstring name;
};

class ResourceManager
{
public:
	ResourceManager(ID3D12Device* device, ID3D12DescriptorHeap* heap, ID3D12CommandQueue* queue, Scene* scene, const RenderConfig& config);
	~ResourceManager();

	Object* LoadModel(const wchar_t* filepath);
	Texture* LoadTexture(const wchar_t* filepath);
	std::unique_ptr<Texture> CreateTextureResource(DirectX::ScratchImage& image, std::wstring name);
	void CreateTextureSRV(Texture* texture, uint32_t descriptorIdex);
	bool CreateMaterialDescriptorTable(DXMaterial* material);
	void UploadLoadedTextures();
	bool CreateFallbackTextures();

	LoadedModel* GetLoadedModel(const std::wstring& filepath);
	Texture* GetLoadedTexture(const std::wstring& filepath);
	size_t GetLoadedModelCount() const {
		return m_loadedModels.size();
	}
	void ClearResources();

private:
	const RenderConfig& m_config;
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
	bool BasicPSOCreated = false;

	std::unordered_map<std::wstring, std::unique_ptr<LoadedModel>> m_loadedModels;
	//std::unordered_map<std::wstring, std::unique_ptr<Texture>> m_loadedTextures;

	std::vector<PendingTextureUpload> m_pendingTextureUploads;

	LoadedModel* LoadModelInternal(const wchar_t* filepath);
	Texture* LoadTextureInternal(const wchar_t* filepath);

	std::string ToUTF8(const std::wstring& value);
	void WaitGPU();

};