#pragma once

class D3D;

class FontShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct PixelBufferType
	{
		XMFLOAT4 pixelColor;
	};

public:
	FontShader();
	~FontShader();

	void Initialize(D3D*d3d);
	void Render(ID3D12GraphicsCommandList*, uint32_t, XMMATRIX, XMMATRIX, XMMATRIX, ID3D12Resource*, XMFLOAT4);

private:
	void InitializeShader(ID3D12Device* device, WCHAR* vsFilename, WCHAR* psFilename);

	void SetShaderParameters(ID3D12GraphicsCommandList*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D12Resource*, XMFLOAT4);
	void RenderShader(ID3D12GraphicsCommandList*, uint32_t);

private:
	D3D* m_d3d;

	ComPtr<ID3D12Resource> m_matrixConstantBuffer;
	ComPtr<ID3D12Resource> m_pixelConstantBuffer;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_wireframePipelineState;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
};