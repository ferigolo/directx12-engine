#include "Entity.h"
#include <d3dx12.h>

using namespace DirectX;

Entity::Entity()
{}

Entity::~Entity()
{}

bool Entity::Initialize(ID3D12Device* device, Mesh* targetMesh)
{
	this->mesh = targetMesh;

	// Creates the constant buffer for each object
	const UINT cbSize = (sizeof(EntityConstants) + 255) & ~255;
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);

	if (FAILED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffer)))) return false;

	// Keeps the pointer open for writing the matriz every frame
	CD3DX12_RANGE readRange(0, 0);
	constantBuffer->Map(0, &readRange, (void**)(&cbvDataBegin));

	return true;
}

void Entity::Update(XMMATRIX viewProjectionMatrix) const
{
	XMMATRIX scaleMat = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	XMMATRIX transMat = XMMatrixTranslation(position.x, position.y, position.z);

	XMMATRIX worldMatrix = scaleMat * rotMat * transMat;
	XMMATRIX wvpMatrix = worldMatrix * viewProjectionMatrix;

	// DirectXMath (CPU) = ROW-MAJOR [l1, l2, l3, ...]
	// HLSL (GPU) = COLUMN-MAJOR     [c1, c2, c3, ...]
	// XMMatrixTranspose: ROW-MAJOR -> COLUMN-MAJOR

	EntityConstants ec{ XMMatrixTranspose(wvpMatrix) };
	memcpy(cbvDataBegin, &ec, sizeof(EntityConstants));
}
