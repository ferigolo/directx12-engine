#pragma once
#include "Entity.h"
#include "Mesh.h"
#include "Pipeline.h"
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
	void Render();
	void Shutdown();

private:
	bool EnableDebugLayer();
	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateSwapChain(HWND hwnd, int width, int height);
	bool CreateDescriptorHeaps();
	bool CreateRenderTargets();
	bool CreateFence();
	bool CreateFactory();
	void MoveToNextFrame();
	inline void IncrementFenceAndWaitsForGpu();

	static const int                  bufferCount = 2; // Double buffering
	ComPtr<ID3D12Device>              device;
	ComPtr<IDXGIFactory4>             factory;
	ComPtr<ID3D12CommandQueue>        commandQueue;
	ComPtr<IDXGISwapChain3>           swapChain;
	ComPtr<ID3D12DescriptorHeap>      rtvHeap;
	ComPtr<ID3D12Resource>            renderTargets[bufferCount];
	ComPtr<ID3D12CommandAllocator>    commandAllocators[bufferCount];
	ComPtr<ID3D12GraphicsCommandList> commandList;

	// synchronization mechanisms
	ComPtr<ID3D12Fence>              fence;
	UINT64                           fenceValues[bufferCount] = { 1 };
	HANDLE                           fenceEvent;
	UINT                             frameIndex;
	UINT                             rtvDescriptorSize;

	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<Mesh> cubeMesh;
	std::unique_ptr<Mesh> triangleMesh;

	std::vector<std::unique_ptr<Entity>> sceneObjects;

	// View and projection
	const XMVECTOR eyePosition = XMVectorSet(0.0f, 1.0f, -3.0f, 1.0f); // Camera a little higher and back
	const XMVECTOR focusPoint = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	const XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

	const XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
	const XMMATRIX viewProjectionMatrix = viewMatrix * projectionMatrix;
};
