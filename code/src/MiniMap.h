#pragma once

class ShaderManager;
class D3D;
class Image;

class MiniMap
{
public:
	MiniMap();
	~MiniMap();

	void Initialize(D3D* d3d, uint32_t screenWidth, uint32_t screenHeight, float terrainWidth, float terrainHeight);
	void Render(ID3D12GraphicsCommandList*, ShaderManager*, XMMATRIX, XMMATRIX, XMMATRIX);
	void PositionUpdate(float, float);

private:
	int32_t m_mapLocationX, m_mapLocationY, m_pointLocationX, m_pointLocationY;
	float m_mapSizeX, m_mapSizeY, m_terrainWidth, m_terrainHeight;
	unique_ptr<Image> m_MiniMapBitmap, m_PointBitmap;
};