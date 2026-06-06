#include "Input.h"
#include "Scene.h"

using namespace DirectX;

Scene::Scene()
{}

Scene::~Scene()
{}

void Scene::Update(float deltaTime, int clientWidth, int clientHeight)
{
	float moveSpeed = 5.0f * deltaTime;
	float rotSpeed = 2.0f * deltaTime;

	if (Input::IsKeyDown('W')) camera.Move(moveSpeed, 0.0f);
	if (Input::IsKeyDown('S')) camera.Move(-moveSpeed, 0.0f);
	if (Input::IsKeyDown('D')) camera.Move(0.0f, moveSpeed);
	if (Input::IsKeyDown('A')) camera.Move(0.0f, -moveSpeed);

	if (Input::IsKeyDown(VK_UP))    camera.Rotate(rotSpeed, 0.0f);
	if (Input::IsKeyDown(VK_DOWN))  camera.Rotate(-rotSpeed, 0.0f);
	if (Input::IsKeyDown(VK_RIGHT)) camera.Rotate(0.0f, rotSpeed);
	if (Input::IsKeyDown(VK_LEFT))  camera.Rotate(0.0f, -rotSpeed);

	float aspectRatio = (clientHeight == 0) ? 1.0f : (float)(clientWidth) / (float)(clientHeight);

	XMMATRIX viewProj = camera.GetViewMatrix() * camera.GetProjectionMatrix(45.0f, aspectRatio, 0.1f, 100.0f);

	static float rotationTimer = 0;
	rotationTimer += 1 * deltaTime; // Speed of rotation

	if (sceneObjects.size() >= 3)
	{
		sceneObjects[0]->SetRotation(rotationTimer, rotationTimer, rotationTimer * 0.5f);
		sceneObjects[1]->SetRotation(0.0f, rotationTimer, 0.0f);
		sceneObjects[2]->SetRotation(rotationTimer, 0.0f, 0.0f);
	}

	for (auto& obj : sceneObjects) obj->Update(viewProj);

	if (gridObj) gridObj->Update(viewProj);
}
