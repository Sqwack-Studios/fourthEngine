#pragma once
#include "include/Input/KeyCodes.h"
//this class should be able to find which window received the message, and find and appropiate owner/father class that handles
//proper functionality

//so far, we'll only handle a single window, and it will remain the same IMO, simplifying the problem


namespace fth
{
	class Win32Window;
	class Mouse;

	class WindowListener
	{
	public:
		static Win32Window* FindWin32WindowFromHWND(HWND hwnd);
		static void AddWindow(Win32Window* window);


		static void ProcessMouseInput(UINT message, WPARAM wParam, LPARAM lParam);
		static void OnResize(Win32Window& targetWindow, uint16_t newWidth, uint16_t newHeight);


		static void ProcessKeyboardInput(WPARAM wParam, LPARAM lParam);

		static void UpdateMousePositionMetrics(Win32Window& targetWindow, WPARAM wParam, LPARAM lParam);
		static void UpdateMouseWheel(Win32Window& targetWindow, WPARAM wParam, LPARAM lParam);

	private:

		static std::vector<Win32Window*>  m_engineWindows;
	};
}