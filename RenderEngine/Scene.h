#pragma once
#include <vector>
#include "Object_win.h"
#include "Camera.h"

class Scene
{
public:
	Scene(float camwidth, float camheight, float camnear, float camfar);
	~Scene();
	std::vector<Object*> objects;
	std::vector<Object*> rootObjects;
	std::vector<Object*> lights;
	std::vector<Camera*> cameras;
	size_t mainCameraIndex;

	void addCamera(Camera* camera);
	void addObject(Object* object);
	bool removeCamera(Camera* camera);
	bool changeMainCamera(size_t index);

};