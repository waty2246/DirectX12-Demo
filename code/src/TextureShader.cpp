#include "stdafx.h"
#include "TextureShader.h"
#include "D3D.h"

TextureShader::TextureShader()
{
}

TextureShader::~TextureShader()
{
}


void TextureShader::Initialize(D3D* d3d)
{
	m_d3d = d3d;

	InitializeShader(d3d->GetDevice(), GetShaderFilePath("texture_vs.hlsl"), GetShaderFilePath("texture_ps.hlsl"));
	
}

void TextureShader::Render(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
								XMMATRIX projectionMatrix, ID3D12Resource* texture)
{
	// Set the shader parameters that it will use for rendering.
	SetShaderParameters(commandList, worldMatrix, viewMatrix, projectionMatrix, texture);

	// Now render the prepared buffers with the shader.
	RenderShader(commandList, indexCount);
}


void TextureShader::InitializeShader(ID3D12Device* device, WCHAR* vsFilename, WCHAR* psFilename)
{
}

void TextureShader::SetShaderParameters(ID3D12GraphicsCommandList* commandList, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
											 XMMATRIX projectionMatrix, ID3D12Resource* texture)
{
}


void TextureShader::RenderShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount)
{
}