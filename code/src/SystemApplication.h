#pragma once

class Application;
class Input;

class System
{
public:
	System();
	~System();

	void Initialize();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(uint32_t & screenWidth, uint32_t& screenHeight);

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;
	unique_ptr<Application> m_application;
	unique_ptr<Input> m_input;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System* ApplicationHandle = 0;