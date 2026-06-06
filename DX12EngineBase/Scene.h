#pragma once
#include "Camera.h"
#include "Entity.h"
#include <memory>
#include <vector>

using namespace std;

class Scene
{
public:
	Scene();
	~Scene();

	void AddObject(unique_ptr<Entity> obj) { sceneObjects.push_back(move(obj)); };
	void SetGrid(unique_ptr<Entity> grid) { gridObj = move(grid); };

	void Update(float deltaTime);

	vector<unique_ptr<Entity>>& GetObjects() { return sceneObjects; }
	Entity* GetGrid() { return gridObj.get(); }
	Camera& GetCamera() { return camera; }

private:
	vector<unique_ptr<Entity>> sceneObjects;
	unique_ptr<Entity> gridObj;
	Camera camera;
};

