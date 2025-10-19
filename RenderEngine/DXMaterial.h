#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

class DXMaterial
{
public:
	DXMaterial();
	~DXMaterial();

	ComPtr<ID3D12PipelineState> m_pPSO;
	std::string DiffuseMapPath;


private:
	ComPtr<ID3D12RootSignature> m_pRootSignature;
	ComPtr<ID3DBlob> m_pVSBlob;
	ComPtr<ID3DBlob> m_pPSBlob;

	DirectX::XMFLOAT3 Diffuse;
	DirectX::XMFLOAT3 Specular;
	float Alpha;
	float Shininess;
};