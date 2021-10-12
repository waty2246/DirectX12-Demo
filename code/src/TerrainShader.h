#pragma once

class D3D;

class TerrainShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct LightBufferType
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding;
	};

public:
	TerrainShader();
	~TerrainShader();

	void Initialize(D3D* d3d);
	void Render(ID3D12GraphicsCommandList*, uint32_t, XMMATRIX, XMMATRIX, XMMATRIX, ID3D12Resource*, ID3D12Resource*, ID3D12Resource*, ID3D12Resource*, XMFLOAT3, XMFLOAT4);

private:
	void InitializeShader(ID3D12Device*, WCHAR*, WCHAR*);

	void SetShaderParameters(ID3D12GraphicsCommandList*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D12Resource*, ID3D12Resource*,
							 ID3D12Resource*, ID3D12Resource*, XMFLOAT3, XMFLOAT4);
	void RenderShader(ID3D12GraphicsCommandList*, uint32_t);

private:
	D3D* m_d3d;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	ComPtr<ID3D12Resource> m_matrixConstantBuffer, m_lightConstantBuffer;
};