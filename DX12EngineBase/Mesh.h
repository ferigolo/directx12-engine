#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct Vertex
{
	XMFLOAT3 position; // X, Y, Z
	XMFLOAT4 color;    // R, G, B, A
};

class Mesh
{
public:
	Mesh();
	~Mesh();

	bool Initialize(ID3D12Device* device);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return vertexBufferView; }

private:
	ComPtr<ID3D12Resource>   vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
};