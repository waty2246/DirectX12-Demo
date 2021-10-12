#pragma once

class Texture2D;
class D3D;

class Image
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	Image();
	~Image();

	void Initialize(D3D* d3d, int32_t screenWidth, int32_t screenHeight, int32_t bitmapWidth, int32_t bitmapHeight, char* textureFilename);
	void Render(ID3D12GraphicsCommandList* commandList, int32_t positionX, int32_t positionY);

	int32_t GetIndexCount();
	ID3D12Resource* GetTexture();

private:
	void InitializeBuffers(ID3D12Device* device);
	bool UpdateBuffers(ID3D12GraphicsCommandList* deviceContent, int32_t positionX, int32_t positionY);
	void RenderBuffers(ID3D12GraphicsCommandList* commandList);
	void LoadTexture(D3D* d3d, const char* filename);

private:
	ComPtr<ID3D12Resource> m_vertexBuffer, m_indexBuffer;
	int32_t m_vertexCount, m_indexCount;
	int32_t m_screenWidth, m_screenHeight;
	int32_t m_bitmapWidth, m_bitmapHeight;
	int32_t m_previousPosX, m_previousPosY;
	unique_ptr<Texture2D> m_Texture;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};