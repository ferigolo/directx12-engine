#include "DX12Context.h"
#include "GeometryGenerator.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

DX12Context::DX12Context() : fenceEvent(nullptr), frameIndex(0), rtvDescriptorSize(0) {}
DX12Context::~DX12Context() { Shutdown(); }

bool DX12Context::Initialize(HWND hwnd, int width, int height)
{
#if defined(_DEBUG)
	EnableDebugLayer(); // Ativa avisos detalhados de erro da GPU no console do VS
#endif
	if (!CreateDevice()) return false;
	if (!CreateCommandQueue()) return false;
	if (!CreateFactory()) return false;
	if (!CreateSwapChain(hwnd, width, height)) return false;
	if (!CreateDescriptorHeaps()) return false;
	if (!CreateRenderTargets()) return false;
	if (!CreateFence()) return false;

	pipeline = std::make_unique<Pipeline>();
	if (!pipeline->CreateRootSignature(device.Get())) return false;
	if (!pipeline->CreatePipelineState(device.Get())) return false;

	commandAllocators[0]->Reset();
	commandList->Reset(commandAllocators[0].Get(), pipeline->GetPipelineState());

	// Load meshes into GPU
	// Each one gets its Upload and Default Buffers
	MeshData cubeData = GeometryGenerator::CreateCube();
	cubeMesh = std::make_unique<Mesh>();
	if (!cubeMesh->Initialize(device.Get(), commandList.Get(), cubeData.Vertices.data(), static_cast<UINT>(cubeData.Vertices.size()), cubeData.Indexes.data(), static_cast<UINT>(cubeData.Indexes.size()))) return false;

	MeshData triangleData = GeometryGenerator::CreateTriangle();
	triangleMesh = std::make_unique<Mesh>();
	if (!triangleMesh->Initialize(device.Get(), commandList.Get(), triangleData.Vertices.data(), static_cast<UINT>(triangleData.Vertices.size()), triangleData.Indexes.data(), static_cast<UINT>(triangleData.Indexes.size()))) return false;

	MeshData gridData = GeometryGenerator::CreateGrid();
	gridMesh = std::make_unique<Mesh>();
	if (!gridMesh->Initialize(device.Get(), commandList.Get(), gridData.Vertices.data(), static_cast<UINT>(gridData.Vertices.size()), gridData.Indexes.data(), static_cast<UINT>(gridData.Indexes.size()))) return false;

	commandList->Close();
	ID3D12CommandList* cmdsLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	IncrementFenceAndWaitsForGpu();

	// Create world
	// Cube at the center
	auto cubeObj = std::make_unique<Entity>();
	if (!cubeObj->Initialize(device.Get(), cubeMesh.get())) return false;
	cubeObj->SetPosition(0.0f, 1.0f, 0.0f);
	sceneObjects.push_back(std::move(cubeObj));

	// Triangle to the left
	auto triangle1 = std::make_unique<Entity>();
	if (!triangle1->Initialize(device.Get(), triangleMesh.get())) return false;
	triangle1->SetPosition(-1.0f, 1.0f, 0.0f);
	sceneObjects.push_back(std::move(triangle1));

	// Triangle to the right
	auto triangle2 = std::make_unique<Entity>();
	if (!triangle2->Initialize(device.Get(), triangleMesh.get())) return false;
	triangle2->SetPosition(1.0f, 1.5f, 1.5f);
	sceneObjects.push_back(std::move(triangle2));

	this->gridObj = std::make_unique<Entity>();
	if (!gridObj->Initialize(device.Get(), gridMesh.get())) return false;
	gridObj->SetPosition(0.0f, -1.0f, 0.0f);
	gridObj->Update(viewProjectionMatrix);

	return true;
}

void DX12Context::Render()
{
	float rotationTimer = UpdateTimer();

	for (auto& obj : sceneObjects)
	{
		obj->Update(viewProjectionMatrix);
		obj->SetRotation(rotationTimer, rotationTimer, rotationTimer * 0.5f);
	}

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

	const FLOAT clearColor[] = { 0.0f, 0.0f, 0.15f, 0.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr); // Clear the next buffer
	// Set the next buffer to be the render target
	// Everything the shaders outputs goes in here
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Set render area (full window)
	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f };
	D3D12_RECT scissorRect = { 0, 0, 1280, 720 };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
	commandList->SetPipelineState(pipeline->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& obj : sceneObjects)
		SetBuffersAndDrawIndexedInstanced(obj);

	SetBuffersAndDrawIndexedInstanced(gridObj);

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

bool DX12Context::CreateSwapChain(HWND hwnd, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32 bits, 8 for each channel
	// UNORM->Unsined Normalized(color values between 0.0 and 1.0)
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Changes the buffer is pointing at and discard its content
	swapChainDesc.SampleDesc.Count = 1;

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
	fenceValues[frameIndex] = 1; // Valeu for the first frame
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	return fenceEvent != nullptr;
}

bool DX12Context::CreateFactory()
{
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return false;
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
	using namespace std::chrono;
	static auto prevTime = high_resolution_clock::now();
	auto currentTime = high_resolution_clock::now();

	float deltaTime = duration<float>(currentTime - prevTime).count(); // How many seconds has passed since last frame
	prevTime = currentTime;

	static float rotationTimer = 0;
	static const float rotationSpeed = 1;
	rotationTimer += rotationSpeed * deltaTime;

	return rotationTimer;
}

void DX12Context::SetBuffersAndDrawIndexedInstanced(std::unique_ptr<Entity>& obj)
{
	commandList->SetGraphicsRootConstantBufferView(0, obj->GetConstantBufferAddress());

	D3D12_VERTEX_BUFFER_VIEW vbv = obj->GetMesh()->GetVertexBufferView();
	commandList->IASetVertexBuffers(0, 1, &vbv);

	D3D12_INDEX_BUFFER_VIEW ibv = obj->GetMesh()->GetIndexBufferView();
	commandList->IASetIndexBuffer(&ibv);

	commandList->DrawIndexedInstanced(obj->GetMesh()->GetIndexCount(), 1, 0, 0, 0);
}
