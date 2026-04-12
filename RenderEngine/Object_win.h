#pragma once
#include <iostream>
#include <vector>
#include <d3d12.h>
#include <DirectXMath.h>
//#include "DXMaterial.h"
#include <wrl/client.h>

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

//To do
//Inport時に複数マテリアルを持つ単一メッシュはサブメッシュに分割する
struct Vertex
{
	DirectX::XMFLOAT3 position;
	//DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
};


//CPU側で必要になった場合使用
struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	uint32_t materialIndex;
};

struct VertexBuffer {
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW view;
};

struct IndexBuffer {
	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW view;
};

struct Texture
{
	ComPtr<ID3D12Resource> resource;
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU;
};

struct ObjectTransform
{
	DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f }; //Euler
	DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
};

struct alignas(256) ObjectConstants
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;
};

struct ObjectCBVInfo {
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
	ObjectConstants* pBuffer;
	ComPtr<ID3D12Resource> buffer;
};

extern Vertex square[];
extern Mesh squareMesh;
extern const size_t squareVertexCount;



class Object
{
public:
	Object();
	~Object();
	bool AddMesh(Mesh mesh, ID3D12Device* device);
	void AddChild(Object* child);
	size_t GetParentID();
	void SetTextureName(const std::wstring& name) { m_textureName = name; }
	const std::wstring& GetTextureName() const { return m_textureName; }
	DirectX::XMMATRIX TransformToMatrix();
	void UpdateWorldMatrix(const DirectX::XMMATRIX& parentWorldMatrix);
	
	std::vector<VertexBuffer> vertexBuffers;
	std::vector<IndexBuffer> indexBuffers;
	std::vector<size_t> childrenIDs;
	
	ObjectConstants constants;
	ObjectTransform localTransform;
	DirectX::XMMATRIX worldMatrix;
	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV;
	ObjectCBVInfo cbv[2];
	int meshCount = 0;
	bool isRoot = true;
	size_t objectID;

private:
	Object* parent = nullptr;
	std::wstring m_textureName;
};