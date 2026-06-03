#include "Mesh.h"
#include <d3dx12.h>

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

bool Mesh::Initialize(ID3D12Device* device)
{
	Vertex triangleVertices[] = {
		// (X, Y, Z)                       // (R, G, B, A)
		{ {  0.0f,  0.25f, 0.0f },         { 1.0f, 0.0f, 0.0f, 1.0f } }, // Top (Red)
		{ {  0.25f, -0.25f, 0.0f },        { 0.0f, 1.0f, 0.0f, 1.0f } }, // Right base (Green)
		{ { -0.25f, -0.25f, 0.0f },        { 0.0f, 0.0f, 1.0f, 1.0f } }  // Left base (Blue)
	};
	const auto vertexBufferSize = sizeof(triangleVertices);

	// Buffer properties
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // CPU needs to write data from RAM to VRAM
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

	if (FAILED(device->CreateCommittedResource( // allocates memory at VRAM for the resource
											   &heapProps,
											   D3D12_HEAP_FLAG_NONE,
											   &bufferDesc,
											   D3D12_RESOURCE_STATE_GENERIC_READ, // CPU will write, GPU read
											   nullptr,
											   IID_PPV_ARGS(&vertexBuffer)
	))) return false;

	// Copying data into vertex buffer
	UINT8* pVertexDataBegin = nullptr;
	CD3DX12_RANGE readRange(0, 0); // We are not going to read, just write

	if (SUCCEEDED(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin))))
	{
		memcpy(pVertexDataBegin, triangleVertices, vertexBufferSize);
		vertexBuffer->Unmap(0, nullptr);
	}
	else return false;

	// Tell the GPU how to interpretate the memory block we've just created
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex); // Size of one vertex
	vertexBufferView.SizeInBytes = vertexBufferSize;

	return true;
}
