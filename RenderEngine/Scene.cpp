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
		rootobjectIDMap[id] = index;
	}
	objectIDCounter++;
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
	return false;
}

void Scene::UpdateWorldTransforms()
{
	using namespace DirectX;
	XMMATRIX identity = XMMatrixIdentity();
	for (auto rootID : rootobjectIDMap) {
		uint32_t id = rootID.first;
		size_t rootIndex = rootID.second;

		if (rootIndex < objects.size())
		{
			Object* rootObject = objects[rootIndex];
			rootObject->UpdateWorldMatrix(identity);
			UpdateWorldTransformsRecursive(rootObject);
		}
	}
}

void Scene::UpdateWorldTransformsRecursive(Object* parentObject)
{
	using namespace DirectX;
	XMMATRIX parentWorldMatrix = parentObject->worldMatrix;
	for (size_t childID : parentObject->childrenIDs) {
		size_t childIndex = objectIDMap.find(childID)->second;
		if (childIndex < objects.size())
		{
			Object* childObject = objects[childIndex];
			childObject->UpdateWorldMatrix(parentWorldMatrix);
			UpdateWorldTransformsRecursive(childObject);
		}
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

void Scene::AddTexture(const std::wstring& name, std::unique_ptr<Texture> texture)
{
	if (textureMap.find(name) != textureMap.end())
	{
		std::cout << L"Warning: Texture already exists with name: " << name.c_str() << L". Overwriting." << std::endl;
		return;
	}
	textureMap[name] = std::move(texture);
}

Texture* Scene::GetTexture(const std::wstring& filepath)
{
	auto it = textureMap.find(filepath);
	if (it != textureMap.end())
	{
		return it->second.get();
	}
	return nullptr;
}