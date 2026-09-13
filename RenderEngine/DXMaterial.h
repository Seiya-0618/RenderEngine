#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include "PipelineKey.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
struct alignas(256) MaterialConstants
{
	DirectX::XMFLOAT4 BaseColor;
	float Roughness=1.0;
	float Metallic=0.0;
	float Padding[2];
};

class DXMaterial
{
public:
	DXMaterial(PipelineKey pipelineKey,
		std::wstring texturename)
		:m_PipelineKey(pipelineKey),
		DiffuseMapName(texturename),
		NormalMapName(L"Fallback_Normal"),
		MRMapName(L"Fallback_MR")
	{
	}
	~DXMaterial()
	{
	}

	PipelineKey GetPipelineKey() const {
		return m_PipelineKey;
	}

	PipelineKey m_PipelineKey;
	std::vector<uint32_t> InheritedObjectIDs;
	std::wstring DiffuseMapName;
	std::wstring NormalMapName;
	std::wstring MRMapName;
	D3D12_GPU_DESCRIPTOR_HANDLE MaterialTexHandle;

private:
	ComPtr<ID3D12RootSignature> m_pRootSignature;

};