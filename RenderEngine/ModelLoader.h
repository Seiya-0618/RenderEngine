#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <codecvt>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <d3d12.h>

#pragma comment (lib, "assimp-vc142-mtd.lib")

bool ModelLoad(wchar_t* filepath);
std::string ToUTF8(const std::wstring& value);