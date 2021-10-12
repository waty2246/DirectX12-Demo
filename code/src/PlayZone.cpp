#include "stdafx.h"
#include "config.h"
#include "PlayZone.h"


PlayZone::PlayZone() : 
	m_displayUI(true),
	m_wireFrame(false),
	m_cellLines(false),
	m_heightLocked(true)
{
}

PlayZone::~PlayZone()
{
}

void PlayZone::Initialize(D3D* d3d, HWND hwnd, int32_t screenWidth, int32_t screenHeight, float screenDepth)
{
	// Create and initialize the user interface object.
	m_ui = make_unique<UserInterfaceClass>();
	m_ui->Initialize(d3d, screenHeight, screenWidth);

	// Create the camera object.
	m_camera = make_unique<Camera>();

	// Set the initial position of the camera and build the matrices needed for rendering.
	m_camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_camera->Render();
	m_camera->RenderBaseViewMatrix();

	// Create and initialize the light object.
	m_light = make_unique<LightClass>();
	m_light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_light->SetDirection(-0.5f, -1.0f, -0.5f);

	// Create and initialize the position object.
	m_position = make_unique<PlayerController>();
	m_position->SetPosition(512.5f, 10.0f, 10.0f);
	m_position->SetRotation(0.0f, 0.0f, 0.0f);

	// Create and initialize the frustum object.
	m_frustum = make_unique<Frustum>();
	m_frustum->Initialize(screenDepth);

	// Create and initialize the sky dome object.
	m_skyDome = make_unique<SkyDome>();
	m_skyDome->Initialize(d3d->GetDevice());

	// Create and initialize the terrain object.
	m_terrain = make_unique<Terrain>();
	m_terrain->Initialize(d3d, GetDataFilePathA("setup.txt"));
}

void PlayZone::Frame(D3D* d3d, Input* input, ShaderManager* shaderManager, Texture2DManager* textureManager, float frameTime, int32_t fps)
{
	bool foundHeight;
	float posX, posY, posZ, rotX, rotY, rotZ, height;


	// Do the frame input processing.
	HandleMovementInput(input, frameTime);

	// Get the view point32_t position/rotation.
	m_position->GetPosition(posX, posY, posZ);
	m_position->GetRotation(rotX, rotY, rotZ);

	// Do the frame processing for the user interface.
	m_ui->Frame(d3d->GetCommandList(), fps, posX, posY, posZ, rotX, rotY, rotZ);

	// Do the terrain frame processing.
	m_terrain->Frame();

	// If the height is locked to the terrain then position the camera on top of it.
	if(m_heightLocked)
	{
		// Get the height of the triangle that is directly underneath the given camera position.
		foundHeight = m_terrain->GetHeightAtPosition(posX, posZ, height);
		if(foundHeight)
		{
			// If there was a triangle under the camera then position the camera just above it by one meter.
			m_position->SetPosition(posX, height + CONFIG_BASE_HEIGHT_CAMERA, posZ);
			m_camera->SetPosition(posX, height + CONFIG_BASE_HEIGHT_CAMERA, posZ);
		}
	}

	// Render the graphics.
	Render(d3d, shaderManager, textureManager);
}

void PlayZone::HandleMovementInput(Input* input, float frameTime)
{
	float posX = 0.0f, posY = 0.0f, posZ = 0.0f, rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;

	// Set the frame time for calculating the updated position.
	m_position->SetFrameTime(frameTime);

	// Handle the input.
	m_position->TurnLeft(input->IsKeyDown(KeyCode::A));
	m_position->TurnRight(input->IsKeyDown(KeyCode::D));
	m_position->MoveForward(input->IsKeyDown(KeyCode::W));
	m_position->MoveBackward(input->IsKeyDown(KeyCode::S));
	m_position->MoveUpward(input->IsKeyDown(KeyCode::E));
	m_position->MoveDownward(input->IsKeyDown(KeyCode::Q));
	m_position->LookUpward(input->IsKeyDown(KeyCode::R));
	m_position->LookDownward(input->IsKeyDown(KeyCode::F));

	// Get the view point32_t position/rotation.
	m_position->GetPosition(posX, posY, posZ);
	m_position->GetRotation(rotX, rotY, rotZ);

	// Set the position of the camera.
	m_camera->SetPosition(posX, posY, posZ);
	m_camera->SetRotation(rotX, rotY, rotZ);

	// Determine if the user interface should be displayed or not.
	if(input->IsKeyDown(KeyCode::F1))
	{
		m_displayUI = !m_displayUI;
	}

	// Determine if the terrain should be rendered in wireframe or not.
	if(input->IsKeyDown(KeyCode::F2))
	{
		m_wireFrame = !m_wireFrame;
	}

	// Determine if we should render the lines around each terrain cell.
	if(input->IsKeyDown(KeyCode::F3))
	{
		m_cellLines = !m_cellLines;
	}

	// Determine if we should be locked to the terrain height when we move around or not.
	if(input->IsKeyDown(KeyCode::F4))
	{
		m_heightLocked = !m_heightLocked;
	}
}


void PlayZone::Render(D3D* d3d, ShaderManager* ShaderManager, Texture2DManager* TextureManager)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, baseViewMatrix, orthoMatrix;
	bool result =false;
	XMFLOAT3 cameraPosition{};
	int32_t i = 0;

	
	// Generate the view matrix based on the camera's position.
	m_camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	d3d->GetWorldMatrix(worldMatrix);
	m_camera->GetViewMatrix(viewMatrix);
	d3d->GetProjectionMatrix(projectionMatrix);
	m_camera->GetBaseViewMatrix(baseViewMatrix);
	d3d->GetOrthoMatrix(orthoMatrix);
	
	// Get the position of the camera.
	cameraPosition = m_camera->GetPosition();
	
	// Construct the frustum.
	m_frustum->ConstructFrustum(projectionMatrix, viewMatrix);

	// Clear the buffers to begin the scene.
	d3d->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Turn off back face culling and turn off the Z buffer.
	d3d->DisableCulling();
	d3d->DisableDepth();

	// Translate the sky dome to be centered around the camera position.
	worldMatrix = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

	// Render the sky dome using the sky dome shader.
	m_skyDome->Render(d3d->GetCommandList());
	ShaderManager->RenderSkyDomeShader(
		d3d->GetCommandList(), 
		m_skyDome->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_skyDome->GetApexColor(),
		m_skyDome->GetCenterColor()
	);

	// Reset the world matrix.
	d3d->GetWorldMatrix(worldMatrix);

	// Turn the Z buffer back and back face culling on.
	d3d->EnableDepth();
	d3d->EnableCulling();
	
	// Turn on wire frame rendering of the terrain if needed.
	if(m_wireFrame)
	{
		d3d->EnableWireframe();
	}

	// Render the terrain cells (and cell lines if needed).
	for(i=0; i<m_terrain->GetCellCount(); i++)
	{
		// Render each terrain cell if it is visible only.
		result = m_terrain->RenderCell(d3d->GetCommandList(), i, m_frustum.get());
		if(result)
		{
			// Render the cell buffers using the hgih quality terrain shader.
			ShaderManager->RenderTerrainShader(
				d3d->GetCommandList(),
				m_terrain->GetCellIndexCount(i), 
				worldMatrix, viewMatrix,projectionMatrix,
				TextureManager->GetTexture(0), 
				TextureManager->GetTexture(1),
				TextureManager->GetTexture(2), 
				TextureManager->GetTexture(3),
				m_light->GetDirection(), m_light->GetDiffuseColor()
			);

			// If needed then render the bounding box around this terrain cell using the color shader. 
			if(m_cellLines)
			{
				m_terrain->RenderCellLines(d3d->GetCommandList(), i);
				ShaderManager->RenderColorShader(
					d3d->GetCommandList(), 
					m_terrain->GetCellLinesIndexCount(i), 
					worldMatrix, viewMatrix, projectionMatrix
				);
			}
		}
	}
	
	// Turn off wire frame rendering of the terrain if it was on.
	if(m_wireFrame)
	{
		d3d->DisableWireframe();  
	}

	// Update the render counts in the UI.
	m_ui->UpdateRenderCounts(
		d3d->GetCommandList(), 
		m_terrain->GetRenderCount(), 
		m_terrain->GetCellsDrawn(), 
		m_terrain->GetCellsCulled()
	);

	// Render the user interface.
	if(m_displayUI)
	{
		m_ui->Render(d3d, ShaderManager, worldMatrix, baseViewMatrix, orthoMatrix);
	}

	// Present the rendered scene to the screen.
	d3d->EndScene();
}