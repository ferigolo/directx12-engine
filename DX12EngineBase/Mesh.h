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

	bool Initialize(ID3D12Device* device, Vertex* vertices, UINT vertexCount, uint16_t* indices, UINT indexCount);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return vertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const { return indexBufferView; }
	UINT GetIndexCount() const { return indexCount; }

private:
	ComPtr<ID3D12Resource>   vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	ComPtr<ID3D12Resource>   indexBuffer;
	D3D12_INDEX_BUFFER_VIEW  indexBufferView = {};
	UINT indexCount = 0;
};