#include "stdafx.h"
#include "ShaderManager.h"
#include "D3D.h"
#include "ColorShader.h"
#include "FontShader.h"
#include "LightShader.h"
#include "SkyDomeShader.h"
#include "TerrainShader.h"
#include "TextureShader.h"

ShaderManager::ShaderManager()
{
}

ShaderManager::~ShaderManager()
{
}

void ShaderManager::Initialize(D3D* d3d)
{
	// Create the color shader object.
	m_ColorShader = make_unique<ColorShader>();

	// Initialize the color shader object.
	m_ColorShader->Initialize(d3d);

	// Create the texture shader object.
	m_TextureShader = make_unique<TextureShader>();

	// Initialize the texture shader object.
	m_TextureShader->Initialize(d3d);

	// Create the light shader object.
	m_LightShader = make_unique<LightShader>();

	// Initialize the light shader object.
	m_LightShader->Initialize(d3d);

	// Create the font shader object.
	m_FontShader = make_unique<FontShader>();

	// Initialize the font shader object.
	m_FontShader->Initialize(d3d);

	// Create the sky dome shader object.
	m_SkyDomeShader = make_unique<SkyDomeShader>();

	// Initialize the sky dome shader object.
	m_SkyDomeShader->Initialize(d3d);

	// Create the terrain shader object.
	m_TerrainShader = make_unique<TerrainShader>();

	// Initialize the terrain shader object.
	m_TerrainShader->Initialize(d3d);
}

void ShaderManager::RenderColorShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	m_ColorShader->Render(commandList, indexCount, worldMatrix, viewMatrix, projectionMatrix);
}


void ShaderManager::RenderTextureShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D12Resource* texture)
{
	m_TextureShader->Render(commandList, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture);
}


void ShaderManager::RenderLightShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,XMMATRIX projectionMatrix, ID3D12Resource* texture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	m_LightShader->Render(commandList, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, diffuseColor);
}


void ShaderManager::RenderFontShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D12Resource* texture, XMFLOAT4 color)
{
	m_FontShader->Render(commandList, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture, color);
}


void ShaderManager::RenderSkyDomeShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT4 apexColor, XMFLOAT4 centerColor)
{
	m_SkyDomeShader->Render(commandList, indexCount, worldMatrix, viewMatrix, projectionMatrix, apexColor, centerColor);
}


void ShaderManager::RenderTerrainShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D12Resource* texture, ID3D12Resource* normalMap, ID3D12Resource* normalMap2, ID3D12Resource* normalMap3, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	m_TerrainShader->Render(commandList, indexCount, worldMatrix, viewMatrix, projectionMatrix, texture, normalMap, normalMap2, normalMap3,
								   lightDirection, diffuseColor);
}