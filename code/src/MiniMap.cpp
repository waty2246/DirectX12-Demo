#include "stdafx.h"
#include "MiniMap.h"
#include "D3D.h"
#include "ShaderManager.h"
#include "Image.h"


MiniMap::MiniMap()
{
}


MiniMap::~MiniMap()
{
}


void MiniMap::Initialize(D3D* d3d, uint32_t screenWidth, uint32_t screenHeight, float terrainWidth, float terrainHeight)
{
	// Set the size of the mini-map  minus the borders.
	m_mapSizeX = 150.0f;
	m_mapSizeY = 150.0f;

	// Initialize the location of the mini-map on the screen.
	m_mapLocationX = screenWidth - (int)m_mapSizeX - 10;
	m_mapLocationY = 10;

	// Store the terrain size.
	m_terrainWidth = terrainWidth;
	m_terrainHeight = terrainHeight;

	// Create the mini-map bitmap object.
	m_MiniMapBitmap = make_unique<Image>();

	// Initialize the mini-map bitmap object.
	m_MiniMapBitmap->Initialize(d3d, screenWidth, screenHeight, 154, 154, GetDataFilePathA("minimap/minimap.tga"));

	// Create the point32_t bitmap object.
	m_PointBitmap = make_unique<Image>();

	// Initialize the point32_t bitmap object.
	m_PointBitmap->Initialize(d3d, screenWidth, screenHeight, 3, 3, GetDataFilePathA("minimap/point.tga"));
}

void MiniMap::Render(ID3D12GraphicsCommandList* commandList, ShaderManager* ShaderManager, XMMATRIX worldMatrix, 
						  XMMATRIX viewMatrix, XMMATRIX orthoMatrix)
{
	// Put the mini-map bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_MiniMapBitmap->Render(commandList, m_mapLocationX, m_mapLocationY);

	// Render the mini-map bitmap using the texture shader.
	ShaderManager->RenderTextureShader(commandList, m_MiniMapBitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_MiniMapBitmap->GetTexture());

	// Put the point32_t bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_PointBitmap->Render(commandList, m_pointLocationX, m_pointLocationY);

	// Render the point32_t bitmap using the texture shader.
	ShaderManager->RenderTextureShader(commandList, m_PointBitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_PointBitmap->GetTexture());
}


void MiniMap::PositionUpdate(float positionX, float positionZ)
{
	float percentX, percentY;


	// Ensure the point32_t does not leave the minimap borders even if the camera goes past the terrain borders.
	if(positionX < 0)
	{
		positionX = 0;
	}

	if(positionZ < 0)
	{
		positionZ = 0;
	}

	if(positionX > m_terrainWidth)
	{
		positionX = m_terrainWidth;
	}

	if(positionZ > m_terrainHeight)
	{
		positionZ = m_terrainHeight;
	}

	// Calculate the position of the camera on the minimap in terms of percentage.
	percentX = positionX / m_terrainWidth;
	percentY = 1.0f - (positionZ / m_terrainHeight);

	// Determine the pixel location of the point32_t on the mini-map.
	m_pointLocationX = (m_mapLocationX + 2) + (int)(percentX * m_mapSizeX);
	m_pointLocationY = (m_mapLocationY + 2) + (int)(percentY * m_mapSizeY);

	// Subtract one from the location to center the point32_t on the mini-map according to the 3x3 point32_t pixel image size.
	m_pointLocationX = m_pointLocationX - 1;
	m_pointLocationY = m_pointLocationY - 1;

	return;
}