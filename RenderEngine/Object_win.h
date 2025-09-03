#pragma once
#include <iostream>
#include <vector>
#include <d3d12.h>
#include <DirectXMath.h>
#include "DXMaterial.h"
#include <wrl/client.h>

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

//To do
//Inport時に複数マテリアルを持つ単一メッシュはサブメッシュに分割する
struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
};


//CPU側で必要になった場合使用
struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

struct VertexBuffer {
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW view;
};

struct IndexBuffer {
	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW view;
};

struct alignas(256) Transform
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;
};

template<typename T> struct ConstantBufferView
{
	ComPtr<ID3D12Resource> buffer;
	D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
	T* pBuffer;
};

extern Vertex square[];
extern Mesh squareMesh;
extern const size_t squareVertexCount;



class Object
{
public:
	Object(uint32_t R_width, uint32_t R_height);
	~Object();
	//void AddMesh(Mesh mesh);
	bool AddMesh(Mesh mesh, ID3D12Device* device);
	//void AddMaterial(DXMaterial material);
	std::vector<VertexBuffer> vertexBuffers;
	std::vector<IndexBuffer> indexBuffers;
	
	//std::vector<Mesh*> meshes;
	//std::vector<DXMaterial*> materials;
	Transform transform;
	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV;
	ConstantBufferView<Transform> cbv[2]; //double buffer
	int meshCount = 0;
	

private:
	uint32_t width;
	uint32_t height;
};