#pragma once

enum class KeyCode : uint8_t
{
	NONE = 0,
	SHIFT = VK_SHIFT,
	ALT = VK_MENU,
	CONTROL = VK_CONTROL,
	LEFT = VK_LEFT,
	RIGHT = VK_RIGHT,
	UP = VK_UP,
	DOWN = VK_DOWN,
	ENTER = VK_RETURN,
	A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	F1 = VK_F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	ESCAPE = VK_ESCAPE,
	UNKNOWN = 0xFF
};

enum class MouseButton : uint8_t
{
	LEFT = 0,
	MIDDLE = 1,
	RIGHT = 2
};

class Input
{
public:
	Input();
	~Input();

	void Initialize();

	bool IsKeyDown(KeyCode keyCode);
	bool IsMouseButtonDown(MouseButton mouseButton);
	void GetMousePosition(int32_t & x, int32_t & y);

	bool HandleInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	bool m_keyboardState[256];
	bool m_mouseState[3];
	int32_t m_mouseX;
	int32_t m_mouseY;
};