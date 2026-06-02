#pragma once
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <windows.h>
#include <wrl.h>

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
	void MoveToNextFrame();

	static const int                  bufferCount = 2; // Double buffering
	ComPtr<ID3D12Device>              device;
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
};