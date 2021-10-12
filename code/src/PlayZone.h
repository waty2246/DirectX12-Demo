#pragma once

#include "D3D.h"
#include "Input.h"
#include "ShaderManager.h"
#include "Texture2DManager.h"
#include "GameTimer.h"
#include "UserInterface.h"
#include "Camera.h"
#include "Light.h"
#include "PlayerController.h"
#include "Frustum.h"
#include "Skydome.h"
#include "Terrain.h"

class PlayZone
{
public:
	PlayZone();
	~PlayZone();

	void Initialize(D3D* d3d, HWND hwnd, int32_t screenWidth, int32_t screenHeight, float screenDepth);
	void Frame(D3D*, Input*, ShaderManager*, Texture2DManager*, float, int32_t);

private:
	void HandleMovementInput(Input*, float);
	void Render(D3D*, ShaderManager*, Texture2DManager*);

private:
	unique_ptr<UserInterfaceClass> m_ui;
	unique_ptr<Camera> m_camera;
	unique_ptr<LightClass> m_light;
	unique_ptr<PlayerController> m_position;
	unique_ptr<Frustum> m_frustum;
	unique_ptr<SkyDome> m_skyDome;
	unique_ptr<Terrain> m_terrain;
	bool m_displayUI, m_wireFrame, m_cellLines, m_heightLocked;
};