#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include "Object_win.h"
#include "Camera.h"
//#include "ModelLoader.h"
//#include "ResourceManager.h"

class Scene
{
public:
	Scene(float camwidth, float camheight, float camnear, float camfar);
	~Scene();
	std::vector<Object*> objects;
	std::vector<uint32_t> objectIDs;
	std::map<uint32_t, size_t> objectIDMap; //uint32_t: objectID size_t: index
	std::map<uint32_t, size_t> rootobjectIDMap;
	std::unordered_map<std::wstring, std::unique_ptr<Texture>> textureMap;

	

	std::vector<Object*> lights;
	std::vector<Camera*> cameras;
	size_t mainCameraIndex;

	void addCamera(Camera* camera);
	void addObject(Object* object);
	void removeObject(Object* object);
	void UpdateWorldTransforms();
	void UpdateWorldTransformsRecursive(Object* parentObject);
	bool removeCamera(Camera* camera);
	bool changeMainCamera(size_t index);
	void AddTexture(const std::wstring& name, std::unique_ptr<Texture> texture);
	Texture* GetTexture(const std::wstring& filepath);
	uint32_t GetObjectCount() const { return static_cast<uint32_t>(objects.size()); }


private:
	uint32_t objectIDCounter;

};