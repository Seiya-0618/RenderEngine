#include "Scene.h"

Scene::Scene(float camwidth, float camheight, float camnear, float camfar)
	:objects(),
	objectIDs(),
	objectIDMap(),
	rootobjectIDMap(),
	textureMap(),
	materialMap(),
	directionalLights(),
	cameras(),
	mainCameraIndex(0),
	objectIDCounter(0),
	materialIDCounter(0),
	directionalLightIDCounter(0)
{
	cameras.push_back(new Camera(camwidth / camheight, camnear, camfar));
	mainCameraIndex = cameras.size() - 1;
	
}

Scene::~Scene()
{
	for (auto& camera : cameras) {
		delete camera;
	}

}

void Scene::addCamera(Camera* camera)
{
	cameras.push_back(camera);
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

void Scene::addObject(Object* object)
{
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

bool Scene::addDirectionalLight(DirectionalLight light)
{
	uint32_t id = directionalLightIDCounter;
	directionalLights.push_back(std::make_unique<DirectionalLight>(light));
	if (directionalLights.size() < directionalLightIDCounter)
	{
		std::cout << "failed to add directionalLight" << std::endl;
		return false;
	}
	directionalLightIDCounter++;
	return true;
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

DirectX::XMMATRIX Scene::GetMainCameraViewMatrix()
{
	if (mainCameraIndex < cameras.size()) {
		return cameras[mainCameraIndex]->GetViewMatrix();
	}
	else {
		std::cout << "Main camera index is out of range." << std::endl;
		return DirectX::XMMatrixIdentity();
	}
}

DirectX::XMMATRIX Scene::GetMainCameraProjectionMatrix()
{
	if (mainCameraIndex < cameras.size()) {
		return cameras[mainCameraIndex]->GetProjectionMatrix();
	}
	else {
		std::cout << "Main camera index is out of range." << std::endl;
		return DirectX::XMMatrixIdentity();
	}
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

uint32_t Scene::AddMaterial(std::unique_ptr<DXMaterial> material)
{
	uint32_t id = materialIDCounter;
	materialMap[id] = std::move(material);
	materialIDCounter++;
	return id;
}



DXMaterial* Scene::GetMaterial(uint32_t materialID)
{
	auto it = materialMap.find(materialID);
	if (it != materialMap.end())
	{
		return it->second.get();
	}
	return nullptr;
}