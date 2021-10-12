#include "stdafx.h"
#include "Text.h"
#include "Font.h"
#include "D3D.h"
#include "ShaderManager.h"

Text::Text()
{
}

Text::~Text()
{
}


void Text::Initialize(D3D* d3d, uint32_t screenWidth, uint32_t screenHeight, uint32_t maxLength, bool shadow, Font* Font, const char* text, int32_t positionX, int32_t positionY, float red, float green, float blue)
{
	// Store the screen width and height.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Store the maximum length of the sentence.
	m_maxLength = maxLength;

	// Store if this sentence is shadowed or not.
	m_shadow = shadow;

	// Initalize the sentence.
	InitializeSentence(d3d, Font, text, positionX, positionY, red, green, blue);
}

void Text::Render(ID3D12GraphicsCommandList* commandList, ShaderManager* ShaderManager, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX orthoMatrix, ID3D12Resource* fontTexture)
{
	// Draw the sentence.
	RenderSentence(commandList, ShaderManager, worldMatrix, viewMatrix, orthoMatrix, fontTexture);

	return;
}


void Text::InitializeSentence(D3D* d3d, Font* Font, const char* text, int32_t positionX, int32_t positionY, float red, float green, float blue)
{
	auto device = d3d->GetDevice();
	auto commandList = d3d->GetCommandList();

	unique_ptr<uint32_t[]> indices;
	uint32_t vertexBufferSize = 0;
	uint32_t indexBufferSize = 0;
	D3D12_RANGE readRange = { 0,0 };

	// Set the vertex and index count.
	m_vertexCount = 6 * m_maxLength;
	m_indexCount = 6 * m_maxLength;

	// Create the index array.
	indices = make_unique<uint32_t[]>(m_indexCount);

	// Initialize the index array.
	for(int32_t i = 0; i < m_indexCount; i++)
	{
		indices[i] = i;
	}

	vertexBufferSize = sizeof(VertexType) * m_vertexCount;
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)
	));

	indexBufferSize = sizeof(uint32_t) * m_indexCount;
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)
	));
	uint32_t* indexData;
	ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&indexData)));
	::memcpy(indexData, indices.get(), indexBufferSize);
	m_indexBuffer->Unmap(0, nullptr);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBufferView.StrideInBytes = sizeof(VertexType);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = indexBufferSize;

	// If shadowed create the second vertex and index buffer.
	if(m_shadow)
	{
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer2)
		));

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_indexBuffer2)
		));

		ThrowIfFailed(m_indexBuffer2->Map(0, &readRange, reinterpret_cast<void**>(&indexData)));
		::memcpy(indexData, indices.get(), indexBufferSize);
		m_indexBuffer2->Unmap(0, nullptr);

		m_shadowVertexBufferView.BufferLocation = m_vertexBuffer2->GetGPUVirtualAddress();
		m_shadowVertexBufferView.SizeInBytes = vertexBufferSize;
		m_shadowVertexBufferView.StrideInBytes = sizeof(VertexType);

		m_shadowIndexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_shadowIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		m_shadowIndexBufferView.SizeInBytes = indexBufferSize;
	}

	// Now add the text data to the sentence buffers.
	UpdateSentence(commandList, Font, text, positionX, positionY, red, green, blue);
}


void Text::UpdateSentence(ID3D12GraphicsCommandList* commandList, Font* Font, const char* text, int32_t positionX, int32_t positionY, float red, float green, float blue)
{
	int32_t numLetters;
	unique_ptr<VertexType[]> vertices;
	float drawX, drawY;
	VertexType* vertexData;
	int32_t vertexBufferSize = sizeof(VertexType)*m_vertexCount;
	D3D12_RANGE readRange = { 0,0 };


	// Store the color of the sentence.
	m_pixelColor = XMFLOAT4(red, green, blue, 1.0f);

	// Get the number of letters in the sentence.
	numLetters = (int32_t)strlen(text);

	// Check for possible buffer overflow.
	ThrowIfTrue(numLetters > m_maxLength);

	// Create the vertex array.
	vertices = make_unique<VertexType[]>(m_vertexCount);

	// Initialize vertex array to zeros at first.
	memset(vertices.get(), 0, vertexBufferSize);

	// Calculate the X and Y pixel position on the screen to start drawing to.
	drawX = (float)(((m_screenWidth / 2) * -1) + positionX);
	drawY = (float)((m_screenHeight / 2) - positionY);

	// Use the font class to build the vertex array from the sentence text and sentence draw location.
	Font->BuildVertexArray((void*)vertices.get(), text, drawX, drawY);

	// Lock the vertex buffer.
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&vertexData)));

	// Copy the vertex array into the vertex buffer.
	::memcpy(vertexData, vertices.get(), vertexBufferSize);

	// Unlock the vertex buffer.
	m_vertexBuffer->Unmap(0, nullptr);

	// If shadowed then do the same for the second vertex buffer but offset by two pixels on both axis.
	if (m_shadow)
	{
		memset(vertices.get(), 0, vertexBufferSize);

		drawX = (float)((((m_screenWidth / 2) * -1) + positionX) + 2);
		drawY = (float)(((m_screenHeight / 2) - positionY) - 2);
		Font->BuildVertexArray((void*)vertices.get(), text, drawX, drawY);

		ThrowIfFailed(m_vertexBuffer2->Map(0, &readRange, reinterpret_cast<void**>(&vertexData)));
		::memcpy(vertexData, vertices.get(), vertexBufferSize);
		m_vertexBuffer2->Unmap(0, nullptr);
	}
}


void Text::RenderSentence(ID3D12GraphicsCommandList* commandList, ShaderManager* ShaderManager, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
							   XMMATRIX orthoMatrix, ID3D12Resource* fontTexture)
{
	XMFLOAT4 shadowColor;

	// If shadowed then render the shadow text first.
	if (m_shadow)
	{
		shadowColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		commandList->IASetVertexBuffers(0, 1, &m_shadowVertexBufferView);
		commandList->IASetIndexBuffer(&m_shadowIndexBufferView);
		commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ShaderManager->RenderFontShader(commandList, m_indexCount, worldMatrix, viewMatrix, orthoMatrix, fontTexture, shadowColor);
	}

	// Render the text buffers.
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ShaderManager->RenderFontShader(commandList, m_indexCount, worldMatrix, viewMatrix, orthoMatrix, fontTexture, m_pixelColor);

	return;
}