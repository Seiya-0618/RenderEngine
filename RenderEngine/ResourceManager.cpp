#include "ResourceManager.h"
#include <iostream>
//#include <DirectXTex.h>



#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



#pragma comment (lib, "assimp-vc142-mtd.lib")

ResourceManager::ResourceManager(ID3D12Device* device)//, ID3D12CommandQueue* queue)
	:m_pDevice(device)
	//m_pCommandQueue(queue)
{
}

ResourceManager::~ResourceManager()
{
	ClearResources();
}

Object* ResourceManager::LoadModel(const wchar_t* filepath)
{
	Object* obj = new Object();
	if (!filepath)
	{
		std::cout << "filepath is null." << std::endl;
		return nullptr;
	}
	auto path = ToUTF8(filepath);
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_GenSmoothNormals;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_RemoveRedundantMaterials;
	flag |= aiProcess_OptimizeMeshes;

	auto pScene = importer.ReadFile(path, flag);
	if (pScene == nullptr)
	{
		std::cout << "Failed to load model(pScene is Null)" << importer.GetErrorString() << std::endl;
		return nullptr;
	}

	size_t meshCount = pScene->mNumMeshes;
	std::vector<Mesh> meshes(meshCount);
	for (size_t i = 0; i < meshCount; ++i)
	{
		auto pMesh = pScene->mMeshes[i];
		size_t vertexCount = pMesh->mNumVertices;
		for (size_t j = 0; j < vertexCount; ++j)
		{
			Vertex vertex;
			vertex.position = { pMesh->mVertices[j].x, pMesh->mVertices[j].y, pMesh->mVertices[j].z };
			if (pMesh->mTextureCoords[0]) {
				vertex.uv = { pMesh->mTextureCoords[0][j].x, pMesh->mTextureCoords[0][j].y };
			}
			else {
				vertex.uv = { 0.0f, 0.0f };
			}
			if (pMesh->mNormals) {
				vertex.normal = { pMesh->mNormals[j].x, pMesh->mNormals[j].y, pMesh->mNormals[j].z };
			}
			else {
				vertex.normal = { 0.0f, 0.0f, 0.0f };
			}
			if (pMesh->mTangents) {
				vertex.tangent = { pMesh->mTangents[j].x, pMesh->mTangents[j].y, pMesh->mTangents[j].z };
			}
			else {
				vertex.tangent = { 0.0f, 0.0f, 0.0f };
			}
			meshes[i].vertices.push_back(vertex);
		}
		size_t faceCount = pMesh->mNumFaces;
		for (size_t j = 0; j < faceCount; ++j)
		{
			aiFace face = pMesh->mFaces[j];
			if (face.mNumIndices != 3)
			{
				std::cout << "Non-triangulated face detected. Skipping." << std::endl;
				continue;
			}
			meshes[i].indices.push_back(face.mIndices[0]);
			meshes[i].indices.push_back(face.mIndices[1]);
			meshes[i].indices.push_back(face.mIndices[2]);
			//meshes[j].materialIndex = pMesh->mMaterialIndex;
		}
		obj->AddMesh(meshes[i], m_pDevice);
	}
	std::cout << "model loaded: " << path.c_str() << std::endl;
	return obj;
}

void ResourceManager::ClearResources()
{
	m_loadedModels.clear();
	m_loadedTextures.clear();
}

std::string ResourceManager::ToUTF8(const std::wstring& value)
{
	if (value.empty()) return std::string();
	int length = WideCharToMultiByte(
		CP_UTF8,
		0,
		value.data(),
		static_cast<int>(value.size()),
		nullptr,
		0,
		nullptr,
		nullptr
	);
	if (length == 0) return std::string();

	std::string utf8str(length, 0);
	WideCharToMultiByte(
		CP_UTF8,
		0,
		value.data(),
		static_cast<int>(value.size()),
		&utf8str[0],
		length,
		nullptr,
		nullptr
	);

	return utf8str;
}