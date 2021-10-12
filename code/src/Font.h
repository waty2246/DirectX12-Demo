#pragma once

class Texture2D;
class D3D;

class Font
{
private:
	struct FontType
	{
		float left, right;
		uint32_t size;
	};

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	Font();
	~Font();

	void Initialize(D3D* d3d, const char* fontFileName, const char* textureFileName, float fontHeight, uint32_t spaceSize);

	ID3D12Resource* GetTexture();
	void BuildVertexArray(void*, const char*, float, float);
	int32_t GetSentencePixelLength(const char*);
	uint32_t GetFontHeight();

private:
	void LoadFontData(const char*);
	void LoadTexture(D3D* d3d, const char*);

private:
	unique_ptr<FontType[]> m_Font;
	unique_ptr<Texture2D> m_texture2D;
	float m_fontHeight;
	uint32_t m_spaceSize;
};