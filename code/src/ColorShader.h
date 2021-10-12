#pragma once

class D3D;

class ColorShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

public:
	ColorShader();
	~ColorShader();

	void Initialize(D3D* d3d);
	void Render(ID3D12GraphicsCommandList* commandList, int32_t indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);

private:
	void InitializeShader(ID3D12Device* device, WCHAR* vsFilename, WCHAR* psFilename);
	void SetShaderParameters(ID3D12GraphicsCommandList* commandList, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);
	void RenderShader(ID3D12GraphicsCommandList* commandList, int32_t indexCount);

private:
	D3D* m_d3d;
	ComPtr<ID3D12Resource> m_matrixConstantBuffer;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12PipelineState> m_wireframePipelineState;
};