#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>

/*
Parameter order
Position
Direction
Color 
Intensity
Range
*/

struct alignas(256) DirectionalLightConstants
{
	DirectX::XMFLOAT3 direction;
	float padding0;
	DirectX::XMFLOAT3 color;
	float padding1;
	float intensity;
	float padding2[3];
};

struct DirectionalLightCBVInfo
{
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
	DirectionalLightConstants* pBuffer = nullptr;
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

struct alignas(256) PointLightConstants
{
	DirectX::XMFLOAT3 position;
	float padding0;

	DirectX::XMFLOAT3 color;
	float padding1;

	float intensity;
	float range;
	float padding2[2];
};

struct alignas(256) SpotLightConstants
{
	DirectX::XMFLOAT3 position;
	float padding0;

	DirectX::XMFLOAT3 direction;
	float padding1;

	DirectX::XMFLOAT3 color;
	float padding2;

	float intensity;
	float range;
	float innerAngle;
	float pading3;
};