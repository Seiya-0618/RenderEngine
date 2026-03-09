#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Protect commonly conflicting macros around Assimp includes
#ifdef Type
#pragma push_macro("Type")
#undef Type
#define ASSIMP_RESTORE_Type
#endif

#ifdef interface
#pragma push_macro("interface")
#undef interface
#define ASSIMP_RESTORE_interface
#endif

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifdef ASSIMP_RESTORE_Type
#pragma pop_macro("Type")
#undef ASSIMP_RESTORE_Type
#endif

#ifdef ASSIMP_RESTORE_interface
#pragma pop_macro("interface")
#undef ASSIMP_RESTORE_interface
#endif

#include <codecvt>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
//#include <d3d12.h>
#include "Object_win.h"

#pragma comment (lib, "assimp-vc142-mtd.lib")

Object* ModelLoad(const wchar_t* filepath, ID3D12Device* device);
//std::unique_ptr<Object> ModelLoad(const wchar_t* filepath, ID3D12Device* device);
std::string ToUTF8(const std::wstring& value);