#include "ModelLoader.h"

#include <d3d12.h>
#include "Object_win.h"

Object* ModelLoad(const wchar_t* filepath, ID3D12Device* device)
{
	Object* obj = new Object();
	if (!filepath)
	{
		std::cout << "filepath is null." << std::endl;
		nullptr;
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
		std::cout << "Failed to load model: " << importer.GetErrorString() << std::endl;
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
			if (face.mNumIndices != 3) {
				std::cout << "Non-triangulated face detected. Skipping." << std::endl;
				continue;
			}
			meshes[i].indices.push_back(face.mIndices[0]);
			meshes[i].indices.push_back(face.mIndices[1]);
			meshes[i].indices.push_back(face.mIndices[2]);
			meshes[i].materialIndex = pMesh->mMaterialIndex;
		}

		obj->AddMesh(meshes[i], device);


		std::cout << "Model read Success" << std::endl;
		return obj;
	}
}

// std::wstring ¨ UTF-8 std::string •ÏŠ·ŠÖ”
std::string ToUTF8(const std::wstring& value)
{
    if (value.empty()) return std::string();

    int length = WideCharToMultiByte(
        CP_UTF8, 0,
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
        CP_UTF8, 0,
        value.data(),
        static_cast<int>(value.size()),
        &utf8str[0],
        length,
        nullptr,
        nullptr
    );

    return utf8str;
}