#include "pch.h"

#pragma once
#include "include/Win32Window.h"
#include "include/Specifications.h"
#include "include/WindowListener.h"
#include "include/App.h"
#include "include/Render/Renderer/D3DRenderer.h"

namespace fth
{
	using namespace DirectX;

	Win32Window::Win32Window(const WindowSpecs& specs):
		m_Width(specs.windowWidth),
		m_Height(specs.windowHeight),
		m_HWND(nullptr),
		m_IsFullscreen(specs.fullscreen)
	{
		{
			m_WindowClassName = m_BaseWindowTitle = m_CurrentWindowTitle = specs.windowName;
		}

		TextureDsc txDsc;
		txDsc.height = 0;
		txDsc.width = 0;
		txDsc.depth = 0;
		txDsc.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		txDsc.dimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
		txDsc.arraySize = 2;
		txDsc.numTextures = 2;
		txDsc.multisamples = 1;
		txDsc.numMips = 1;
		txDsc.isCubemap = false;
		m_backBuffer.SetDesc(txDsc);
	}

	Win32Window::~Win32Window()
	{}

	void Win32Window::Init()
	{
		//Register window class
		RegisterWindowClass();
		//Create its HWND
		m_HWND = CreateWindowHWND();
		//Show the window if everything is ok
		if(m_HWND)
		{
			ShowWindow(m_HWND, SW_SHOW);
			WindowListener::AddWindow(this);
		}
	}

	void Win32Window::OnAttach(uint16_t newWidth, uint16_t newHeight)
	{
		m_isRendererAttached = true;

		Resize(*this, newWidth, newHeight);
		InitSwapChain();
		ResizeBackBuffers(newWidth, newHeight);
	}

	void Win32Window::InitSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC1 desc;

		// clear out the struct for use
		memset(&desc, 0, sizeof(DXGI_SWAP_CHAIN_DESC1));

		// fill the swap chain description struct
		desc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.Width = (UINT)m_backBuffer.Width();
		desc.Height = (UINT)m_backBuffer.Width();
		desc.BufferCount = 2;
		desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.Flags = 0;
		desc.Format = m_backBuffer.Format();
		desc.SampleDesc.Count = (UINT) m_backBuffer.Multisamples();                               // how many multisamples
		desc.SampleDesc.Quality = 0;                             // ???
		desc.Scaling = DXGI_SCALING_NONE;
		desc.Stereo = false;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		DX_CALL(s_factory->CreateSwapChainForHwnd(s_device, GetHWND(), &desc, NULL, NULL, m_swapChain.reset()));
		

	}

	void Win32Window::InitBackBuffer()
	{
		if (m_backBuffer.GetResource().valid())
		{
			m_backBuffer.GetResource().release();
			DX_CALL(m_swapChain->ResizeBuffers(0, (UINT)m_backBuffer.Width(), (UINT)m_backBuffer.Height(), DXGI_FORMAT_UNKNOWN, 0));
		}

		DX_CALL(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)m_backBuffer.GetResource().reset()));

	}


	void Win32Window::InitViewport()
	{
		D3D11_VIEWPORT vp;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = static_cast<float>(m_Width);
		vp.Height = static_cast<float>(m_Height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		
		renderer::D3DRenderer::Get().SetMainViewport(vp);
	}

	void Win32Window::ShutdownSwapChain()
	{
		m_swapChain.release();
	}

	void Win32Window::ShutdownBackBuffer()
	{
		m_backBuffer.GetResource().release();
	}


	void Win32Window::Shutdown()
	{
		ShutdownBackBuffer();
		ShutdownSwapChain();
	}

	void Win32Window::SetParentIApp(EngineApp* IApp)
	{
		m_ParentApplication = IApp;
	}


	void Win32Window::Present()
	{
		DX_CALL(m_swapChain->Present(0, 0));
	}


	void Win32Window::RegisterWindowClass()
	{
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW; //affects every single instance of this window class
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);


		wcex.hIcon = LoadIcon(NULL, IDI_QUESTION);
		wcex.hIconSm = LoadIcon(NULL, IDI_QUESTION);
		wcex.lpszClassName = m_WindowClassName.c_str();
		wcex.lpszMenuName = nullptr;
		wcex.hInstance = GetModuleHandle(NULL);
		wcex.lpfnWndProc = Win32Window::WindowProcess;

		FTH_VERIFY_HR(RegisterClassEx(&wcex));

	}

	HWND Win32Window::CreateWindowHWND()
	{
		uint16_t screenWidth = static_cast<uint16_t>(GetSystemMetrics(SM_CXSCREEN));
		uint16_t screenHeight = static_cast<uint16_t>(GetSystemMetrics(SM_CYSCREEN));

		RECT windowRect{ 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };

		AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

		uint16_t windowWidth = static_cast<uint16_t>(windowRect.right - windowRect.left);
		uint16_t windowHeight = static_cast<uint16_t>(windowRect.bottom - windowRect.top);

		//Center the window within the screen, clamp for (0,0) top-left corner

		uint16_t screenX = std::max<uint16_t>(0, (screenWidth - windowWidth) / 2);
		uint16_t screenY = std::max<uint16_t>(0, (screenHeight - windowHeight) / 2);


		HWND hWnd = CreateWindowExW(
			NULL,
			m_WindowClassName.c_str(),
			m_CurrentWindowTitle.c_str(),
			WS_OVERLAPPEDWINDOW,
			screenX,
			screenY,
			windowWidth,
			windowHeight,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL
		);

		FTH_VERIFY_ENGINE(hWnd, "Failed to create WHND");

		return hWnd;
	}

	void Win32Window::ResizeBackBuffers(uint16_t newWidth, uint16_t newHeight)
	{
		//First, lets verify if we actually need to resize buffers. If multisampling is different, then we have to recreate everything

		bool differentRes{ newWidth != m_backBuffer.Width() || newHeight != m_backBuffer.Height() };
		TextureDsc txDsc = m_backBuffer.GetDesc();

		txDsc.width = newWidth;
		txDsc.height = newHeight;
		m_backBuffer.SetDesc(txDsc);

		if (differentRes)
		{
			InitBackBuffer();
			InitViewport();
		}
	}

	bool Win32Window::Resize(Win32Window& targetWindow, uint16_t newWidth, uint16_t newHeight)
	{
		if (targetWindow.GetWindowWidth() != newWidth || targetWindow.GetWindowHeight() != newHeight)
		{
			
			uint16_t correctedWidth{ (std::max)(static_cast<uint16_t>(10), newWidth) };
			uint16_t correctedHeight{ (std::max)(static_cast<uint16_t>(10), newHeight) };
			targetWindow.SetWindowWidth(correctedWidth);
			targetWindow.SetWindowHeight(correctedHeight);

			RECT windowRect{ 0, 0, static_cast<LONG>(correctedWidth), static_cast<LONG>(correctedHeight) };

			AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
			return true;
		}
		return false;
	}

	void Win32Window::UpdateWindowText(std::wstring_view text)
	{
		
		std::wstring updatedTitle{ m_BaseWindowTitle + (std::wstring)(text) };
		::SetWindowText(GetHWND(), updatedTitle.c_str());
	}

	LRESULT Win32Window::WindowProcess(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
	{

		Win32Window* targetWindow = WindowListener::FindWin32WindowFromHWND(hWnd);

		if (targetWindow)
		{
			switch (message)
			{
			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			case WM_SYSKEYUP:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
			{
				WindowListener::ProcessKeyboardInput(wparam, lparam);
			}
			break;
			//TODO: check to mask lparams and wparams and set multiple inputs at the same time
			case WM_MOUSEWHEEL:
			{
				WindowListener::UpdateMouseWheel(*targetWindow, wparam, lparam);
			}
			break;
			case WM_MOUSEMOVE:
			{
				WindowListener::UpdateMousePositionMetrics(*targetWindow, wparam, lparam);
			}
			break;

			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			{
				WindowListener::ProcessMouseInput(message, wparam, lparam);
			}
			break;
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			{
				WindowListener::ProcessMouseInput(message, wparam, lparam);
			}
			break;
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			{
				WindowListener::ProcessMouseInput(message, wparam, lparam);
			}
	
			break;
			case WM_SIZE:
			{
				RECT clientRect{};
				::GetClientRect(targetWindow->GetHWND(), &clientRect);
				uint16_t newWidth = static_cast<uint16_t>(clientRect.right - clientRect.left);
				uint16_t newHeight = static_cast<uint16_t>(clientRect.bottom - clientRect.top);

				WindowListener::OnResize(*targetWindow, newWidth, newHeight);

			}
			break;
			default:
				return DefWindowProcW(hWnd, message, wparam, lparam);
				break;
			}
		}
		
		return DefWindowProcW(hWnd, message, wparam, lparam);
	}
}