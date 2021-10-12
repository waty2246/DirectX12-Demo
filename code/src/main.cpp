#include "stdafx.h"
#include "SystemApplication.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int32_t iCmdshow)
{
	unique_ptr<System> systemApplication;

	// Create the system object.
	systemApplication = make_unique<System>();

	// Initialize and run the system object.
	systemApplication->Initialize();

	systemApplication->Run();

	return 0;
}