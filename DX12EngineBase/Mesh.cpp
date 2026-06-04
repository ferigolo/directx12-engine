#include "Mesh.h"
#include <d3dx12.h>

Mesh::Mesh()
{}

Mesh::~Mesh()
{}

bool Mesh::Initialize(ID3D12Device* device, Vertex* vertices, UINT vertexCount, uint16_t* indexes, UINT indexCount)
{
	const auto vertexBufferSize = vertexCount * sizeof(Vertex);

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

	if (SUCCEEDED(vertexBuffer->Map(0, &readRange, (void**)(&pVertexDataBegin))))
	{
		memcpy(pVertexDataBegin, vertices, vertexBufferSize);
		vertexBuffer->Unmap(0, nullptr);
	}
	else return false;

	// Tell the GPU how to interpretate the memory block we've just created
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex); // Size of one vertex
	vertexBufferView.SizeInBytes = vertexBufferSize;

	// Index buffer
	const auto indexBufferSize = indexCount * sizeof(uint16_t);
	auto ibDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);

	device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &ibDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));

	UINT8* pIndexDataBegin = nullptr; // Points to address which starts the index buffer
	indexBuffer->Map(0, &readRange, (void**)&pIndexDataBegin);
	memcpy(pIndexDataBegin, indexes, sizeof(indexes));
	indexBuffer->Unmap(0, nullptr);

	// Describes the index buffer to GPU
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	indexBufferView.SizeInBytes = indexBufferSize;

	return true;
}
