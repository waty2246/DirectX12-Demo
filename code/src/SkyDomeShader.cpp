#include "stdafx.h"
#include "SkyDomeShader.h"
#include "D3D.h"

SkyDomeShader::SkyDomeShader()
{
}

SkyDomeShader::~SkyDomeShader()
{
}


void SkyDomeShader::Initialize(D3D* d3d)
{
	m_d3d = d3d;

	// Initialize the vertex and pixel shaders.
	InitializeShader(d3d->GetDevice(), GetShaderFilePath("skydome_vs.hlsl"), GetShaderFilePath("skydome_ps.hlsl"));
}

void SkyDomeShader::Render(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT4 apexColor, XMFLOAT4 centerColor)
{
	// Set the shader parameters that it will use for rendering.
	SetShaderParameters(commandList, worldMatrix, viewMatrix, projectionMatrix, apexColor, centerColor);

	// Now render the prepared buffers with the shader.
	RenderShader(commandList, indexCount);
}


void SkyDomeShader::InitializeShader(ID3D12Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	// Create root signature match with Sky Dome shader.
	{
		CD3DX12_ROOT_PARAMETER parameters[2];
		ZeroMemory(parameters, _countof(parameters) * sizeof(CD3DX12_ROOT_PARAMETER));

		parameters[0].InitAsConstantBufferView(0);
		parameters[1].InitAsConstantBufferView(1);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(_countof(parameters), parameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
	ThrowIfFailed(D3DCompileFromFile(vsFilename, NULL, NULL, "SkyDomeVertexShader", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));

	// Compile the pixel shader code.
	ThrowIfFailed(D3DCompileFromFile(psFilename, NULL, NULL, "SkyDomePixelShader", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	// Define the vertex input layout match with font vertex shader.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

	LOG_DEBUG("[Create SkyDome Shader Pipeline State]");
	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_disabledDepthAndCullingPSO)));

	// Create Matrix Constant Buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // Upload per frame by CPU
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(MatrixBufferType)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_matrixConstantBuffer))
	);

	// Create Pixel Constant Buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // Upload per frame by CPU
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ColorBufferType)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_colorConstantBuffer))
	);
}

void SkyDomeShader::SetShaderParameters(ID3D12GraphicsCommandList* commandList, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT4 apexColor, XMFLOAT4 centerColor)
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

	ColorBufferType* colorData;
	ThrowIfFailed(m_colorConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&colorData)));
	colorData->apexColor = apexColor;
	colorData->centerColor = centerColor;
	m_colorConstantBuffer->Unmap(0, nullptr);

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	commandList->SetGraphicsRootConstantBufferView(0, m_matrixConstantBuffer->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, m_colorConstantBuffer->GetGPUVirtualAddress());
}


void SkyDomeShader::RenderShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount)
{
	if (!m_d3d->IsEnabledDepth() && !m_d3d->IsEnabledCulling())
	{
		commandList->SetPipelineState(m_disabledDepthAndCullingPSO.Get());
	}
	else
	{
		commandList->SetPipelineState(m_pipelineState.Get());
	}
	

	// Render the Sky Dome data.
	commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}