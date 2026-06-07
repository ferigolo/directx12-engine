#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class Pipeline
{
public:
	Pipeline(int sampleDescCount = 4);
	~Pipeline();

	bool CreateRootSignature(ID3D12Device* device);
	bool CreatePipelineState(ID3D12Device* device); // Compiles the shaders and mounts the pipeline package

	ID3D12RootSignature* GetRootSignature() const { return rootSignature.Get(); }
	ID3D12PipelineState* GetPipelineState() const { return pipelineState.Get(); } // NOVO
private:
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pipelineState;
	const int                   sampleDescCount;
};