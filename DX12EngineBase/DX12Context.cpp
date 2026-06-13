#include "DX12Context.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

DX12Context::DX12Context() : fenceEvent(nullptr), frameIndex(0), rtvDescriptorSize(0) {}
DX12Context::~DX12Context() { Shutdown(); }

bool DX12Context::Initialize(HWND hwnd, int width, int height)
{
#if defined(_DEBUG)
	EnableDebugLayer(); // Ativa avisos detalhados de erro da GPU no console do VS
#endif
	clientWidth = width;
	clientHeight = height;

	if (!CreateDevice()) return false;
	if (!CreateCommandQueue()) return false;
	if (!CreateFactory()) return false;
	if (!CreateSwapChain(hwnd)) return false;
	if (!CreateDescriptorHeaps()) return false;
	if (!CreateRenderTargets()) return false;
	if (!CreateDepthStencil()) return false;
	if (!CreateFence()) return false;

	pipeline = std::make_unique<Pipeline>(sampleDescCount);
	if (!pipeline->CreateRootSignature(device.Get())) return false;
	if (!pipeline->CreatePipelineState(device.Get())) return false;

	commandAllocators[0]->Reset();
	commandList->Reset(commandAllocators[0].Get(), pipeline->GetPipelineState());

	// Load meshes into GPU
	// Each one gets its Upload and Default Buffers
	if (!AllocateMesh(GeometryGenerator::CreateCube(), "cube")) return false;
	if (!AllocateMesh(GeometryGenerator::CreateTriangle(), "triangle")) return false;
	if (!AllocateMesh(GeometryGenerator::CreateCylinder(0.25f, 0.25f, 1.0f, 40, 2), "cylinder")) return false;
	if (!AllocateMesh(GeometryGenerator::CreateSphere(0.25f, 20, 20), "sphere")) return false;
	if (!AllocateMesh(GeometryGenerator::CreateIcosphere(0.25f, 6U), "icosphere")) return false;
	if (!AllocateMesh(GeometryGenerator::CreateGrid(), "grid")) return false;

	commandList->Close();
	ID3D12CommandList* cmdsLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	IncrementFenceAndWaitsForGpu();

	for (auto& pair : meshMap)
		pair.second->DisposeUploadBuffers(); // GPU had already coppied the data from vertex and index buffers, so we clean them

	// Create world
	if (!CreateEntity("cube", XMFLOAT3{ 0.0f, 1.0f, -0.5f })) return false; // Cube at the center
	if (!CreateEntity("triangle", XMFLOAT3{ -0.5f, 0.5f, -1.0f })) return false; // Triangle to the left
	if (!CreateEntity("triangle", XMFLOAT3{ 1.0f, 0.0f, 0.0f })) return false; // Triangle to the right
	if (!CreateEntity("cylinder", XMFLOAT3{ 0.0f, 0.0f, -2.0f })) return false;
	if (!CreateEntity("sphere", XMFLOAT3{ 0.0f, 1.5f, 1.0f })) return false;
	if (!CreateEntity("icosphere", XMFLOAT3{ -1.0f, 1.0f, -2.0f })) return false;
	if (!CreateEntity("grid", XMFLOAT3{ 0.0f, -2.0f, 0.0f }, true)) return false;
	return true;
}

bool DX12Context::AllocateMesh(const MeshData& meshData, std::string meshName)
{
	auto mesh = std::make_unique<Mesh>();

	if (!mesh->Initialize(
		device.Get(),
		commandList.Get(),
		(Vertex*)meshData.Vertices.data(),
		static_cast<UINT>(meshData.Vertices.size()),
		(uint16_t*)meshData.Indexes.data(),
		static_cast<UINT>(meshData.Indexes.size())
	)) return false;

	meshMap[meshName] = std::move(mesh);
	return true;
}

bool DX12Context::CreateEntity(std::string name, XMFLOAT3 position, bool isGrid)
{
	auto entity = std::make_unique<Entity>();
	if (!entity->Initialize(device.Get(), meshMap[name].get())) return false;
	entity->SetPosition(position);
	isGrid ? mainScene.SetGrid(std::move(entity)) : mainScene.AddObject(std::move(entity));
	return true;
}

void DX12Context::Render()
{
	mainScene.Update(UpdateTimer(), clientWidth, clientHeight);

	auto& commandAllocator = commandAllocators[frameIndex];
	commandAllocator->Reset(); // Erases the content in the allocator memory
	commandList->Reset(commandAllocator.Get(), nullptr);

	auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition( // prepare resource to be the render target
															renderTargets[frameIndex].Get(),
															D3D12_RESOURCE_STATE_PRESENT,
															D3D12_RESOURCE_STATE_RENDER_TARGET
	); // Transition it from presenting to painting (render target)
	commandList->ResourceBarrier(1, &barrierToRT); // Append the transition command

	// Gets the pointers for the buffers
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart()); // Pointer for the first item of buffers list
	rtvHandle.ptr += frameIndex * rtvDescriptorSize; // Gets address of next buffer to render on

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();

	static const FLOAT clearColor[] = { 0.0f, 0.0f, 0.15f, 0.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr); // Clear the next buffer
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	// Set the next buffer to be the render target
	// Everything the shaders outputs goes in here
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Set render area (full window)
	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, clientWidth, clientHeight, 0.0f, 1.0f };
	D3D12_RECT scissorRect = { 0, 0, clientWidth, clientHeight };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
	commandList->SetPipelineState(pipeline->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& obj : mainScene.GetObjects()) SetBuffersAndDrawIndexedInstanced(obj.get());
	if (mainScene.GetGrid()) SetBuffersAndDrawIndexedInstanced(mainScene.GetGrid());

	auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition( // prepare resource to present its content
																 renderTargets[frameIndex].Get(),
																 D3D12_RESOURCE_STATE_RENDER_TARGET,
																 D3D12_RESOURCE_STATE_PRESENT
	);
	commandList->ResourceBarrier(1, &barrierToPresent);

	commandList->Close();

	// Send commands to the GPU
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Swap the frames
	swapChain->Present(1, 0);

	MoveToNextFrame();
}

void DX12Context::Shutdown()
{
	if (fenceEvent != nullptr)
	{
		CloseHandle(fenceEvent);
		fenceEvent = nullptr;
	}
}

bool DX12Context::EnableDebugLayer()
{
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		return true;
	}
	return false;
}

bool DX12Context::CreateDevice() // represents the GPU in the code
{                                // used to create everything else
	// Creates the device pointing to the system main GPU
	if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) return false;
	return true;
}

bool DX12Context::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {}; // Descriptor -> configuration structure
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Graphic, math and copy commands focused queue
	// D3D12_COMMAND_LIST_TYPE_COMPUTE -> heavy math commands, compute shaders, simulations, AI
	// D3D12_COMMAND_LIST_TYPE_COPY -> heavy data loading from RAM to VRAM through PCIe
	if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)))) return false;

	for (UINT i = 0; i < bufferCount; i++)
		if (FAILED(device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&commandAllocators[i])))) return false; // Memory space for command list to write commands

	if (FAILED(device->CreateCommandList(0, queueDesc.Type, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList)))) return false;

	commandList->Close();
	return true;
}

bool DX12Context::CreateSwapChain(HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Width = clientWidth;
	swapChainDesc.Height = clientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32 bits, 8 for each channel
	// UNORM->Unsined Normalized(color values between 0.0 and 1.0)
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Changes the buffer is pointing at and discard its content
	swapChainDesc.SampleDesc.Count = sampleDescCount;

	ComPtr<IDXGISwapChain1> swapChain;
	if (FAILED(factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain))) return false;

	swapChain.As(&this->swapChain);
	frameIndex = this->swapChain->GetCurrentBackBufferIndex();
	return true;
}

bool DX12Context::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = bufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)))) return false;
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // Size of the buffer in VRAM (we calculate the address of each buffer by frameIndex * rtvDescriptorSize)
	return true;
}

bool DX12Context::CreateRenderTargets()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < bufferCount; i++)
	{
		if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])))) return false;
		device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += rtvDescriptorSize;
	}
	return true;
}

bool DX12Context::CreateFence()
{
	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) return false;
	for (UINT i = 0; i < bufferCount; i++)
		fenceValues[i] = 0; // Current value for each frame
	fenceValues[frameIndex] = 1; // Value for the first frame
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	return fenceEvent != nullptr;
}

bool DX12Context::CreateFactory()
{
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return false;
	return true;
}

bool DX12Context::CreateDepthStencil()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)))) return false;

	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = clientWidth;
	depthStencilDesc.Height = clientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT; // 32-bits for high depth precision
	depthStencilDesc.SampleDesc.Count = sampleDescCount;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // Explicity tells the GPU to write depth data 

	D3D12_CLEAR_VALUE optClear = {};
	optClear.Format = DXGI_FORMAT_D32_FLOAT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT); // Pure VRAM (CPU will not write here)
	if (FAILED(device->CreateCommittedResource( // Allocates memory for depthStencilBuffer
											   &heapProps,
											   D3D12_HEAP_FLAG_NONE,
											   &depthStencilDesc,
											   D3D12_RESOURCE_STATE_DEPTH_WRITE,
											   &optClear,
											   IID_PPV_ARGS(&depthStencilBuffer)
	))) return false;

	device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	// The CPU allocates a block of raw bytes in VRAM(Resource).
	//	The CPU allocates a contiguous array of memory reserved for storing metadata(Descriptor Heap).
	//	The CPU invokes a View function to generate the physical structure of the access metadata (the Descriptor) and writes this structure to a predetermined index within the Heap.
	//	CommandList commands pass the memory offset from the Heap to the GPU, instructing the graphics processor cores to read the correct descriptor before operating on the raw resource.

	return true;
}

void DX12Context::MoveToNextFrame()
{
	const auto currentFanceValue = fenceValues[frameIndex];
	commandQueue->Signal(fence.Get(), currentFanceValue);

	frameIndex = swapChain->GetCurrentBackBufferIndex(); // Gets next frame buffer

	if (fence->GetCompletedValue() < fenceValues[frameIndex])
	{
		fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE); // Waits for GPU stop using allocator the CPU wants to write
	}
	fenceValues[frameIndex] = currentFanceValue + 1;
}

inline void DX12Context::IncrementFenceAndWaitsForGpu()
{
	fenceValues[frameIndex]++;
	commandQueue->Signal(fence.Get(), fenceValues[frameIndex]);
	if (fence->GetCompletedValue() < fenceValues[frameIndex])
	{
		fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

float DX12Context::UpdateTimer()
{
	using namespace chrono;
	static auto prevTime = high_resolution_clock::now();
	auto currentTime = high_resolution_clock::now();

	float deltaTime = duration<float>(currentTime - prevTime).count(); // How many seconds has passed since last frame
	prevTime = currentTime;

	return deltaTime;
}

void DX12Context::SetBuffersAndDrawIndexedInstanced(Entity* obj)
{
	commandList->SetGraphicsRootConstantBufferView(0, obj->GetConstantBufferAddress());

	D3D12_VERTEX_BUFFER_VIEW vbv = obj->GetMesh()->GetVertexBufferView();
	commandList->IASetVertexBuffers(0, 1, &vbv);

	D3D12_INDEX_BUFFER_VIEW ibv = obj->GetMesh()->GetIndexBufferView();
	commandList->IASetIndexBuffer(&ibv);

	commandList->DrawIndexedInstanced(obj->GetMesh()->GetIndexCount(), 1, 0, 0, 0);
}

void DX12Context::OnResize(int newWidth, int newHeight)
{
	if (device == nullptr || swapChain == nullptr || commandAllocators[0] == nullptr) return;

	clientWidth = newWidth;
	clientHeight = newHeight;

	IncrementFenceAndWaitsForGpu(); // CPU need to wait for GPU to finish the current frame

	// Clear buffers
	for (int i = 0; i < bufferCount; i++) renderTargets[i].Reset();
	depthStencilBuffer.Reset();

	if (FAILED(swapChain->ResizeBuffers(
		bufferCount,
		clientWidth,
		clientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0))) return;

	frameIndex = swapChain->GetCurrentBackBufferIndex();

	CreateRenderTargets();
	CreateDepthStencil();
}
