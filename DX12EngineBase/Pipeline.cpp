#include "d3dx12.h"
#include "Pipeline.h"
#pragma comment(lib, "d3dcompiler.lib")

Pipeline::Pipeline() {}
Pipeline::~Pipeline() {}

bool Pipeline::CreateRootSignature(ID3D12Device* device)
{
	// Data interface that describes to the GPU which variables the Shaders will have to read
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init(
		0, nullptr, // Zero params
		0, nullptr, // Zero 'Static Samplers' (None sample configured)
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ComPtr<ID3DBlob> signatureBlob, // Where the serilized bytecode will live
		errorBlob;

	// Serialize the Root Signature so the GPU understands the code
	if (FAILED(D3D12SerializeRootSignature( // Translates the signature into bytecode
										   &rootSignatureDesc,
										   D3D_ROOT_SIGNATURE_VERSION_1,
										   &signatureBlob,
										   &errorBlob
	)))
	{
		if (errorBlob)
		{
			const char* errorMessage = static_cast<const char*>(errorBlob->GetBufferPointer());
			OutputDebugStringA("\n[ROOT SIGNATURE ERROR]:\n");
			OutputDebugStringA(errorMessage + '\n');
		};
		return false;
	};

	return SUCCEEDED(device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature)
	));
}

bool Pipeline::CreatePipelineState(ID3D12Device* device)
{
	ComPtr<ID3DBlob> vertexShader, pixelShader, errorBlob;

	// Compiles pixelShader from file
	if (FAILED(D3DCompileFromFile(L"C:/Users/ferig/source/repos/DX12EngineBase/Shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShader, &errorBlob)))
	{
		if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return false;
	}

	// Compiles vertexShader from file
	if (FAILED(D3DCompileFromFile(L"C:/Users/ferig/source/repos/DX12EngineBase/Shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexShader, &errorBlob)))
	{
		if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return false;
	}

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		// "POSITION": Starts at byte 0 of struct, 3 floats (RGB32)
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		// "COLOR": Starts at byte 12 of struct, 4 floats (RGBA32)
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Mounting the PSO package
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get()); // Insere o Vertex Shader binário
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());

	// Default draw config for rasterizing and color blending
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE; // No depth compute
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	return SUCCEEDED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
}
