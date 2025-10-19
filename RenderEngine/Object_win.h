#pragma once
#include <iostream>
#include <vector>
#include <d3d12.h>
#include <DirectXMath.h>
//#include "DXMaterial.h"
#include <wrl/client.h>

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

//To do
//Inport���ɕ����}�e���A�������P�ꃁ�b�V���̓T�u���b�V���ɕ�������
struct Vertex
{
	DirectX::XMFLOAT3 position;
	//DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
};


//CPU���ŕK�v�ɂȂ����ꍇ�g�p
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

struct alignas(256) Transform
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;
};

struct ObjectCBVInfo {
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
	Transform* pBuffer;
	ComPtr<ID3D12Resource> buffer;
};

extern Vertex square[];
extern Mesh squareMesh;
extern const size_t squareVertexCount;



class Object
{
public:
	Object(uint32_t R_width, uint32_t R_height);
	~Object();
	bool AddMesh(Mesh mesh, ID3D12Device* device);
	void AddChild(Object* child);
	Object* GetParent();
	std::vector<VertexBuffer> vertexBuffers;
	std::vector<IndexBuffer> indexBuffers;
	std::vector<Object*> children;
	
	Transform transform;
	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV;
	ObjectCBVInfo cbv[2];
	int meshCount = 0;
	

private:
	Object* parent;
	uint32_t width;
	uint32_t height;
};