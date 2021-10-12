#include "stdafx.h"
#include "TerrainCell.h"
#include "D3D.h"

TerrainCell::TerrainCell()
{
}

TerrainCell::~TerrainCell()
{
}

void TerrainCell::Initialize(D3D* d3d, void* terrainModelPtr, int32_t nodeIndexX, int32_t nodeIndexY, int32_t cellHeight, int32_t cellWidth, int32_t terrainWidth)
{
	auto device = d3d->GetDevice();

	ModelType* terrainModel;

	// Coerce the pointer to the terrain model into the model type.
	terrainModel = (ModelType*)terrainModelPtr;

	// Load the rendering buffers with the terrain data for this cell index.
	InitializeBuffers(device, nodeIndexX, nodeIndexY, cellHeight, cellWidth, terrainWidth, terrainModel);

	// Release the pointer to the terrain model now that we no longer need it.
	terrainModel = 0;

	// Calculuate the dimensions of this cell.
	CalculateCellDimensions();

	// Build the debug line buffers to produce the bounding box around this cell.
	BuildLineBuffers(device);

	InitializeBufferViews();
}

void TerrainCell::Render(ID3D12GraphicsCommandList* commandList)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(commandList);

	return;
}


int32_t TerrainCell::GetVertexCount()
{
	return m_vertexCount;
}


int32_t TerrainCell::GetIndexCount()
{
	return m_indexCount;
}


bool TerrainCell::InitializeBuffers(ID3D12Device* device, int32_t nodeIndexX, int32_t nodeIndexY, int32_t cellHeight, int32_t cellWidth, 
										 int32_t terrainWidth, ModelType* terrainModel)
{
	VertexType* vertices;
	unsigned long* indices;
	int32_t i, j, modelIndex, index;
	D3D12_RANGE readRange = { 0,0 };


	// Calculate the number of vertices in this terrain cell.
	m_vertexCount = (cellHeight - 1) * (cellWidth - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}

	// Setup the indexes into the terrain model data and the local vertex/index array.
	modelIndex = ((nodeIndexX * (cellWidth - 1)) + (nodeIndexY * (cellHeight - 1) * (terrainWidth - 1))) * 6;
	index = 0;

	// Load the vertex array and index array with data.
	for(j=0; j<(cellHeight - 1); j++)
	{
		for(i=0; i<((cellWidth - 1) * 6); i++)
		{
			vertices[index].position = XMFLOAT3(terrainModel[modelIndex].x, terrainModel[modelIndex].y, terrainModel[modelIndex].z);
			vertices[index].texture = XMFLOAT2(terrainModel[modelIndex].tu, terrainModel[modelIndex].tv);
			vertices[index].normal = XMFLOAT3(terrainModel[modelIndex].nx, terrainModel[modelIndex].ny, terrainModel[modelIndex].nz);
			vertices[index].tangent = XMFLOAT3(terrainModel[modelIndex].tx, terrainModel[modelIndex].ty, terrainModel[modelIndex].tz);
			vertices[index].binormal = XMFLOAT3(terrainModel[modelIndex].bx, terrainModel[modelIndex].by, terrainModel[modelIndex].bz);
			vertices[index].color = XMFLOAT3(terrainModel[modelIndex].r, terrainModel[modelIndex].g, terrainModel[modelIndex].b);
			vertices[index].texture2 = XMFLOAT2(terrainModel[modelIndex].tu2, terrainModel[modelIndex].tv2);
			indices[index] = index;
			modelIndex++;
			index++;
		}
		modelIndex += (terrainWidth * 6) - (cellWidth * 6);
	}

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexType) * m_vertexCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)
	));

	void* data;
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&data)));
	memcpy(data, vertices, sizeof(VertexType) * m_vertexCount);
	m_vertexBuffer->Unmap(0, nullptr);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint32_t) * m_indexCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)
	));

	ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&data)));
	memcpy(data, indices, sizeof(uint32_t) * m_indexCount);
	m_indexBuffer->Unmap(0, nullptr);

	// Create a public vertex array that will be used for accessing vertex information about this cell.
	m_vertexList = new VectorType[m_vertexCount];
	if(!m_vertexList)
	{
		return false;
	}

	// Keep a local copy of the vertex position data for this cell.
	for(i=0; i<m_vertexCount; i++)
	{
		m_vertexList[i].x = vertices[i].position.x;
		m_vertexList[i].y = vertices[i].position.y;
		m_vertexList[i].z = vertices[i].position.z;
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}

void TerrainCell::RenderBuffers(ID3D12GraphicsCommandList* commandList)
{
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

	// Set the index buffer to active in the input assembler so it can be rendered.
	commandList->IASetIndexBuffer(&m_indexBufferView);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


void TerrainCell::CalculateCellDimensions()
{
	int32_t i;
	float width, height, depth;


	// Initialize the dimensions of the node.
	m_maxWidth = -1000000.0f;
	m_maxHeight = -1000000.0f;
	m_maxDepth = -1000000.0f;

	m_minWidth = 1000000.0f;
	m_minHeight = 1000000.0f;
	m_minDepth = 1000000.0f;

	for(i=0; i<m_vertexCount; i++)
	{
		width = m_vertexList[i].x;
		height = m_vertexList[i].y;
		depth = m_vertexList[i].z;

		// Check if the width exceeds the minimum or maximum.
		if(width > m_maxWidth)
		{
			m_maxWidth = width;
		}
		if(width < m_minWidth)
		{
			m_minWidth = width;
		}

		// Check if the height exceeds the minimum or maximum.
		if(height > m_maxHeight)
		{
			m_maxHeight = height;
		}
		if(height < m_minHeight)
		{
			m_minHeight = height;
		}

		// Check if the depth exceeds the minimum or maximum.
		if(depth > m_maxDepth)
		{
			m_maxDepth = depth;
		}
		if(depth < m_minDepth)
		{
			m_minDepth = depth;
		}
	}

	// Calculate the center position of this cell.
	m_positionX = (m_maxWidth - m_minWidth) + m_minWidth;
	m_positionY = (m_maxHeight - m_minHeight) + m_minHeight;
	m_positionZ = (m_maxDepth - m_minDepth) + m_minDepth;

	return;
}


bool TerrainCell::BuildLineBuffers(ID3D12Device* device)
{
	ColorVertexType* vertices;
	unsigned long* indices;
	D3D12_RANGE readRange = { 0,0 };
	void* data = nullptr;
	XMFLOAT4 lineColor;
	int32_t index, vertexCount, indexCount;


	// Set the color of the lines to orange.
	lineColor = XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f);

	// Set the number of vertices in the vertex array.
	vertexCount = 24;

	// Set the number of indices in the index array.
	indexCount = vertexCount;

	// Create the vertex array.
	vertices = new ColorVertexType[vertexCount];

	// Create the index array.
	indices = new unsigned long[indexCount];

	// Load the vertex and index array with data.
	index = 0;

	// 8 Horizontal lines.
	vertices[index].position = XMFLOAT3(m_minWidth, m_minHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_minHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_minHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_minHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_minHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_minHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_minHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_minHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_maxHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_maxHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_maxHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_maxHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_maxHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_maxHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_maxHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_maxHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	// 4 Verticle lines.
	vertices[index].position = XMFLOAT3(m_maxWidth, m_maxHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_minHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_maxHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_minHeight, m_maxDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_maxHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_maxWidth, m_minHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_maxHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;
	index++;

	vertices[index].position = XMFLOAT3(m_minWidth, m_minHeight, m_minDepth);
	vertices[index].color = lineColor;
	indices[index] = index;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(ColorVertexType)* vertexCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_lineVertexBuffer)
	));

	ThrowIfFailed(m_lineVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&data)));
	memcpy(data, vertices, sizeof(ColorVertexType)* vertexCount);
	m_lineVertexBuffer->Unmap(0, nullptr);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(unsigned long)* indexCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_lineIndexBuffer)
	));
	ThrowIfFailed(m_lineIndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&data)));
	memcpy(data, indices, sizeof(unsigned long)* indexCount);
	m_lineIndexBuffer->Unmap(0, nullptr);

	// Store the index count for rendering.
	m_lineIndexCount = indexCount;

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}

void TerrainCell::InitializeBufferViews()
{
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(VertexType);
	m_vertexBufferView.SizeInBytes = sizeof(VertexType) * m_vertexCount;

	m_lineVertexBufferView.BufferLocation = m_lineVertexBuffer->GetGPUVirtualAddress();
	m_lineVertexBufferView.StrideInBytes = sizeof(ColorVertexType);
	m_lineVertexBufferView.SizeInBytes = sizeof(ColorVertexType) * 24;

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = sizeof(uint32_t) * m_indexCount;

	m_lineIndexBufferView.BufferLocation = m_lineIndexBuffer->GetGPUVirtualAddress();
	m_lineIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_lineIndexBufferView.SizeInBytes = sizeof(uint32_t) * 24;
}

void TerrainCell::RenderLineBuffers(ID3D12GraphicsCommandList* commandList)
{
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	commandList->IASetVertexBuffers(0, 1, &m_lineVertexBufferView);

	// Set the index buffer to active in the input assembler so it can be rendered.
	commandList->IASetIndexBuffer(&m_lineIndexBufferView);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case lines.
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	return;
}


int32_t TerrainCell::GetLineBuffersIndexCount()
{
	return m_lineIndexCount;
}


void TerrainCell::GetCellDimensions(float& maxWidth, float& maxHeight, float& maxDepth, 
										 float& minWidth, float& minHeight, float& minDepth)
{
	maxWidth = m_maxWidth;
	maxHeight = m_maxHeight;
	maxDepth = m_maxDepth;
	minWidth = m_minWidth;
	minHeight = m_minHeight;
	minDepth = m_minDepth;
	return;
}