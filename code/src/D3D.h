#pragma once

class D3D
{
public:
	D3D();
	~D3D();

	void Initialize(uint32_t screenWidth, uint32_t screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenNear, float screenFar);
	void BeginScene(float red, float green, float blue, float alpha);
	void EndScene();

	ID3D12Device* GetDevice();
	ID3D12GraphicsCommandList* GetCommandList();

	void GetProjectionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void GetVideoCardInfo(char*, int&);

	// Graphics state settings
	void EnableDepth();
	void DisableDepth();

	void EnableCulling();
	void DisableCulling();

	void EnableAlphaBlending();
	void DisableAlphaBlending();

	void EnableWireframe();
	void DisableWireframe();

	bool IsEnabledDepth();
	bool IsEnabledCulling();
	bool IsEnabledAlphaBlending();
	bool IsEnabledWireframe();

	DXGI_FORMAT GetDepthStencilBufferFormat();
	UINT GetRTVDescriptorHandleIncrementSize();

		
	void ExecuteAndWaitForGPU();

protected:
	int32_t GetCurrentFrameIndex();
	void ResetCommandBeforeUse();

	// CPU-GPU synchronization
	void WaitForGPU();

private:
	void EnableDebugLayer();
	void CreateGraphicsCore();
	void GetAdapterAndOutputInformation(uint32_t screenWidth, uint32_t screenHeight);
	void CreateCommandQueue();
	void CreateSwapChain(HWND hwnd, uint32_t screenWidth, uint32_t screenHeight, bool fullscreen);
	void CacheDescriptorIncrementSize();
	void CreateRenderTargetView();
	void CreateDepthStencilView(uint32_t screenWidth, uint32_t screenHeight);
	void CreateViewportAndScissorRect(uint32_t screenWidth,uint32_t screenHeight);
	void CreateRenderMatrix(uint32_t screenWidth,uint32_t screenHeight,float screenNear,float screenFar);

private:
	bool m_vsyncEnabled;
	int32_t m_videoCardMemory;
	char m_videoCardDescription[128];
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;
	HANDLE m_fenceEvent;
	UINT m_rtvSize;
	UINT m_dsvSize;
	UINT m_frameIndex;
	uint32_t m_screenRefreshRate; // In hz

	bool m_isEnabledDepth;
	bool m_isEnabledAlphaBlending;
	bool m_isEnabledCulling;
	bool m_isEnabledWireframe;

	ComPtr<IDXGIFactory6> m_factory;
	ComPtr<IDXGIAdapter> m_adapter;
	ComPtr<ID3D12Device> m_device;

	ComPtr<IDXGISwapChain3> m_swapChain;
	
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	vector<ComPtr<ID3D12Resource>> m_renderTargets;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12Resource> m_depthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	XMMATRIX m_projectionMatrix;
	XMMATRIX m_worldMatrix;
	XMMATRIX m_orthoMatrix;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
};