#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

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
	DXMaterial(std::string pipelineKey,
		std::wstring texturename,
		float Roughness,
		float Metallic)
		:PipelineKey(pipelineKey),
		DiffuseMapName(texturename),
		Roughness(Roughness),
		Metallic(Metallic)
	{
	}
	~DXMaterial()
	{
	}

	std::string PipelineKey;
	std::wstring DiffuseMapName;
	std::vector<uint32_t> InheritedObjectIDs;
	float Roughness;
	float Metallic;


private:
	ComPtr<ID3D12RootSignature> m_pRootSignature;

};