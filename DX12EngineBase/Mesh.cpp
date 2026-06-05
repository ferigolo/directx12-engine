#include "Mesh.h"
#include <d3dx12.h>

Mesh::Mesh()
{}

Mesh::~Mesh()
{}

bool Mesh::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Vertex* vertices, UINT vertexCount, uint16_t* indexes, UINT indexCount)
{
	this->indexCount = indexCount;
	const UINT vertexBufferSize = vertexCount * sizeof(Vertex);
	const UINT indexBufferSize = indexCount * sizeof(uint16_t);

	// Copying data: Upload buffer (RAM) -> Default buffer (VRAM)
	if (!CreateDefaultBuffer(device, commandList, vertices, vertexBufferSize, vertexBuffer, vertexBufferUploader)) return false;
	if (!CreateDefaultBuffer(device, commandList, indexes, indexBufferSize, indexBuffer, indexBufferUploader)) return false;

	// Tell the GPU how to interpretate the memory block we've just created
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex); // Size of one vertex
	vertexBufferView.SizeInBytes = vertexBufferSize;

	// Describes the index buffer to GPU
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	indexBufferView.SizeInBytes = indexBufferSize;

	return true;
}

bool Mesh::CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& defaultBuffer, ComPtr<ID3D12Resource>& uploadBuffer)
{
	// Final buffer on VRAM
	auto defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);

	if (FAILED(device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&defaultBuffer)))) return false;

	// Upload buffer on RAM
	auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	if (FAILED(device->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)))) return false;

	// CPU copies data into Upload buffer
	UINT8* pData{};
	uploadBuffer->Map(0, nullptr, (void**)&pData);
	memcpy(pData, initData, byteSize);
	uploadBuffer->Unmap(0, nullptr);

	commandList->CopyBufferRegion(defaultBuffer.Get(), 0, uploadBuffer.Get(), 0, byteSize);

	// Transition from destiny of copy to readble buffer
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandList->ResourceBarrier(1, &barrier);

	return true;
}
