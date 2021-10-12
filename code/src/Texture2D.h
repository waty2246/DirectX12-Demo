#pragma once

class D3D;

class Texture2D
{
private:
	struct TargaHeader
	{
		uint8_t data1[12];
		uint16_t width;
		uint16_t height;
		uint8_t bpp;
		uint8_t data2;
	};

public:
	Texture2D();
	~Texture2D();

	void Initialize(D3D* d3d,const char* fileName);

	ID3D12Resource* GetTexture();

private:
	unique_ptr<uint8_t[]> Texture2D::LoadTarga(const char* fileName, uint32_t& height, uint32_t& width);
	unique_ptr<uint8_t[]> Texture2D::LoadBMP(const char* fileName, uint32_t& height, uint32_t& width);

private:
	ComPtr<ID3D12Resource> m_texture;
};