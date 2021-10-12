#pragma once

class D3D;
class Texture2D;

class Texture2DManager
{
public:
	Texture2DManager();
	~Texture2DManager();

	void Initialize(uint32_t textureCount);
	void LoadTexture(D3D* d3d, const char* fileName, uint32_t textureID);

	ID3D12Resource* GetTexture(uint32_t textureID);

private:
	unique_ptr<Texture2D[]> m_textureArray;
	uint32_t m_textureCount;
};