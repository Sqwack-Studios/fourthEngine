#include "pch.h"


#pragma once
#include "include/App.h"
#include "include/Win32Window.h"
#include "include/Specifications.h"
#include "include/ParallelExecutor.h"
#include "include/Input/InputController.h"
#include "include/Render/Renderer/ConstantBuffers.h"
#include "include/Managers/TextureManager.h"
#include "include/Managers/ModelManager.h"
#include "include/Systems/TransformSystem.h"
#include "include/Systems/LightSystem.h"
#include "include/Systems/MeshSystem.h"
#include "include/Systems/ParticleSystem.h"
#include "include/Systems/DecalSystem.h"
#include "include/Render/Renderer/ShadingGroups/ShadingGroup.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "include/Render/Renderer/PostProcess.h"
#include "include/Utils/DxRes.h"
#include "include/Utils/Random.h"

namespace fth
{
	void Init(uint16_t width, uint16_t height, uint16_t multisamples)
	{
		LogSystem::Get().Init();
		Random::Init();
		InputController::Get().Init();
		renderer::D3DRenderer::Get().Init(width, height, multisamples);
		renderer::PostProcess::Get().Init();
		TextureManager::Get().Init();
		ModelManager::Get().Init();
		TransformSystem::Get().Init();
		LightSystem::Get().Init();
		MeshSystem::Get().Init();
		ParticleSystem::Get().Init();
		DecalSystem::Get().Init();
	}

	void Shutdown()
	{
		DecalSystem::Get().Shutdown();
		ParticleSystem::Get().Shutdown();
		MeshSystem::Get().Shutdown();
		LightSystem::Get().Shutdown();
		TransformSystem::Get().Shutdown();
		TextureManager::Get().Shutdown();
		ModelManager::Get().Shutdown();
		renderer::D3DRenderer::Get().Shutdown();
		InputController::Get().Shutdown();
		LogSystem::Get().Shutdown();
	}
}



namespace fth
{
	std::unique_ptr<Win32Window> EngineApp::m_window = nullptr;

	EngineApp::EngineApp(const EngineAppSpecs& specs):
		m_appSpecs(specs)
	{


		// Set numThreads to the number of logical cores minus 4 if there are more than 8, otherwise just half of them
		uint32_t numThreads = (std::max)(1u, (std::max)(ParallelExecutor::MAX_THREADS > 4u ? ParallelExecutor::MAX_THREADS - 4u : 1u, ParallelExecutor::HALF_THREADS));
		m_executor = std::make_unique<ParallelExecutor>(numThreads);

		float frameTime{ 1.0f / specs.targetFPS };
		m_Tick.SetTargetPeriod(frameTime);

	}

	EngineApp::~EngineApp()
	{
		

		
	}




	void EngineApp::Init()
	{
		fth::Init(m_appSpecs.windowWidth, m_appSpecs.windowHeight, m_appSpecs.multisamples);
	}

	void EngineApp::Shutdown()
	{
		OnShutdown();
		ShutdownWindow();
		fth::Shutdown();
	}

	void EngineApp::AttachWindow(std::unique_ptr<Win32Window>&& window)
	{
		m_window = std::move(window);
		m_window->OnAttach(m_appSpecs.windowWidth, m_appSpecs.windowHeight);
		renderer::D3DRenderer::Get().AttachLDR_Target(m_window->GetTexture());
	}

	void EngineApp::ToggleLuminanceDebugging()
	{
		renderer::D3DRenderer::Get().ToggleLuminanceDebugging();
	}

	void EngineApp::SetLuminanceDebugging(bool debugLuminance)
	{
		renderer::D3DRenderer::Get().SetLuminanceDebugging(debugLuminance);
	}

	void EngineApp::ShutdownWindow()
	{
		m_window->Shutdown();
	}

	void EngineApp::Run(EngineApp* app)
	{
		m_window->SetParentIApp(app);
		PostInit();


		m_Tick.Start();
		MSG msg{};
		while (true)
		{
			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) //no msg filter, removed after processed.
			{
				if (msg.message == WM_QUIT)
					return; //why break? if msg is quit, shut the program, right?

				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}


			if (m_Tick.IsPeriodCompleted())
			{
				/*LOG_ENGINE_TRACE("", "Entering loop");*/
				InputController::Get().Update();
				float deltaTime{ m_Tick.CalculateDeltaTimeSec() };
				StartFrame();

				UpdateInternal(deltaTime);
				renderer::D3DRenderer::Get().Render();

				OnRender();
				EndFrame();
			}




			std::this_thread::yield();
			
	
		}


	}

	void EngineApp::UpdateInternal(float deltaTime)
	{
		Update(deltaTime);
		MeshSystem::Get().Update(deltaTime);
		LightSystem::Get().Update();
		ParticleSystem::Get().Update(deltaTime);
		DecalSystem::Get().update();

		uint16_t renderWidth { m_window ? m_window->GetWindowWidth() : m_appSpecs.windowWidth};
		uint16_t renderHeight { m_window ? m_window->GetWindowHeight() : m_appSpecs.windowHeight };
		renderer::D3DRenderer::Get().Update(UpdatePerFrameConstantBuffers(deltaTime), renderWidth, renderHeight, m_appSpecs.multisamples);
	}

	void EngineApp::StartFrame()
	{
		m_Tick.Step();
		renderer::D3DRenderer::Get().StartFrame();

	}

	void EngineApp::EndFrame()
	{
		renderer::D3DRenderer::Get().EndFrame();
		m_window->Present();
		CalculateFrameStatistics();
		InputController::Get().ResetMouseWheel();
	}

	PER_FRAME_CONSTANT_BUFFER EngineApp::UpdatePerFrameConstantBuffers(float deltaTime)
	{
		math::Vector4 resolution =
			math::Vector4(
				static_cast<float>(m_window->GetWindowWidth()),
				static_cast<float>(m_window->GetWindowHeight()),
				1.0f / static_cast<float>(m_window->GetWindowWidth()),
				1.0f / static_cast<float>(m_window->GetWindowHeight())
			);

		MousePos mPos{ InputController::Get().GetMouse().GetMousePos() };
		math::Vector4 mousePos = math::Vector4((float)mPos.xPos, (float)mPos.yPos, 1.0f / mPos.xPos, 1.0f / mPos.yPos);
		float time = m_Tick.TotalActiveTimeInSeconds();

		return { resolution, mousePos, time, deltaTime};
	}

	void EngineApp::CalculateFrameStatistics()
	{
		static uint16_t frameCount{ 0 };
		static float    elapsedTime{ 0.0f };


		++frameCount;

		//Maybe I should use deltatime
		if (m_Tick.TotalActiveTimeInSeconds() - elapsedTime >= 1.0f)
		{
			//we average the frame counts every second
			float FPS = static_cast<float>(frameCount);
			float msPerFrame = 1000.0f / FPS;


			std::wstring strFPS{ std::to_wstring(FPS)};
			std::wstring strMS{ std::to_wstring(msPerFrame) };

			std::wstring strStats{ L"   FPS: " + strFPS + L"     ms/Frame: " + strMS };

			m_window->UpdateWindowText(strStats);

			// Reset for next average.
			frameCount = 0;
			elapsedTime += 1.0f;

		}

	}
}