#include "stdafx.h"
#include "config.h"
#include "SystemApplication.h"
#include "Application.h"
#include "Input.h"

System::System() :
	m_applicationName(nullptr),
	m_hinstance(nullptr),
	m_hwnd(nullptr)
{
}

System::~System()
{
#if !CONFIG_SHOW_CURSOR
	// Show the mouse cursor.
	ShowCursor(true);
#endif
	
#if USE_FULLSCREEN_WINDOW
	ChangeDisplaySettings(nullptr, 0);
#endif

	m_hwnd = nullptr;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = nullptr;

	// Release the pointer to this class.
	ApplicationHandle = nullptr;
}


void System::Initialize()
{
	
	uint32_t screenWidth, screenHeight;

	// Initialize the width and height of the screen to zero before sending the variables into the function.
	screenWidth = 0;
	screenHeight = 0;

	// Initialize the windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Create and initialize the input object.
	m_input = make_unique<Input>();
	m_input->Initialize();

	// Create and initialize the application wrapper object.
	m_application = make_unique<Application>();
	m_application->Initialize(m_input.get(),m_hinstance, m_hwnd, screenWidth, screenHeight);
}

void System::Run()
{
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));
	
	// Loop until there is a quit message from the window or the user.
	done = false;
	while(!done)
	{
		// Handle the windows messages.
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			result = Frame();
			if(!result)
			{
				done = true;
			}
		}

	}
}


bool System::Frame()
{
	bool result;


	// Do the frame processing for the application object.
	result = m_application->Frame();
	if (!result)
	{
		return false;
	}

	return true;
}


LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (m_input->HandleInput(hwnd, msg, wParam, lParam))
	{
		return 0;
	}
	
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


void System::InitializeWindows(uint32_t& screenWidth, uint32_t& screenHeight)
{
	WNDCLASSEX wclex;
	int32_t realScreenWidth = 0;
	int32_t realScreenHeight = 0;
	DWORD appStyle = WS_EX_APPWINDOW;

	// Get an external pointer to this object.	
	ApplicationHandle = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(nullptr);

	// Give the application a name.
	m_applicationName = L"Direct3D-Demo";

	// Setup the windows class with default settings.
	ZeroMemory(&wclex, sizeof(WNDCLASSEX));
	wclex.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wclex.lpfnWndProc   = WndProc;
	wclex.cbClsExtra    = 0;
	wclex.cbWndExtra    = 0;
	wclex.hInstance     = m_hinstance;
	wclex.hIcon		 = LoadIcon(nullptr, IDI_WINLOGO);
	wclex.hIconSm       = wclex.hIcon;
	wclex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wclex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wclex.lpszMenuName  = nullptr;
	wclex.lpszClassName = m_applicationName;
	wclex.cbSize        = sizeof(WNDCLASSEX);
	
	// Register the window class.
	ThrowIfTrue(RegisterClassEx(&wclex) == FALSE);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
#if USE_FULLSCREEN_WINDOW
	DEVMODE dmScreenSettings;

	// If full screen set the screen to maximum size of the users desktop and 32bit.
	ZeroMemory(&dmScreenSettings,sizeof(DEVMODE));
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth = static_cast<DWORD>(screenWidth);
	dmScreenSettings.dmPelsHeight = static_cast<DWORD>(screenHeight);
	dmScreenSettings.dmBitsPerPel = 32;
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	// Change the display settings to full screen.
	ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	realScreenWidth = screenWidth;
	realScreenHeight = screenHeight;
#else
	// If windowed then set it to 800x600 resolution.
	screenWidth = CONFIG_WINDOW_WIDTH;
	screenHeight = CONFIG_WINDOW_HEIGHT;

	RECT windowRect = { 0,0,screenWidth,screenHeight };

	ThrowIfTrue(AdjustWindowRect(&windowRect, appStyle, false) == FALSE);

	realScreenWidth = windowRect.right - windowRect.left;
	realScreenHeight = windowRect.bottom - windowRect.top;
#endif

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(
		appStyle,
		m_applicationName, 
		m_applicationName, 
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		0, 0,
		realScreenWidth, realScreenHeight,
		nullptr, nullptr, m_hinstance, nullptr
	);

	ThrowIfTrue(m_hwnd == nullptr);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

#if !CONFIG_SHOW_CURSOR
	// Hide the mouse cursor.
	ShowCursor(false);
#endif
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			DestroyWindow(hwnd);
			return 0;
		}

		// All other messages pass to the message handler in the system class.
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}