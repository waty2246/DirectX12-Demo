#include "stdafx.h"
#include "Input.h"

Input::Input() :
    m_mouseX(0),
    m_mouseY(0),
    m_keyboardState(),
    m_mouseState()
{
}

Input::~Input()
{
}

void Input::Initialize()
{
    POINT mousePosition = { 0 };
    GetCursorPos(&mousePosition);

    m_mouseX = mousePosition.x;
    m_mouseY = mousePosition.y;
}

bool Input::IsKeyDown(KeyCode keyCode)
{
    return m_keyboardState[static_cast<size_t>(keyCode)];
}

bool Input::IsMouseButtonDown(MouseButton mouseButton)
{
    return m_mouseState[static_cast<size_t>(mouseButton)];
}

void Input::GetMousePosition(int32_t & x, int32_t & y)
{
    x = m_mouseX;
    y = m_mouseY;
}

bool Input::HandleInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_KEYDOWN:
        {
            m_keyboardState[static_cast<uint8_t>(wParam)] = true;
            return true;
        }
        case WM_KEYUP:
        {
            m_keyboardState[static_cast<uint8_t>(wParam)] = false;
            return true;
        }
        case WM_MOUSEMOVE:
        {
            m_mouseX = GET_X_LPARAM(lParam);
            m_mouseY = GET_Y_LPARAM(lParam);
            return true;
        }
        case WM_LBUTTONDOWN:
        {
            m_mouseState[static_cast<uint8_t>(MouseButton::LEFT)] = true;
            return true;
        }
        case WM_MBUTTONDOWN:
        {
            m_mouseState[static_cast<uint8_t>(MouseButton::MIDDLE)] = true;
            return true;
        }
        case WM_RBUTTONDOWN:
        {
            m_mouseState[static_cast<uint8_t>(MouseButton::RIGHT)] = true;
            return true;
        }
        case WM_LBUTTONUP:
        {
            m_mouseState[static_cast<uint8_t>(MouseButton::LEFT)] = false;
            return true;
        }
        case WM_MBUTTONUP:
        {
            m_mouseState[static_cast<uint8_t>(MouseButton::MIDDLE)] = false;
            return true;
        }
        case WM_RBUTTONUP:
        {
            m_mouseState[static_cast<uint8_t>(MouseButton::RIGHT)] = false;
            return true;
        }
    }

    return false;
}

