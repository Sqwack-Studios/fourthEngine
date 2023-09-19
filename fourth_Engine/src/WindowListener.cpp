#include "pch.h"

#pragma once
#include "include/WindowListener.h"
#include "include/App.h"
#include "include/Win32Window.h"
#include "include/Input/InputController.h"


namespace fth
{

	std::vector<Win32Window*> WindowListener::m_engineWindows;

	Win32Window* WindowListener::FindWin32WindowFromHWND(HWND hwnd)
	{

		for (Win32Window* window : m_engineWindows)
		{
			if (hwnd && window->GetHWND() && window->GetHWND() == hwnd)
			{
				return window;
			}
		}
		return nullptr;
	}

	void WindowListener::AddWindow(Win32Window* window)
	{
		m_engineWindows.push_back(window);
	}

	
	void WindowListener::ProcessKeyboardInput(WPARAM wParam, LPARAM lParam)
	{
		InputController::Get().ProcessKeyboardInput(wParam, lParam);
	}


	void WindowListener::ProcessMouseInput(UINT message, WPARAM wParam, LPARAM lParam)
	{


		switch (message)
		{
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		{
			InputController::Get().ProcessMouseInput(MouseKey::LMB, wParam, lParam);
		}
			break;
		case WM_RBUTTONUP:
		case WM_RBUTTONDOWN:
		{
			InputController::Get().ProcessMouseInput(MouseKey::RMB, wParam, lParam);
		}
			break;
		case WM_MBUTTONUP:
		case WM_MBUTTONDOWN:
		{
			InputController::Get().ProcessMouseInput(MouseKey::MMB, wParam, lParam);
		}
			break;
		default:
			break;
		}
		
	}

	void WindowListener::OnResize(Win32Window& targetWindow, uint16_t newWidth, uint16_t newHeight)
	{
		if(targetWindow.GetParentIApp())
				targetWindow.GetParentIApp()->OnResize(newWidth, newHeight);
	}





	void WindowListener::UpdateMouseWheel(Win32Window& targetWindow, WPARAM wParam, LPARAM lParam)
	{
		InputController::Get().ProcessMouseWheel(wParam, lParam);
	}

	void WindowListener::UpdateMousePositionMetrics(Win32Window& targetWindow, WPARAM wParam, LPARAM lParam)
	{
		InputController::Get().ProcessMousePositionMetrics(targetWindow.GetWindowWidth(), targetWindow.GetWindowHeight(), wParam, lParam);
	}
}

