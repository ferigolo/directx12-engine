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
	if (!CreateDevice()) return false;
	if (!CreateCommandQueue()) return false;
	if (!CreateSwapChain(hwnd, width, height)) return false;
	if (!CreateDescriptorHeaps()) return false;
	if (!CreateRenderTargets()) return false;
	if (!CreateFence()) return false;

	return true;
}

void DX12Context::Render()
{
	auto& commandAllocator = commandAllocators[frameIndex];
	commandAllocator->Reset();
	commandList->Reset(commandAllocator.Get(), nullptr);

	auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(
		renderTargets[frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	commandList->ResourceBarrier(1, &barrierToRT);

	// Gets the pointer for the current buffer and cleans the screen
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.ptr += frameIndex * rtvDescriptorSize;

	const float clearColor[] = { 0.08f, 0.12f, 0.18f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
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
	ComPtr<IDXGIFactory4> factory;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return false;
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
	// D3D12_COMMAND_LIST_TYPE_COPY -> heavy data loading from RAM to VRAM throw PCIe
	if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)))) return false;

	for (UINT i = 0; i < bufferCount; i++)
		if (FAILED(device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&commandAllocators[i])))) return false; // Memory space to the command list write commands (writes into it)

	if (FAILED(device->CreateCommandList(0, queueDesc.Type, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList)))) return false;

	commandList->Close();
	return true;
}

bool DX12Context::CreateSwapChain(HWND hwnd, int width, int height)
{
	ComPtr<IDXGIFactory4> factory;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return false;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Modelo de flip moderno
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
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)))) return false;
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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

void DX12Context::MoveToNextFrame()
{
	const auto currentFanceValue = fenceValues[frameIndex];
	commandQueue->Signal(fence.Get(), currentFanceValue);

	frameIndex = swapChain->GetCurrentBackBufferIndex(); // gets which is the next frame (which buffer is the backbuffer now)

	if (fence->GetCompletedValue() < fenceValues[frameIndex])
	{
		fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE); // Waits for GPU to stop using the allocator the CPU wants to write
	}
	fenceValues[frameIndex] = currentFanceValue + 1;
}