#include "stdafx.h"
#include "config.h"
#include "Application.h"

#include "Input.h"
#include "D3D.h"
#include "ShaderManager.h"
#include "Texture2DManager.h"
#include "GameTimer.h"
#include "Fps.h"
#include "PlayZone.h"


Application::Application()
{
}

Application::~Application()
{
}


void Application::Initialize(Input* input, HINSTANCE hinstance, HWND hwnd, uint32_t screenWidth, uint32_t screenHeight)
{
	// Store the input object.
	m_input = input;

	// Create the Direct3D object.
	m_d3d = make_unique<D3D>();

	// Initialize the Direct3D object.
	m_d3d->Initialize(screenWidth, screenHeight, USE_VSYNC, hwnd, USE_FULLSCREEN_WINDOW, CONFIG_SCREEN_NEAR_PLANE_DEPTH, CONFIG_SCREEN_FAR_PLANE_DEPTH);

	// Create the shader manager object.
	m_shaderManager = make_unique<ShaderManager>();

	// Initialize the shader manager object.
	m_shaderManager->Initialize(m_d3d.get());

	// Create the texture manager object.
	m_texture2DManager = make_unique<Texture2DManager>();

	// Initialize the texture manager object.
	m_texture2DManager->Initialize(10);

	// Load textures into the texture manager.
	m_texture2DManager->LoadTexture(m_d3d.get(), GetTextureFilePathA("rock01d.tga"), 0);
	m_texture2DManager->LoadTexture(m_d3d.get(), GetTextureFilePathA("rock01n.tga"), 1);
	m_texture2DManager->LoadTexture(m_d3d.get(), GetTextureFilePathA("snow01n.tga"), 2);
	m_texture2DManager->LoadTexture(m_d3d.get(), GetTextureFilePathA("distance01n.tga"), 3);

	// Create the timer object.
	m_timer = make_unique<GameTimer>();

	// Initialize the timer object.
	m_timer->Initialize();

	// Create the fps object.
	m_fps = make_unique<Fps>();

	// Initialize the fps object.
	m_fps->Initialize();

	// Create the zone object.
	m_zone = make_unique<PlayZone>();

	// Initialize the zone object.
	m_zone->Initialize(m_d3d.get(), hwnd, screenWidth, screenHeight, CONFIG_SCREEN_FAR_PLANE_DEPTH);

	m_d3d->GetCommandList()->Close();
}

bool Application::Frame()
{
	// Update the system stats.
	m_fps->Frame();
	m_timer->Frame();

	// Check if the user pressed escape and wants to exit the application.
	if(m_input->IsKeyDown(KeyCode::ESCAPE))
	{
		return false;
	}

	// Do the zone frame processing.
	m_zone->Frame(m_d3d.get(), m_input, m_shaderManager.get(), m_texture2DManager.get(), m_timer->GetTime(), m_fps->GetFps());

	return true;
}