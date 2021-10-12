#pragma once

class SkyDome
{
private:
	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VertexType
	{
		XMFLOAT3 position;
	};

public:
	SkyDome();
	~SkyDome();

	void Initialize(ID3D12Device*);
	void Render(ID3D12GraphicsCommandList*);

	int32_t GetIndexCount();
	XMFLOAT4 GetApexColor();
	XMFLOAT4 GetCenterColor();

private:
	void LoadSkyDomeModel(char*);
	void InitializeBuffers(ID3D12Device*);
	void RenderBuffers(ID3D12GraphicsCommandList*);

private:
	unique_ptr<ModelType[]> m_model;
	int32_t m_vertexCount, m_indexCount;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	XMFLOAT4 m_apexColor, m_centerColor;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};