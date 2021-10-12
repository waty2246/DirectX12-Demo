#pragma once

class D3D;
class ColorShader;
class TextureShader;
class LightShader;
class FontShader;
class SkyDomeShader;
class TerrainShader;

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	void Initialize(D3D* d3d);

	void RenderColorShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);
	void RenderTextureShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D12Resource* texture);
	void RenderLightShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D12Resource* texture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor);
	void RenderFontShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D12Resource* texture, XMFLOAT4 color);
	void RenderSkyDomeShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT4 apexColor, XMFLOAT4 centerColor);
	void RenderTerrainShader(ID3D12GraphicsCommandList* commandList, uint32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D12Resource* texture, ID3D12Resource* normalMap, ID3D12Resource* normalMap2, ID3D12Resource* normalMap3, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor);

private:
	unique_ptr<ColorShader> m_ColorShader;
	unique_ptr<TextureShader> m_TextureShader;
	unique_ptr<LightShader> m_LightShader;
	unique_ptr<FontShader> m_FontShader;
	unique_ptr<SkyDomeShader> m_SkyDomeShader;
	unique_ptr<TerrainShader> m_TerrainShader;
};
