#pragma once
#include "include/Input/Mouse.h"
#include "include/Render/Texture.h"

struct IDXGISwapChain1;

namespace fth
{
	struct WindowSpecs;
	class EngineApp;


	class Win32Window
	{
	public:
		Win32Window(const WindowSpecs& specs);

		~Win32Window();


		Win32Window() = delete;
		//TODO: Win32Window Class: implement move constructor and move assignement operator
		Win32Window(const Win32Window& win) = delete;
		Win32Window operator=(const Win32Window& win) = delete;
		Win32Window(Win32Window&& win) = delete;
		Win32Window operator=(Win32Window&& win) = delete;

	public:
		void Init();
		void Shutdown();
		void OnAttach(uint16_t newWidth, uint16_t newHeight);

		inline const uint16_t& GetWindowWidth() const { return m_Width; }
		inline const uint16_t& GetWindowHeight() const { return m_Height; }

		inline const HWND& GetHWND() const { return m_HWND; }

		void SetParentIApp(EngineApp* IApp);
		inline EngineApp* GetParentIApp() { return m_ParentApplication; }

		void ResizeBackBuffers(uint16_t newWidth, uint16_t newHeight);
		static bool Resize(Win32Window& targetWindow, uint16_t newWidth, uint16_t newHeight);
		void UpdateWindowText(std::wstring_view text);

		inline DxResPtr<IDXGISwapChain1>& GetSwapChain() { return m_swapChain; }

		inline const fth::Texture& GetTexture() { return m_backBuffer; }

		void Present();

	private:
		static LRESULT CALLBACK WindowProcess(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
		
		void RegisterWindowClass();
		HWND CreateWindowHWND();

		void SetWindowWidth(const uint16_t& width) { m_Width = width; }
		void SetWindowHeight(const uint16_t& height) { m_Height = height; }

	private:

		void InitBackBuffer();
		void InitViewport();
		void ShutdownSwapChain();
		void ShutdownBackBuffer();
		void InitSwapChain();


	private:

		uint16_t                               m_Height;
		uint16_t                               m_Width;
		bool                                   m_IsFullscreen;
		

		//Win32 API internals

		HWND                                   m_HWND;
		std::wstring                           m_WindowClassName;
		std::wstring                           m_CurrentWindowTitle;
		std::wstring                           m_BaseWindowTitle;
		//icons...?

		EngineApp*                             m_ParentApplication;



	private:

		//only scene class is able to write in the back buffer
		//later, it will be a Renderer class




		bool                                                            m_isRendererAttached;
		fth::Texture                                                    m_backBuffer;
		DxResPtr<IDXGISwapChain1>                                       m_swapChain;
	
	};
}