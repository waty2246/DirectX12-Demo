#include "stdafx.h"
#include "image.h"
#include "Texture2D.h"
#include "D3D.h"

Image::Image() : 
	m_vertexCount(0),
	m_indexCount(0),
	m_screenWidth(0),
	m_screenHeight(0),
	m_bitmapWidth(0),
	m_bitmapHeight(0),
	m_previousPosX(0),
	m_previousPosY(0),
	m_indexBufferView(),
	m_vertexBufferView()
{
}

Image::~Image()
{
}


void Image::Initialize(D3D* d3d, int32_t screenWidth, int32_t screenHeight, int32_t bitmapWidth, int32_t bitmapHeight, char* textureFilename)
{
	auto device = d3d->GetDevice();
	auto commandList = d3d->GetCommandList();

	// Store the screen size.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Store the size in pixels that this bitmap should be rendered at.
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;

	// Initialize the previous rendering position to negative one.
	m_previousPosX = -1;
	m_previousPosY = -1;

	// Initialize the vertex and index buffer that hold the geometry for the bitmap quad.
	InitializeBuffers(device);
	

	// Load the texture for this bitmap.
	LoadTexture(d3d, textureFilename);
}

void Image::Render(ID3D12GraphicsCommandList* commandList, int32_t positionX, int32_t positionY)
{
	// Re-build the dynamic vertex buffer for rendering to possibly a different location on the screen.
	if(!UpdateBuffers(commandList, positionX, positionY))
	{
		return;
	}

	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(commandList);
}


int32_t Image::GetIndexCount()
{
	return m_indexCount;
}


ID3D12Resource* Image::GetTexture()
{
	return m_Texture->GetTexture();
}


void Image::InitializeBuffers(ID3D12Device* device)
{
	unique_ptr<VertexType[]> vertices;
	unique_ptr<uint32_t[]> indices;
	int32_t i;


	// Set the number of vertices in the vertex array.
	m_vertexCount = 6;

	// Set the number of indices in the index array.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = make_unique<VertexType[]>(m_vertexCount);

	// Create the index array.
	indices = make_unique<uint32_t[]>(m_indexCount);

	// Initialize vertex array to zeros at first.
	memset(vertices.get(), 0, (sizeof(VertexType) * m_vertexCount));

	// Load the index array with data.
	for(i=0; i<m_indexCount; i++)
	{
		indices[i] = i;
	}

	// Set up the description of the dynamic vertex buffer.
	D3D12_RESOURCE_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D12_RESOURCE_DESC));
	vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexBufferDesc.DepthOrArraySize = 1;
	vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	vertexBufferDesc.SampleDesc.Count = 1;
	vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vertexBufferDesc.Width = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.Height = 1;
	vertexBufferDesc.MipLevels = 1;

	D3D12_HEAP_PROPERTIES vertexHeapProperties;
	ZeroMemory(&vertexHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	vertexHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// Now finally create the vertex buffer.
	ThrowIfFailed(device->CreateCommittedResource(&vertexHeapProperties,D3D12_HEAP_FLAG_NONE,&vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&m_vertexBuffer)));
	
	D3D12_RANGE readRange;
	ZeroMemory(&readRange, sizeof(D3D12_RANGE)); // We do not intend to read from this resource on the CPU.

	UINT8* vertexData = nullptr;
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&vertexData)));
	memcpy(vertexData, vertices.get(), sizeof(VertexType) * m_vertexCount);
	m_vertexBuffer->Unmap(0, nullptr);

	D3D12_HEAP_PROPERTIES indexHeapProperties;
	ZeroMemory(&indexHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	indexHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D12_RESOURCE_DESC));
	indexBufferDesc.DepthOrArraySize = 1;
	indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	indexBufferDesc.SampleDesc.Count = 1;
	indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	indexBufferDesc.Height = 1;
	indexBufferDesc.MipLevels = 1;
	indexBufferDesc.Width = sizeof(uint32_t) * m_indexCount;

	ThrowIfFailed(device->CreateCommittedResource(&indexHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_indexBuffer)));

	UINT8* indexData = nullptr;
	ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&indexData)));
	memcpy(indexData, indices.get(), sizeof(unsigned long) * m_indexCount );
	m_indexBuffer->Unmap(0, nullptr);
}

bool Image::UpdateBuffers(ID3D12GraphicsCommandList* deviceContent, int32_t positionX, int32_t positionY)
{
	float left, right, top, bottom;
	unique_ptr<VertexType[]> vertices;

	// If the position we are rendering this bitmap to has not changed then don't update the vertex buffer since it
	// currently has the correct parameters.
	if((positionX == m_previousPosX) && (positionY == m_previousPosY))
	{
		return true;
	}

	// If it has changed then update the position it is being rendered to.
	m_previousPosX = positionX;
	m_previousPosY = positionY;

	// Calculate the screen coordinates of the left side of the bitmap.
	left = (float)((m_screenWidth / 2) * -1) + (float)positionX;

	// Calculate the screen coordinates of the right side of the bitmap.
	right = left + (float)m_bitmapWidth;

	// Calculate the screen coordinates of the top of the bitmap.
	top = (float)(m_screenHeight / 2) - (float)positionY;

	// Calculate the screen coordinates of the bottom of the bitmap.
	bottom = top - (float)m_bitmapHeight;

	// Create the vertex array.
	vertices = make_unique<VertexType[]>(m_vertexCount);

	// Load the vertex array with data.
	// First triangle.
	vertices[0].position = XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

	// Second triangle.
	vertices[3].position = XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = XMFLOAT3(right, top, 0.0f);  // Top right.
	vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

	D3D12_RANGE readRange;
	ZeroMemory(&readRange, sizeof(D3D12_RANGE));

	// Lock the vertex buffer.
	UINT8* vertexData = nullptr;
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange,reinterpret_cast<void**>(&vertexData)));
	memcpy(vertexData, vertices.get(), (sizeof(VertexType) * m_vertexCount));
	m_vertexBuffer->Unmap(0, nullptr);

	return true;
}


void Image::RenderBuffers(ID3D12GraphicsCommandList* commandList)
{
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

	// Set the index buffer to active in the input assembler so it can be rendered.
	commandList->IASetIndexBuffer(&m_indexBufferView);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


void Image::LoadTexture(D3D* d3d, const char* filename)
{
	// Create the texture object.
	m_Texture = make_unique<Texture2D>();

	// Initialize the texture object.
	m_Texture->Initialize(d3d, filename);
}