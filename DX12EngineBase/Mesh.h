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

class Mesh // VRAM wrapper
{          // Handles the Vertex and Index buffers life-cicles
public:
	Mesh();
	~Mesh();

	bool Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Vertex* vertices, UINT vertexCount, uint16_t* indices, UINT indexCount);
	// Clear the temp memory out of RAM because data will live exclusive on VRAM (Default buffers)
	void DisposeUploadBuffers();

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return vertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const { return indexBufferView; }
	UINT GetIndexCount() const { return indexCount; }

private:
	ComPtr<ID3D12Resource>   vertexBuffer;
	ComPtr<ID3D12Resource>   vertexBufferUploader;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

	ComPtr<ID3D12Resource>   indexBuffer;
	ComPtr<ID3D12Resource>   indexBufferUploader;
	D3D12_INDEX_BUFFER_VIEW  indexBufferView = {};
	UINT indexCount = 0;

	bool CreateDefaultBuffer(ID3D12Device* device,
		ID3D12GraphicsCommandList* commandList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& defaultBuffer, ComPtr<ID3D12Resource>& uploadBuffer);
};