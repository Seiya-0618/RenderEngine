#include "ModelLoader.h"

bool ModelLoad(wchar_t* filepath)
{
	if (!filepath)
	{
		std::cout << "filepath is null." << std::endl;
		return false;
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
		return false;
	}

	size_t meshCount = pScene->mNumMeshes;
	for (size_t i=0; i < meshCount; ++i)
	{
		auto pMesh = pScene->mMeshes[i];
	}


	return true;
}

// std::wstring Å® UTF-8 std::string ïœä∑ä÷êî
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