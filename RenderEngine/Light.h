#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>

struct alignas(256) DirectionalLightConstants
{
	DirectX::XMFLOAT3 color;
	float intensity;
	DirectX::XMFLOAT3 direction;
};

struct DirectionalLightCBVInfo
{
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
	DirectionalLightConstants* pBuffer;
	ComPtr<ID3D12Resource> buffer;
};

struct DirectionalLight
{
	DirectionalLight(uint32_t framecount = 2)
		: direction(0.0f, -1.0f, 0.0f),
		color(1.0f, 1.0f, 1.0f),
		intensity(1.0f),
		cbv(framecount)
	{}
	DirectX::XMFLOAT3 direction;
	DirectX::XMFLOAT3 color;
	float intensity;
	std::vector<DirectionalLightCBVInfo> cbv;
};

struct alignas(256) PointLight
{
	DirectX::XMFLOAT3 color;
	float intensity;
	DirectX::XMFLOAT3 position;
	float range;
};

struct alignas(256) SpotLight
{
	DirectX::XMFLOAT3 color;
	float intensity;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 direction;
	float range;
	float innerAngle;
};