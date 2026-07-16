#pragma once
#include <DirectXMath.h>

struct alignas(256) CameraConstants
{
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
};

struct CameraCBVInfo
{
    D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
    CameraConstants* pBuffer;
    ComPtr<ID3D12Resource> buffer;
};

class Camera
{
public:
    Camera(float aspect, float nearZ = 0.1f, float farZ = 100.0f, uint32_t frameCount=2)
        : position(0.0f, 1.0f, -2.0f),
        target(0.0f, 0.8f, 0.0f),
        up(0.0f, 1.0f, 0.0f),
        aspectRatio(aspect),
        nearPlane(nearZ),
        farPlane(farZ),
        cbv(frameCount)
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
    std::vector<CameraCBVInfo> cbv;

private:
    float aspectRatio;
    float nearPlane;
    float farPlane;
};
