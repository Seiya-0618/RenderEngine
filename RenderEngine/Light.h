#pragma once
#include <d3d12.h>
#include <DirectXMath.h>

enum class LightType
{
	Directional,
	Point,
	Spot
};

class Light
{
public:
	Light(DirectX::XMFLOAT3 color, float intensity)
		: color(color), intensity(intensity) {
	}
	virtual ~Light() = default;
	DirectX::XMFLOAT3 color;
	float intensity;
};

class DirectionalLight : public Light
{
public:
	DirectionalLight(const DirectX::XMFLOAT3& dir, DirectX::XMFLOAT3 color, float intensity)
		: Light(color, intensity), direction(dir) {
	}
	DirectX::XMFLOAT3 direction;
	LightType GetLightType() const {
		return LightType::Directional;
	}
	DirectX::XMFLOAT3 GetDirection() const {
		return direction;
	}
	void SetDirection(const DirectX::XMFLOAT3& dir) {
		direction = dir;
	}
};