#include "stdafx.h"
#include "SkyDome.h"


SkyDome::SkyDome()
{
}

SkyDome::~SkyDome()
{
}

void SkyDome::Initialize(ID3D12Device* device)
{
	// Load in the sky dome model.
	LoadSkyDomeModel(GetDataFilePathA("skydome/skydome.txt"));

	// Load the sky dome into a vertex and index buffer for rendering.
	InitializeBuffers(device);

	// Set the color at the top of the sky dome.
	m_apexColor = XMFLOAT4(0.0f, 0.05f, 0.6f, 1.0f);

	// Set the color at the center of the sky dome.
	m_centerColor = XMFLOAT4(0.0f, 0.5f, 0.8f, 1.0f);
}

void SkyDome::Render(ID3D12GraphicsCommandList* commandList)
{
	// Render the sky dome.
	RenderBuffers(commandList);
}


int32_t SkyDome::GetIndexCount()
{
	return m_indexCount;
}


XMFLOAT4 SkyDome::GetApexColor()
{
	return m_apexColor;
}


XMFLOAT4 SkyDome::GetCenterColor()
{
	return m_centerColor;
}


void SkyDome::LoadSkyDomeModel(char* filename)
{
	ifstream fin;
	char input;
	int32_t i;


	// Open the model file.
	fin.open(filename);

	// If it could not open the file then exit.
	ThrowIfTrue(fin.fail());

	// Read up to the value of vertex count.
	fin.get(input);
	while(input != ':')
	{
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> m_vertexCount;

	// Set the number of indices to be the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the model using the vertex count that was read in.
	m_model = make_unique<ModelType[]>(m_vertexCount);

	// Read up to the beginning of the data.
	fin.get(input);
	while(input != ':')
	{
		fin.get(input);
	}

	fin.get(input);
	fin.get(input);

	// Read in the vertex data.
	for(i=0; i<m_vertexCount; i++)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	// Close the model file.
	fin.close();
}


void SkyDome::InitializeBuffers(ID3D12Device* device)
{
	unique_ptr<VertexType[]> vertices;
	unique_ptr<uint32_t[]> indices;
	D3D12_RANGE readRange = { 0,0 };
	uint32_t vertexBufferSize = sizeof(VertexType) * m_vertexCount;
	uint32_t indexBufferSize = sizeof(uint32_t) * m_indexCount;


	// Create the vertex and index array.
	vertices = make_unique<VertexType[]>(m_vertexCount);
	indices = make_unique<uint32_t[]>(m_indexCount);

	// Load the vertex array and index array with data.
	for (int32_t i = 0; i < m_vertexCount; i++)
	{
		vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		indices[i] = i;
	}

	// Create and initialize the skydome vertex buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)
	));

	VertexType* vertexData = nullptr;
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&vertexData)));
	memcpy(vertexData, vertices.get(), vertexBufferSize);
	m_vertexBuffer->Unmap(0,nullptr);

	// Create and initialize the skydome index buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)
	));

	uint32_t* indexData = nullptr;
	ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&indexData)));
	memcpy(indexData, indices.get(), indexBufferSize);
	m_indexBuffer->Unmap(0, nullptr);

	// Initialize the skydome vertex and index buffer views
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBufferView.StrideInBytes = sizeof(VertexType);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = indexBufferSize;
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}


void SkyDome::RenderBuffers(ID3D12GraphicsCommandList* commandList)
{
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	commandList->IASetVertexBuffers(0,1, &m_vertexBufferView);

	// Set the index buffer to active in the input assembler so it can be rendered.
	commandList->IASetIndexBuffer(&m_indexBufferView);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}