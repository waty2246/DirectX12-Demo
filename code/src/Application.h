#pragma once

class Input;
class D3D;
class ShaderManager;
class Texture2DManager;
class GameTimer;
class Fps;
class PlayZone;

class Application
{
public:
	Application();
	~Application();

	void Initialize(Input* input, HINSTANCE hinstance, HWND hwnd, uint32_t screenWidth, uint32_t screenHeight);
	bool Frame();

private:
	Input* m_input;
	unique_ptr<D3D> m_d3d;
	unique_ptr<ShaderManager> m_shaderManager;
	unique_ptr<Texture2DManager> m_texture2DManager;
	unique_ptr<GameTimer> m_timer;
	unique_ptr<Fps> m_fps;
	unique_ptr<PlayZone> m_zone;
};