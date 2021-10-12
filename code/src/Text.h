#pragma once

class Font;
class D3D;
class ShaderManager;

class Text
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	Text();
	~Text();

	void Initialize(D3D* d3d, uint32_t screenWidth, uint32_t screenHeight, uint32_t maxLength, bool shadow, Font* Font, const char* text, int32_t positionX, int32_t positionY, float red, float green, float blue);
	void Render(ID3D12GraphicsCommandList*, ShaderManager*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D12Resource*);

	void UpdateSentence(ID3D12GraphicsCommandList* commandList, Font* Font, const char* text, int32_t positionX, int32_t positionY, float red, float green, float blue);

private:
	void InitializeSentence(D3D* d3d, Font* Font, const char* text, int32_t positionX, int32_t positionY, float red, float green, float blue);
	void RenderSentence(ID3D12GraphicsCommandList*, ShaderManager*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D12Resource*);

private:
	ComPtr<ID3D12Resource> m_vertexBuffer, m_indexBuffer, m_vertexBuffer2, m_indexBuffer2;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView, m_shadowVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView, m_shadowIndexBufferView;
	int32_t m_screenWidth, m_screenHeight, m_maxLength, m_vertexCount, m_indexCount;
	bool m_shadow;
	XMFLOAT4 m_pixelColor;
};