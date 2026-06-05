#pragma once
#include "Mesh.h"
#include <DirectXMath.h>
#include <wrl.h>

using namespace DirectX;
using namespace Microsoft::WRL;

struct EntityConstants
{
	XMMATRIX wvp;
};

class Entity
{
public:
	Entity();
	~Entity();

	bool Initialize(ID3D12Device* device, Mesh* targetMesh);
	void Update(XMMATRIX viewProjectionMatrix) const;

	void SetPosition(float x, float y, float z) { position = { x, y, z }; }
	void SetRotation(float x, float y, float z) { rotation = { x, y, z }; }
	void SetScale(float x, float y, float z) { scale = { x, y, z }; }

	Mesh* GetMesh() const { return mesh; }
	D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferAddress() const { return constantBuffer->GetGPUVirtualAddress(); }

private:
	Mesh* mesh{};

	// Fisical state
	XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };

	ComPtr<ID3D12Resource> constantBuffer;
	UINT8* cbvDataBegin{};
};

