#pragma once
#include <iostream>
#include <vector>
#include <d3d12.h>
#include <DirectXMath.h>
#include "DXMaterial.h"
#include <wrl/client.h>

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

//To do
//Inport���ɕ����}�e���A�������P�ꃁ�b�V���̓T�u���b�V���ɕ�������
struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
};


//CPU���ŕK�v�ɂȂ����ꍇ�g�p
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

extern Vertex square[];
extern Mesh squareMesh;
extern const size_t squareVertexCount;



class Object
{
public:
	Object();
	~Object();
	//void AddMesh(Mesh mesh);
	bool AddMesh(Mesh mesh, ID3D12Device* device);
	//void AddMaterial(DXMaterial material);
	std::vector<VertexBuffer> vertexBuffers;
	std::vector<IndexBuffer> indexBuffers;
	//std::vector<Mesh*> meshes;
	//std::vector<DXMaterial*> materials;
	DirectX::XMMATRIX transform;
	int meshCount = 0;
	

private:
};