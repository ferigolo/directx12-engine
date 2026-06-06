#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
	Camera();
	~Camera();

	void Move(float forward, float right);
	void Rotate(float pitch, float yaw);

	XMMATRIX GetViewMatrix() const;
	XMMATRIX GetProjectionMatrix(float fov, float aspectRatio, float nearZ, float farZ) const;
private:
	XMFLOAT3 position;
	float pitch; // Rotation X (up/down)
	float yaw;   // Rotation Y (left/right)

	XMVECTOR GetForwardVector() const;
	XMVECTOR GetRightVector() const;
};

