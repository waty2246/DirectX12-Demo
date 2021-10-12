#pragma once

class D3D;

class TerrainCell
{
private:
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

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
		XMFLOAT3 color;
		XMFLOAT2 texture2;
	};

	struct VectorType
	{
		float x, y, z;
	};

	struct ColorVertexType
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

public:
	TerrainCell();
	~TerrainCell();

	void Initialize(D3D* d3d, void* terrainModelPtr, int32_t nodeIndexX, int32_t nodeIndexY, int32_t cellHeight, int32_t cellWidth, int32_t terrainWidth);
	void Render(ID3D12GraphicsCommandList*);
	void RenderLineBuffers(ID3D12GraphicsCommandList*);

	int32_t GetVertexCount();
	int32_t GetIndexCount();
	int32_t GetLineBuffersIndexCount();
	void GetCellDimensions(float&, float&, float&, float&, float&, float&);

private:
	bool InitializeBuffers(ID3D12Device* device, int32_t nodeIndexX, int32_t nodeIndexY, int32_t cellHeight, int32_t cellWidth,
		int32_t terrainWidth, ModelType* terrainModel);
	void RenderBuffers(ID3D12GraphicsCommandList*);
	void CalculateCellDimensions();
	bool BuildLineBuffers(ID3D12Device*);
	void InitializeBufferViews();

public:
	VectorType* m_vertexList;

private:
	int32_t m_vertexCount, m_indexCount, m_lineIndexCount;
	ComPtr<ID3D12Resource> m_vertexBuffer, m_indexBuffer, m_lineVertexBuffer, m_lineIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView, m_lineVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView, m_lineIndexBufferView;
	float m_maxWidth, m_maxHeight, m_maxDepth, m_minWidth, m_minHeight, m_minDepth;
	float m_positionX, m_positionY, m_positionZ;
};