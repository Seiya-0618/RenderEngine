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
	float Roughness;
	float Metallic;
	float Padding[2];
};

class DXMaterial
{
public:
	DXMaterial(PipelineKey pipelineKey,
		std::wstring texturename,
		float Roughness,
		float Metallic)
		:m_PipelineKey(pipelineKey),
		DiffuseMapName(texturename),
		Roughness(Roughness),
		Metallic(Metallic)
	{
	}
	~DXMaterial()
	{
	}

	PipelineKey m_PipelineKey;
	std::wstring DiffuseMapName;
	std::vector<uint32_t> InheritedObjectIDs;
	float Roughness;
	float Metallic;


private:
	ComPtr<ID3D12RootSignature> m_pRootSignature;

};