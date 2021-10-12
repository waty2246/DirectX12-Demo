#include "stdafx.h"
#include "config.h"
#include "D3D.h"

#if USE_HIGH_PERFORMANCE_GPU && USE_GLOBAL_VARIABLE_METHOD_FOR_HIGH_PERFORMANCE_GPU
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int32_t AmdPowerXpressRequestHighPerformance = 1;
}
#endif

D3D::D3D() : 
	m_vsyncEnabled(false),
	m_videoCardMemory(0),
	m_renderTargets(CONFIG_BACK_BUFFER_COUNT),
	m_fenceValue(1),
	m_fenceEvent(nullptr),
	m_rtvSize(0),
	m_dsvSize(0),
	m_frameIndex(0),
	m_projectionMatrix(),
	m_worldMatrix(),
	m_orthoMatrix(),
	m_viewport(),
	m_scissorRect(),
	m_videoCardDescription()
{
}

D3D::~D3D()
{
}


void D3D::Initialize(uint32_t screenWidth, uint32_t screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenNear, float screenFar)
{
	float fieldOfView = 0.0f, screenAspect = 0.0f;
	UINT dxgiFactoryFlags = 0;

	ComPtr<IDXGIFactory6> factory;
	ComPtr<IDXGIAdapter> adapter;
	ComPtr<IDXGIOutput> adapterOutput;

	
	
	
	D3D12_VIEWPORT viewport;
	
	ZeroMemory(&viewport, sizeof(D3D12_VIEWPORT));

	// Store the vsync setting.
	m_vsyncEnabled = vsync;

	this->EnableDebugLayer();
	this->CreateGraphicsCore();
	this->GetAdapterAndOutputInformation(screenWidth, screenHeight);
	this->CreateCommandQueue();
	this->CreateSwapChain(hwnd,screenWidth,screenHeight,fullscreen);
	this->CacheDescriptorIncrementSize();

	// Initilize current frame index
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	this->CreateRenderTargetView();
	this->CreateDepthStencilView(screenWidth,screenHeight);
	this->CreateViewportAndScissorRect(screenWidth, screenHeight);
	this->CreateRenderMatrix(screenWidth, screenHeight, screenNear, screenFar);
}

void D3D::BeginScene(float red, float green, float blue, float alpha)
{
	// Reset command before use
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr = rtvHandle.ptr + m_frameIndex * m_rtvSize;
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	// Setup the color to clear the buffer to.
	const float clearColor[] = { red, green,blue, alpha };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor,0,nullptr);
    
	// Clear the depth buffer.
	m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH,0.0f,0,0,nullptr);
}


void D3D::EndScene()
{
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET , D3D12_RESOURCE_STATE_PRESENT));
	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* ppGraphicsCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppGraphicsCommandLists), ppGraphicsCommandLists);

	// Present the back buffer to the screen since rendering is complete.
	if(m_vsyncEnabled)
	{
		// Lock to screen refresh rate.
		m_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		m_swapChain->Present(0, 0);
	}

	this->WaitForGPU();

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}


ID3D12Device* D3D::GetDevice()
{
	return m_device.Get();
}


ID3D12GraphicsCommandList* D3D::GetCommandList()
{
	return m_commandList.Get();
}


void D3D::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}


void D3D::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
}


void D3D::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
}


void D3D::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
}


void D3D::EnableDepth()
{
	m_isEnabledDepth = true;
}

void D3D::DisableDepth()
{
	m_isEnabledDepth = false;
}

void D3D::EnableAlphaBlending()
{
	m_isEnabledAlphaBlending = true;
}

void D3D::DisableAlphaBlending()
{
	m_isEnabledAlphaBlending = false;
}

void D3D::EnableCulling()
{
	m_isEnabledCulling = true;
}

void D3D::DisableCulling()
{
	m_isEnabledCulling = false;
}

void D3D::EnableWireframe()
{
	m_isEnabledWireframe = true;
}


void D3D::DisableWireframe()
{
	m_isEnabledWireframe = false;
}

bool D3D::IsEnabledDepth()
{
	return false;
}

bool D3D::IsEnabledCulling()
{
	return false;
}

bool D3D::IsEnabledAlphaBlending()
{
	return false;
}

bool D3D::IsEnabledWireframe()
{
	return false;
}

DXGI_FORMAT D3D::GetDepthStencilBufferFormat()
{
	return DXGI_FORMAT_D24_UNORM_S8_UINT;
}

UINT D3D::GetRTVDescriptorHandleIncrementSize()
{
	return m_rtvSize;
}

void D3D::ExecuteAndWaitForGPU()
{
	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* ppGraphicsCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppGraphicsCommandLists), ppGraphicsCommandLists);

	this->WaitForGPU();
	this->ResetCommandBeforeUse();
}

int32_t D3D::GetCurrentFrameIndex()
{
	return m_frameIndex;
}

void D3D::ResetCommandBeforeUse()
{
	// Reset command before use
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);
}

void D3D::WaitForGPU()
{
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	++m_fenceValue;

	// To make the application simple, we will wait for current frame finish instead of go to next frame.
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void D3D::EnableDebugLayer()
{
#if !defined(NDEBUG)
	ComPtr<ID3D12Debug> debugLayer;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)));

	debugLayer->EnableDebugLayer();
#endif
}

void D3D::CreateGraphicsCore()
{
	uint32_t factoryCreationFlags = 0;
#if !defined(NDEBUG)
	factoryCreationFlags |= DXGI_CREATE_FACTORY_DEBUG;

#endif
	// Create a DirectX graphics interface factory.
	{
		ComPtr<IDXGIFactory2> tmpFactory;
		ThrowIfFailed(CreateDXGIFactory2(factoryCreationFlags, IID_PPV_ARGS(&tmpFactory)));
		tmpFactory.As(&m_factory);
	}

	// Use the factory to create an adapter for the primary graphics interface (video card) based on high performance preference.
#if USE_HIGH_PERFORMANCE_GPU && !USE_GLOBAL_VARIABLE_METHOD_FOR_HIGH_PERFORMANCE_GPU
	{
		ThrowIfFailed(m_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter)));
	}

#else
	ThrowIfFailed(m_factory->EnumAdapters(0, &m_adapter));
#endif

	//Create device with minimum support for directx 11 compatible.
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	ThrowIfFailed(D3D12CreateDevice(m_adapter.Get(), featureLevel, IID_PPV_ARGS(&m_device)));
}

void D3D::GetAdapterAndOutputInformation(uint32_t screenWidth, uint32_t screenHeight)
{
	uint32_t numModes = 0, numerator = 1, denominator = 1;
	unsigned long long stringLength = 0;
	int32_t error = 0;
	ComPtr<IDXGIOutput> adapterOutput;

	// Enumerate the primary adapter output (monitor).
	ThrowIfFailed(m_adapter->EnumOutputs(0, &adapterOutput));

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	ThrowIfFailed(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL));

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	unique_ptr<DXGI_MODE_DESC[]> displayModeList;
	displayModeList = std::make_unique<DXGI_MODE_DESC[]>(numModes);

	// Now fill the display mode list structures.
	ThrowIfFailed(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.get()));

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (uint32_t i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	if (numerator == 0 || denominator == 0)
	{
		m_screenRefreshRate = 60;
	}
	else
	{
		m_screenRefreshRate = numerator / denominator;
		m_screenRefreshRate = m_screenRefreshRate < 60 ? 60 : m_screenRefreshRate;
	}

	// Get the adapter (video card) description.
	DXGI_ADAPTER_DESC adapterDesc;
	ZeroMemory(&adapterDesc, sizeof(DXGI_ADAPTER_DESC));

	ThrowIfFailed(m_adapter->GetDesc(&adapterDesc));

	// Store the dedicated video card memory in megabytes.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	ThrowIfTrue(error != 0);
}

void D3D::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC graphicsCommandQueueDesc;
	ZeroMemory(&graphicsCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	graphicsCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&graphicsCommandQueueDesc, IID_PPV_ARGS(&m_commandQueue)));

	// Create Command List
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	ThrowIfTrue(m_fenceEvent == nullptr);
}

void D3D::CreateSwapChain(HWND hwnd, uint32_t screenWidth, uint32_t screenHeight, bool fullscreen)
{
#if USE_LEGACY_SWAPCHAIN_CREATION
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

		// Set to a single back buffer.
		swapChainDesc.BufferCount = CONFIG_BACK_BUFFER_COUNT;

		// Set the width and height of the back buffer.
		swapChainDesc.BufferDesc.Width = screenWidth;
		swapChainDesc.BufferDesc.Height = screenHeight;

		// Set regular 32-bit surface for the back buffer.
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Set the refresh rate of the back buffer.
		if (m_vsyncEnabled)
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = m_screenRefreshRate;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}
		else
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}

		// Set the usage of the back buffer.
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		// Set the handle for the window to render to.
		swapChainDesc.OutputWindow = hwnd;

		// Turn multisampling off.
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		// Set to full screen or windowed mode.
		if (fullscreen)
		{
			swapChainDesc.Windowed = false;
		}
		else
		{
			swapChainDesc.Windowed = true;
		}

		// Discard the back buffer contents after presenting.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		ComPtr<IDXGISwapChain> tempSwapChain;
		ThrowIfFailed(m_factory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, &tempSwapChain));
		tempSwapChain.As(&m_swapChain);
	}
#else
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	swapChainDesc.BufferCount = CONFIG_BACK_BUFFER_COUNT;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Width = screenWidth;
	swapChainDesc.Height = screenHeight;

	ComPtr<IDXGISwapChain1> tempSwapChain;
	ThrowIfFailed(m_factory->CreateSwapChainForHwnd(m_commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &tempSwapChain));
	tempSwapChain.As(&m_swapChain);
#endif
}

void D3D::CacheDescriptorIncrementSize()
{
	// Get RTV and DSV increment size
	m_rtvSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void D3D::CreateRenderTargetView()
{
	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	rtvDesc.NumDescriptors = CONFIG_BACK_BUFFER_COUNT;
	rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_rtvHeap)));

	// Get the pointer to the back buffer and create RTV
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (int32_t i = 0; i < CONFIG_BACK_BUFFER_COUNT; ++i)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr = rtvHandle.ptr + m_rtvSize;
	}
}

void D3D::CreateDepthStencilView(uint32_t screenWidth,uint32_t screenHeight)
{
	// Create DSV
	D3D12_RESOURCE_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D12_RESOURCE_DESC));
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Width = screenWidth;
	depthStencilDesc.Height = screenHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue;
	ZeroMemory(&clearValue, sizeof(D3D12_CLEAR_VALUE));
	clearValue.Format = depthStencilDesc.Format;

	D3D12_HEAP_PROPERTIES depthHeapProperties;
	ZeroMemory(&depthHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	depthHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&depthHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&clearValue,
		IID_PPV_ARGS(&m_depthStencilBuffer))
	);

	D3D12_DESCRIPTOR_HEAP_DESC depthStencilHeapDesc;
	ZeroMemory(&depthStencilHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	depthStencilHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	depthStencilHeapDesc.NumDescriptors = 1;

	ThrowIfFailed(m_device->CreateDescriptorHeap(&depthStencilHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Format = depthStencilDesc.Format;

	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_RESOURCE_BARRIER depthBarrier;
	ZeroMemory(&depthBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	depthBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	depthBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	depthBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	depthBarrier.Transition.pResource = m_depthStencilBuffer.Get();
	depthBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &depthBarrier);
}

void D3D::CreateViewportAndScissorRect(uint32_t screenWidth, uint32_t screenHeight)
{
	// Setup the viewport for rendering.
	m_viewport.Width = (float)screenWidth;
	m_viewport.Height = (float)screenHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = screenWidth;
	m_scissorRect.bottom = screenHeight;
}

void D3D::CreateRenderMatrix(uint32_t screenWidth, uint32_t screenHeight, float screenNear, float screenFar)
{
	// Setup the projection matrix.
	float fieldOfView = 3.141592654f / 4.0f;
	float screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenFar);

	// Initialize the world matrix to the identity matrix.
	m_worldMatrix = XMMatrixIdentity();

	// Create an orthographic projection matrix for 2D rendering.
	m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenFar);
}
