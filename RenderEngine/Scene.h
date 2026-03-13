#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include "Object_win.h"
#include "Camera.h"
//#include "ModelLoader.h"
#include "ResourceManager.h"

class Scene
{
public:
	Scene(float camwidth, float camheight, float camnear, float camfar);
	~Scene();
	std::vector<Object*> objects;
	std::vector<uint32_t> objectIDs;
	std::map<uint32_t, size_t> objectIDMap; //uint32_t: objectID size_t: index
	std::map<uint32_t, size_t> rootobjectIDMap;

	

	std::vector<Object*> lights;
	std::vector<Camera*> cameras;
	size_t mainCameraIndex;

	void addCamera(Camera* camera);
	void addObject(Object* object);
	Object* callLoader(const wchar_t* path, ID3D12Device* device);
	void removeObject(Object* object);
	bool removeCamera(Camera* camera);
	bool changeMainCamera(size_t index);

private:
	uint32_t objectIDCounter;

};