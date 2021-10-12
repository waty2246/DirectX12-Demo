#include "stdafx.h"
#include "config.h"
#include "D3D.h"

#include "ColorShader.h"


ColorShader::ColorShader()
{
}

ColorShader::~ColorShader()
{
}


void ColorShader::Initialize(D3D* d3d)
{
	m_d3d = d3d;
	auto device = d3d->GetDevice();

	// Initialize the vertex and pixel shaders.
	InitializeShader(device, GetShaderFilePath("color_vs.hlsl"), GetShaderFilePath("color_ps.hlsl"));
}

void ColorShader::Render(ID3D12GraphicsCommandList* commandList, int32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	// Set the shader parameters that it will use for rendering.
	SetShaderParameters(commandList, worldMatrix, viewMatrix, projectionMatrix);
	
	// Now render the prepared buffers with the shader.
	RenderShader(commandList, indexCount);
}


void ColorShader::InitializeShader(ID3D12Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	// Create root signature match with color shader.
	{
		CD3DX12_ROOT_PARAMETER parameter;
		parameter.InitAsConstantBufferView(0);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(1, &parameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr));
		ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

    // Compile the vertex shader code.
	ThrowIfFailed(D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", compileFlags, 0,&vertexShader, nullptr));

    // Compile the pixel shader code.
	ThrowIfFailed(D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", compileFlags, 0,&pixelShader, nullptr));

	// Define the vertex input layout match with color vertex shader.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Create render pso
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;

	LOG_DEBUG("[Create Color Shader Pipeline State]");
	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	// Create wireframe pso
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_wireframePipelineState)));

    // Create Matrix Constant Buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // Upload per frame by CPU
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(MatrixBufferType)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_matrixConstantBuffer))
	);
}

void ColorShader::SetShaderParameters(ID3D12GraphicsCommandList* commandList, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// Update Matrix Constant Buffer
	D3D12_RANGE readRange = { 0,0 };
	MatrixBufferType* matrixData;

	ThrowIfFailed(m_matrixConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&matrixData)));
	matrixData->world = worldMatrix;
	matrixData->view = viewMatrix;
	matrixData->projection = projectionMatrix;
	m_matrixConstantBuffer->Unmap(0, nullptr);

	commandList->SetGraphicsRootConstantBufferView(0, m_matrixConstantBuffer->GetGPUVirtualAddress());
}


void ColorShader::RenderShader(ID3D12GraphicsCommandList* commandList, int32_t indexCount)
{
	PIXBeginEvent(commandList, 0, L"Draw Using ColorShader");

	if (m_d3d->IsEnabledWireframe())
	{
		commandList->SetPipelineState(m_wireframePipelineState.Get());
	}
	else
	{
		commandList->SetPipelineState(m_pipelineState.Get());
	}

	// Render the data.
	commandList->DrawIndexedInstanced(indexCount, 1, 0,0,0);

	PIXEndEvent(commandList);
}