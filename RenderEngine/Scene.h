#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include "Object_win.h"
#include "Camera.h"
#include "DXMaterial.h"
#include "Light.h"
//#include "ModelLoader.h"
//#include "ResourceManager.h"

class Scene
{
public:
	Scene(float camwidth, float camheight, float camnear, float camfar);
	~Scene();
	std::vector<Object*> objects;
	std::vector<uint32_t> objectIDs;
	std::map<uint32_t, uint32_t> objectIDMap;    //objectID, index
	std::map<uint32_t, uint32_t> rootobjectIDMap;
	std::unordered_map<std::wstring, std::unique_ptr<Texture>> textureMap;
	std::unordered_map<uint32_t, std::unique_ptr<DXMaterial>> materialMap;

	//std::map<uint32_t, std::unique_ptr<DirectionalLight>> directionalLights;
	std::vector<std::unique_ptr<DirectionalLight>> directionalLights;
	std::vector<Camera*> cameras;
	size_t mainCameraIndex;

	void addCamera(Camera* camera);
	//void removeCamera(Camera* camera);
	bool removeCamera(Camera* camera);
	void addObject(Object* object);
	void removeObject(Object* object);
	bool addDirectionalLight(DirectionalLight light);
	void UpdateWorldTransforms();
	void UpdateWorldTransformsRecursive(Object* parentObject);

	bool changeMainCamera(size_t index);
	DirectX::XMMATRIX GetMainCameraViewMatrix();
	DirectX::XMMATRIX GetMainCameraProjectionMatrix();
	void AddTexture(const std::wstring& name, std::unique_ptr<Texture> texture);
	Texture* GetTexture(const std::wstring& filepath);
	uint32_t AddMaterial(std::unique_ptr<DXMaterial> material);
	DXMaterial* GetMaterial(uint32_t materialID);
	uint32_t GetObjectCount() const { return static_cast<uint32_t>(objects.size()); }


private:
	uint32_t objectIDCounter;
	uint32_t materialIDCounter;
	uint32_t directionalLightIDCounter;

};