#include "Camera.h"
using namespace DirectX;

Camera::Camera() : position(0.0f, 1.0f, -3.0f), pitch(0.0f), yaw(0.0f)
{}
Camera::~Camera()
{}

void Camera::Move(float forward, float right)
{
	XMVECTOR posVec = XMLoadFloat3(&position);
	posVec += GetForwardVector() * forward;
	posVec += GetRightVector() * right;

	XMStoreFloat3(&position, posVec);
}

void Camera::Rotate(float pitch, float yaw)
{
	this->pitch += pitch;
	this->yaw += yaw;

	if (this->pitch > XM_PIDIV2 - 0.01f) this->pitch = XM_PIDIV2 - 0.01f;
	if (this->pitch < -XM_PIDIV2 + 0.01f) this->pitch = -XM_PIDIV2 + 0.01f;
}

XMMATRIX Camera::GetViewMatrix() const
{
	XMVECTOR posVec = XMLoadFloat3(&position),
		forward = GetForwardVector(),
		lookAt = posVec + forward;

	return XMMatrixLookAtLH(posVec, forward, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

XMMATRIX Camera::GetProjectionMatrix(float fov, float aspectRatio, float nearZ, float farZ) const
{
	return XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), aspectRatio, nearZ, farZ);
}

XMVECTOR Camera::GetForwardVector() const
{
	XMVECTOR forward = XMVectorSet(
		sinf(yaw) * cosf(pitch),
		sinf(pitch),
		cosf(yaw) * cosf(pitch),
		0.0f
	);
	return XMVector3Normalize(forward);
}

XMVECTOR Camera::GetRightVector() const
{
	XMVECTOR forward = GetForwardVector();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	return XMVector3Normalize(XMVector3Cross(up, forward));
}
