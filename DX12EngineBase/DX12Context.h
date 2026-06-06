#pragma once
#include "Entity.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Scene.h"
#include <chrono>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <dxgi1_6.h>

using Microsoft::WRL::ComPtr;

class DX12Context
{
public:
	DX12Context();
	~DX12Context();

	bool Initialize(HWND hwnd, int width, int height);
	void OnResize(int newWidth, int newHeight);
	void Render();
	void Shutdown();

private:
	bool EnableDebugLayer();
	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateSwapChain(HWND hwnd);
	bool CreateDescriptorHeaps();
	bool CreateRenderTargets();
	bool CreateFence();
	bool CreateFactory();
	bool CreateDepthStencil();
	void MoveToNextFrame();
	inline void IncrementFenceAndWaitsForGpu();
	static float UpdateTimer();
	void SetBuffersAndDrawIndexedInstanced(Entity* obj);

	static const int                  bufferCount = 2; // Double buffering
	ComPtr<ID3D12Device>              device;
	ComPtr<IDXGIFactory4>             factory;
	ComPtr<ID3D12CommandQueue>        commandQueue;
	ComPtr<IDXGISwapChain3>           swapChain;
	ComPtr<ID3D12DescriptorHeap>      rtvHeap;
	ComPtr<ID3D12Resource>            renderTargets[bufferCount];
	ComPtr<ID3D12CommandAllocator>    commandAllocators[bufferCount];
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12DescriptorHeap>      dsvHeap; // Depth Stencil View Heap
	ComPtr<ID3D12Resource>            depthStencilBuffer; // Depth texture

	// Synchronization mechanisms
	ComPtr<ID3D12Fence>              fence;
	UINT64                           fenceValues[bufferCount] = { 1 };
	HANDLE                           fenceEvent;
	UINT                             frameIndex;
	UINT                             rtvDescriptorSize;

	int clientWidth = 1080;
	int clientHeight = 1920;

	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<Mesh> cubeMesh;
	std::unique_ptr<Mesh> triangleMesh;
	std::unique_ptr<Mesh> gridMesh;

	Scene mainScene;
};
