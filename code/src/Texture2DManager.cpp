#include "stdafx.h"
#include "Texture2DManager.h"
#include "D3D.h"
#include "Texture2D.h"

Texture2DManager::Texture2DManager() :
	m_textureCount(0)
{
}

Texture2DManager::~Texture2DManager()
{
}

void Texture2DManager::Initialize(uint32_t textureCount)
{
	m_textureCount = textureCount;

	// Create the color texture object.
	m_textureArray = make_unique<Texture2D[]>(m_textureCount);
}

void Texture2DManager::LoadTexture(D3D* d3d, const char* fileName, uint32_t textureID)
{
	// Initialize the color texture object.
	m_textureArray[textureID].Initialize(d3d, fileName);
}

ID3D12Resource* Texture2DManager::GetTexture(uint32_t textureID)
{
	return m_textureArray[textureID].GetTexture();
}