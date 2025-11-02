#include "Scene.h"

Scene::Scene(float camwidth, float camheight, float camnear, float camfar)
	:objects(),
	rootObjects(),
	lights()
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
	objects.push_back(object);
	if (object->isRoot)
	{
		rootObjects.push_back(object);
	}
	
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