#include "stdafx.h"
#include "UserInterface.h"
#include "D3D.h"
#include "Font.h"
#include "Text.h"
#include "MiniMap.h"


UserInterfaceClass::UserInterfaceClass()
{
}

UserInterfaceClass::~UserInterfaceClass()
{
}

void UserInterfaceClass::Initialize(D3D* d3d, uint32_t screenHeight, uint32_t screenWidth)
{
	auto device = d3d->GetDevice();
	auto commandList = d3d->GetCommandList();

	char videoCard[128];
	int32_t videoMemory;
	char videoString[144];
	char memoryString[32];
	char tempString[16];
	int32_t i;


	// Create the first font object.
	m_font = make_unique<Font>();

	// Initialize the first font object.
	m_font->Initialize(d3d, GetDataFilePathA("font/font01.txt"), GetDataFilePathA("font/font01.tga"), 32.0f, 3);

	// Create the text object for the fps string.
	m_fpsString = make_unique<Text>();

	// Initialize the fps text string.
	m_fpsString->Initialize(d3d, screenWidth, screenHeight, 16, false, m_font.get(), "Fps: 0", 10, 50, 0.0f, 1.0f, 0.0f);

	// Initial the previous frame fps.
	m_previousFps = -1;

	// Setup the video card strings.
	d3d->GetVideoCardInfo(videoCard, videoMemory);
	strcpy_s(videoString, "Video Card: ");
	strcat_s(videoString, videoCard);

	_itoa_s(videoMemory, tempString, 10);

	strcpy_s(memoryString, "Video Memory: ");
	strcat_s(memoryString, tempString);
	strcat_s(memoryString, " MB");

	// Create the text objects for the video strings.
	m_videoStrings = make_unique<Text[]>(2);

	// Initialize the video text strings.
	m_videoStrings[0].Initialize(d3d, screenWidth, screenHeight, 256, false, m_font.get(), videoString, 10, 10, 1.0f, 1.0f, 1.0f);

	m_videoStrings[1].Initialize(d3d, screenWidth, screenHeight, 32, false, m_font.get(), memoryString, 10, 30, 1.0f, 1.0f, 1.0f);

	// Create the text objects for the position strings.
	m_positionStrings = make_unique<Text[]>(6);

	// Initialize the position text strings.
	m_positionStrings[0].Initialize(d3d, screenWidth, screenHeight, 16, false, m_font.get(), "X: 0",  10, 310, 1.0f, 1.0f, 1.0f);

	m_positionStrings[1].Initialize(d3d, screenWidth, screenHeight, 16, false, m_font.get(), "Y: 0",  10, 330, 1.0f, 1.0f, 1.0f);

	m_positionStrings[2].Initialize(d3d, screenWidth, screenHeight, 16, false, m_font.get(), "Z: 0",  10, 350, 1.0f, 1.0f, 1.0f);

	m_positionStrings[3].Initialize(d3d, screenWidth, screenHeight, 16, false, m_font.get(), "rX: 0", 10, 370, 1.0f, 1.0f, 1.0f);

	m_positionStrings[4].Initialize(d3d, screenWidth, screenHeight, 16, false, m_font.get(), "rY: 0", 10, 390, 1.0f, 1.0f, 1.0f);

	m_positionStrings[5].Initialize(d3d, screenWidth, screenHeight, 16, false, m_font.get(), "rZ: 0", 10, 410, 1.0f, 1.0f, 1.0f); 

	// Initialize the previous frame position.
	for(i=0; i<6; i++)
	{
		m_previousPosition[i] = -1;
	}

	// Create the text objects for the render count strings.
	m_renderCountStrings = make_unique<Text[]>(3);

	// Initialize the render count strings.
	m_renderCountStrings[0].Initialize(d3d, screenWidth, screenHeight, 32, false, m_font.get(),"Polys Drawn: 0", 10, 260, 1.0f, 1.0f, 1.0f);

	m_renderCountStrings[1].Initialize(d3d, screenWidth, screenHeight, 32, false, m_font.get(), "Cells Drawn: 0", 10, 280, 1.0f, 1.0f, 1.0f);

	m_renderCountStrings[2].Initialize(d3d, screenWidth, screenHeight, 32, false, m_font.get(), "Cells Culled: 0", 10, 300, 1.0f, 1.0f, 1.0f);

	// Create the mini-map object.
	m_miniMap = make_unique<MiniMap>();

	// Initialize the mini-map object.
	m_miniMap->Initialize(d3d, screenWidth, screenHeight, 1025, 1025);
}

void UserInterfaceClass::Frame(ID3D12GraphicsCommandList* commandList, int32_t fps, float posX, float posY, float posZ, float rotX, float rotY, float rotZ)
{
	// Update the fps string.
	UpdateFpsString(commandList, fps);

	// Update the position strings.
	UpdatePositionStrings(commandList, posX, posY, posZ, rotX, rotY, rotZ);

	// Update the mini-map position indicator.
	m_miniMap->PositionUpdate(posX, posZ);
}


void UserInterfaceClass::Render(D3D* d3d, ShaderManager* shaderManager, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX orthoMatrix)
{
	int32_t i;

	// Turn off the Z buffer and enable alpha blending to begin 2D rendering.
	d3d->DisableDepth();
	d3d->EnableAlphaBlending();

	// Render the fps string.
	PIXBeginEvent(d3d->GetCommandList(), 0, L"Render FPS string");
	m_fpsString->Render(d3d->GetCommandList(), shaderManager, worldMatrix, viewMatrix, orthoMatrix, m_font->GetTexture());
	PIXEndEvent(d3d->GetCommandList());

	// Render the video card strings.
	m_videoStrings[0].Render(d3d->GetCommandList(), shaderManager, worldMatrix, viewMatrix, orthoMatrix, m_font->GetTexture());
	m_videoStrings[1].Render(d3d->GetCommandList(), shaderManager, worldMatrix, viewMatrix, orthoMatrix, m_font->GetTexture());

	// Render the position and rotation strings.
	for(i=0; i<6; i++)
	{
		m_positionStrings[i].Render(d3d->GetCommandList(), shaderManager, worldMatrix, viewMatrix, orthoMatrix, m_font->GetTexture());
	}

	// Render the render count strings.
	for(i=0; i<3; i++)
	{
		m_renderCountStrings[i].Render(d3d->GetCommandList(), shaderManager, worldMatrix, viewMatrix, orthoMatrix, m_font->GetTexture());
	}

	// Turn off alpha blending now that the text has been rendered.
	d3d->DisableAlphaBlending();

	// Render the mini-map.
	m_miniMap->Render(d3d->GetCommandList(), shaderManager, worldMatrix, viewMatrix, orthoMatrix);

	// Turn the Z buffer back on now that the 2D rendering has completed.
	d3d->EnableDepth();
}


void UserInterfaceClass::UpdateFpsString(ID3D12GraphicsCommandList* commandList, int32_t fps)
{
	char tempString[16];
	char finalString[16];
	float red, green, blue;


	// Check if the fps from the previous frame was the same, if so don't need to update the text string.
	if(m_previousFps == fps)
	{
		return;
	}

	// Store the fps for checking next frame.
	m_previousFps = fps;

	// Truncate the fps to below 100,000.
	if(fps > 99999)
	{
		fps = 99999;
	}

	// Convert the fps integer to string format.
	_itoa_s(fps, tempString, 10);

	// Setup the fps string.
	strcpy_s(finalString, "Fps: ");
	strcat_s(finalString, tempString);

	// If fps is 60 or above set the fps color to green.
	if(fps >= 60)
	{
		red = 0.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// If fps is below 60 set the fps color to yellow.
	if(fps < 60)
	{
		red = 1.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// If fps is below 30 set the fps color to red.
	if(fps < 30)
	{
		red = 1.0f;
		green = 0.0f;
		blue = 0.0f;
	}

	// Update the sentence vertex buffer with the new string information.
	m_fpsString->UpdateSentence(commandList, m_font.get(), finalString, 10, 50, red, green, blue);
}


void UserInterfaceClass::UpdatePositionStrings(ID3D12GraphicsCommandList* commandList, float posX, float posY, float posZ, 
											   float rotX, float rotY, float rotZ)
{
	int32_t positionX, positionY, positionZ, rotationX, rotationY, rotationZ;
	char tempString[16];
	char finalString[16];

	// Convert the float values to integers.
	positionX = (int)posX;
	positionY = (int)posY;
	positionZ = (int)posZ;
	rotationX = (int)rotX;
	rotationY = (int)rotY;
	rotationZ = (int)rotZ;

	// Update the position strings if the value has changed since the last frame.
	if(positionX != m_previousPosition[0])
	{
		m_previousPosition[0] = positionX;
		_itoa_s(positionX, tempString, 10);
		strcpy_s(finalString, "X: ");
		strcat_s(finalString, tempString);
		m_positionStrings[0].UpdateSentence(commandList, m_font.get(), finalString, 10, 100, 1.0f, 1.0f, 1.0f);
	}

	if(positionY != m_previousPosition[1])
	{
		m_previousPosition[1] = positionY;
		_itoa_s(positionY, tempString, 10);
		strcpy_s(finalString, "Y: ");
		strcat_s(finalString, tempString);
		m_positionStrings[1].UpdateSentence(commandList, m_font.get(), finalString, 10, 120, 1.0f, 1.0f, 1.0f);
	}

	if(positionZ != m_previousPosition[2])
	{
		m_previousPosition[2] = positionZ;
		_itoa_s(positionZ, tempString, 10);
		strcpy_s(finalString, "Z: ");
		strcat_s(finalString, tempString);
		m_positionStrings[2].UpdateSentence(commandList, m_font.get(), finalString, 10, 140, 1.0f, 1.0f, 1.0f);
	}

	if(rotationX != m_previousPosition[3])
	{
		m_previousPosition[3] = rotationX;
		_itoa_s(rotationX, tempString, 10);
		strcpy_s(finalString, "rX: ");
		strcat_s(finalString, tempString);
		m_positionStrings[3].UpdateSentence(commandList, m_font.get(), finalString, 10, 180, 1.0f, 1.0f, 1.0f);
	}

	if(rotationY != m_previousPosition[4])
	{
		m_previousPosition[4] = rotationY;
		_itoa_s(rotationY, tempString, 10);
		strcpy_s(finalString, "rY: ");
		strcat_s(finalString, tempString);
		m_positionStrings[4].UpdateSentence(commandList, m_font.get(), finalString, 10, 200, 1.0f, 1.0f, 1.0f);
	}

	if(rotationZ != m_previousPosition[5])
	{
		m_previousPosition[5] = rotationZ;
		_itoa_s(rotationZ, tempString, 10);
		strcpy_s(finalString, "rZ: ");
		strcat_s(finalString, tempString);
		m_positionStrings[5].UpdateSentence(commandList, m_font.get(), finalString, 10, 220, 1.0f, 1.0f, 1.0f);
	}
}


void UserInterfaceClass::UpdateRenderCounts(ID3D12GraphicsCommandList* commandList, int32_t renderCount, int32_t nodesDrawn, int32_t nodesCulled)
{
	char tempString[32];
	char finalString[32];


	// Convert the render count integer to string format.
	_itoa_s(renderCount, tempString, 10);

	// Setup the render count string.
	strcpy_s(finalString, "Polys Drawn: ");
	strcat_s(finalString, tempString);

	// Update the sentence vertex buffer with the new string information.
	m_renderCountStrings[0].UpdateSentence(commandList, m_font.get(), finalString, 10, 260, 1.0f, 1.0f, 1.0f);

	// Convert the cells drawn integer to string format.
	_itoa_s(nodesDrawn, tempString, 10);

	// Setup the cells drawn string.
	strcpy_s(finalString, "Cells Drawn: ");
	strcat_s(finalString, tempString);

	// Update the sentence vertex buffer with the new string information.
	m_renderCountStrings[1].UpdateSentence(commandList, m_font.get(), finalString, 10, 280, 1.0f, 1.0f, 1.0f);

	// Convert the cells culled integer to string format.
	_itoa_s(nodesCulled, tempString, 10);

	// Setup the cells culled string.
	strcpy_s(finalString, "Cells Culled: ");
	strcat_s(finalString, tempString);

	// Update the sentence vertex buffer with the new string information.
	m_renderCountStrings[2].UpdateSentence(commandList, m_font.get(), finalString, 10, 300, 1.0f, 1.0f, 1.0f);
}