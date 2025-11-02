#pragma once
#include <DirectXMath.h>

class Camera
{
public:
    Camera(float aspect, float nearZ = 0.1f, float farZ = 100.0f)
        : position(0.0f, 0.0f, -5.0f),
        target(0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f),
        aspectRatio(aspect),
        nearPlane(nearZ),
        farPlane(farZ)
    {
    }

    DirectX::XMMATRIX GetViewMatrix() const {
        return DirectX::XMMatrixLookAtRH(
            DirectX::XMLoadFloat3(&position),
            DirectX::XMLoadFloat3(&target),
            DirectX::XMLoadFloat3(&up)
        );
    }

    DirectX::XMMATRIX GetProjectionMatrix() const {
        // 視野角45度（ラジアン）
        constexpr float fovY = DirectX::XMConvertToRadians(45.0f);
        return DirectX::XMMatrixPerspectiveFovRH(
            fovY, aspectRatio, nearPlane, farPlane
        );
    }

    // 必要に応じてカメラ位置やターゲットを変更可能
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 target;
    DirectX::XMFLOAT3 up;
    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorRect;

private:
    float aspectRatio;
    float nearPlane;
    float farPlane;
};
