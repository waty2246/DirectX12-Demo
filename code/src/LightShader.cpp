#include "stdafx.h"
#include "LightShader.h"
#include "D3D.h"

LightShader::LightShader()
{
}

LightShader::~LightShader()
{
}

void LightShader::Initialize(D3D* d3d)
{
	m_d3d = d3d;

	// Initialize the vertex and pixel shaders.
	InitializeShader(d3d->GetDevice(), GetShaderFilePath("light_vs.hlsl"), GetShaderFilePath("light_ps.hlsl"));
}

void LightShader::Render(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D12Resource* texture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	SetShaderParameters(commandList, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, diffuseColor);

	// Now render the prepared buffers with the shader.
	RenderShader(commandList, indexCount);
}


void LightShader::InitializeShader(ID3D12Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
	// Create root signature match with light shader.
	{
		CD3DX12_DESCRIPTOR_RANGE range;
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER parameters[3];
		ZeroMemory(parameters, _countof(parameters) * sizeof(CD3DX12_ROOT_PARAMETER));

		parameters[0].InitAsConstantBufferView(0);
		parameters[1].InitAsConstantBufferView(1);
		parameters[2].InitAsDescriptorTable(1, &range);

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		ZeroMemory(&sampler, sizeof(D3D12_STATIC_SAMPLER_DESC));
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 1;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(_countof(parameters), parameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
	ThrowIfFailed(D3DCompileFromFile(vsFilename, NULL, NULL, "LightVertexShader", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));

	// Compile the pixel shader code.
	ThrowIfFailed(D3DCompileFromFile(psFilename, NULL, NULL, "LightPixelShader", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	// Define the vertex input layout match with font vertex shader.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

	LOG_DEBUG("[Create Light Shader Pipeline State]");
	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	// Create Matrix Constant Buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // Upload per frame by CPU
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(MatrixBufferType)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_matrixConstantBuffer))
	);

	// Create Ligh Constant Buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // Upload per frame by CPU
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(LightBufferType)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_lightConstantBuffer))
	);
}

void LightShader::SetShaderParameters(ID3D12GraphicsCommandList* commandList, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D12Resource* texture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	auto device = m_d3d->GetDevice();

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

	LightBufferType* dataPtr2;
	ThrowIfFailed(m_lightConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&dataPtr2)));
	dataPtr2->diffuseColor = diffuseColor;
	dataPtr2->lightDirection = lightDirection;
	dataPtr2->padding = 0.0f;
	m_lightConstantBuffer->Unmap(0, nullptr);

	commandList->SetGraphicsRootConstantBufferView(0, m_matrixConstantBuffer->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, m_lightConstantBuffer->GetGPUVirtualAddress());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texture->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(texture, &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	commandList->SetGraphicsRootDescriptorTable(2, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
}

void LightShader::RenderShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount)
{
	commandList->SetPipelineState(m_pipelineState.Get());

	// Render the triangle.
	commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}