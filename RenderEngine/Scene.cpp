#include "Scene.h"

Scene::Scene(float camwidth, float camheight, float camnear, float camfar)
	:objects(),
	rootobjectIDMap(),
	lights(),
	objectIDCounter(0)
	//rootObjectIDCounter(0)
{
	cameras.push_back(new Camera(camwidth / camheight, camnear, camfar));
	mainCameraIndex = cameras.size() - 1;
	
}

Scene::~Scene()
{
	for (auto& camera : cameras) {
		delete camera;
	}
	//for (auto& object : objects) {
	//	delete object;
	//}
	for (auto& light : lights) {
		delete light;
	}
}

void Scene::addCamera(Camera* camera)
{
	cameras.push_back(camera);
}

void Scene::addObject(Object* object)
{
	//objects.insert({ objectIDCounter, object });
	uint32_t id = objectIDCounter;
	size_t index = objects.size();
	objects.push_back(object);
	object->objectID = id;
	objectIDs.push_back(id);
	objectIDMap[id] = index;
	if (object->isRoot) {
		size_t rootobjectindex = rootobjectIDMap.size();
		rootobjectIDMap[id] = rootobjectindex;
	}
	objectIDCounter++;
}

Object* Scene::callLoader(const wchar_t* path, ID3D12Device* device)
{
	ResourceManager resourceManager(device);
	Object* loadedObject = resourceManager.LoadModel(path);
	return loadedObject;
}

void Scene::removeObject(Object* object)
{
	uint32_t id = object->objectID;
	objects.erase(std::remove(objects.begin(), objects.end(), object), objects.end());
	objectIDs.erase(std::remove(objectIDs.begin(), objectIDs.end(), id), objectIDs.end());
	objectIDMap.erase(id);
	if (object->isRoot) {
		rootobjectIDMap.erase(id);
	}
	delete object;
}

bool Scene::removeCamera(Camera* camera)
{
	auto it = std::find(cameras.begin(), cameras.end(), camera);
	if (cameras.size() <= 1) {
		std::cout << "at least one camera" << std::endl;
		return false;
	}
	if (it == cameras.end()) {
		std::cout << "Camera not found in scene." << std::endl;
		return false;
	}
	if (it != cameras.end()) {
		auto index = std::distance(cameras.begin(), it);
		cameras.erase(it);
		if (mainCameraIndex == index) {
			mainCameraIndex = 0;
		}
		else if (mainCameraIndex > index) {
			--mainCameraIndex;
		}
		return true;
	}
}

bool Scene::changeMainCamera(size_t index)
{
	if (index < cameras.size()) {
		mainCameraIndex = index;
		return true;
	}
	else return false;
}