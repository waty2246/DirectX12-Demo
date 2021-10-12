#pragma once

class TerrainCell;
class Frustum;
class D3D;

class Terrain
{
private:
	struct HeightMapType
	{
		float x, y, z;
		float nx, ny, nz;
		float r, g, b;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
		float tx, ty, tz;
		float bx, by, bz;
		float r, g, b;
		float tu2, tv2;
	};

	struct VectorType
	{
		float x, y, z;
	};

	struct TempVertexType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

public:
	Terrain();
	~Terrain();

	void Initialize(D3D*d3d, char*);
	void Frame();

	bool RenderCell(ID3D12GraphicsCommandList*, int32_t, Frustum*);
	void RenderCellLines(ID3D12GraphicsCommandList*, int32_t);

	int32_t GetCellIndexCount(int32_t);
	int32_t GetCellLinesIndexCount(int32_t);
	int32_t GetCellCount();
	int32_t GetRenderCount();
	int32_t GetCellsDrawn();
	int32_t GetCellsCulled();

	bool GetHeightAtPosition(float, float, float&);

private:
	void LoadSetupFile(char*);
	void LoadRawHeightMap();
	void SetTerrainCoordinates();
	bool CalculateNormals();
	void LoadColorMap();
	bool BuildTerrainModel();
	void CalculateTerrainVectors();
	void CalculateTangentBinormal(TempVertexType, TempVertexType, TempVertexType, VectorType&, VectorType&);
	void LoadTerrainCells(D3D*);
	bool CheckHeightOfTriangle(float, float, float&, float[3], float[3], float[3]);

private:
	int32_t m_terrainHeight, m_terrainWidth, m_vertexCount;
	float m_heightScale;
	unique_ptr<char[]> m_terrainFilename;
	unique_ptr<char[]> m_colorMapFilename;
	unique_ptr<HeightMapType[]> m_heightMap;
	unique_ptr<ModelType[]> m_terrainModel;
	unique_ptr<TerrainCell[]> m_TerrainCells;
	int32_t m_cellCount, m_renderCount, m_cellsDrawn, m_cellsCulled;
};