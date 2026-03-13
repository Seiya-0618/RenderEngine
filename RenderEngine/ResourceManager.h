#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <d3d12.h>
#include <wrl/client.h>
#include "Object_win.h"

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

struct LoadedModel
{
	std::vector<Mesh> meshes;
	std::wstring filepath;
};

struct LoadedTexture
{
	ComPtr<ID3D12Resource> resource;
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU;
};

class ResourceManager
{
public:
	ResourceManager(ID3D12Device* device);// , ID3D12CommandQueue* queue);
	~ResourceManager();

	Object* LoadModel(const wchar_t* filepath);
	LoadedTexture* LoadTexture(const wchar_t* filepath);

	LoadedModel* GetLoadedModel(const std::wstring& filepath);
	LoadedTexture* GetLoadedTexture(const std::wstring& filepath);

	size_t GetLoadedModelCount() const {
		return m_loadedModels.size();
	}
	size_t GetLoadedTextureCoount() const {
		return m_loadedTextures.size();
	}

	void ClearResources();

private:
	ID3D12Device* m_pDevice;
	ID3D12CommandQueue* m_pCommandQueue;

	std::unordered_map<std::wstring, std::unique_ptr<LoadedModel>> m_loadedModels;
	std::unordered_map<std::wstring, std::unique_ptr<LoadedTexture>> m_loadedTextures;

	LoadedModel* LoadModelInternal(const wchar_t* filepath);
	LoadedTexture* LoadTextureInternal(const wchar_t* filepath);

	std::string ToUTF8(const std::wstring& value);

};