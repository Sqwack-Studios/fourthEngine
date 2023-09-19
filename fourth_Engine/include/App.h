#pragma once
#include <memory>
#include "include/Utils/Timer.h"
#include "include/Specifications.h"

namespace fth
{
	//Use this if you use engine library without creating an EngineApp. You will have to provide Input feedback to InputController if you want to use it.
	void Init(uint16_t width = 640, uint16_t height = 320, uint16_t multisamples = 4);
	//If you use the engine without EngineApp, and u attach a fth::Win32Window, please, remember to shutdown the window before the engine.
	void Shutdown();
}


namespace fth
{
	class Win32Window;
	struct ParallelExecutor;
	struct EngineAppSpecs;
	struct PER_FRAME_CONSTANT_BUFFER;

	//TODO: Add wrapper functions to Win32Window to separate further separate high level implementations. I dont want to add Win32Window.h to fourthE.h



	class EngineApp
	{
	public:
		EngineApp() = delete;
		EngineApp(const EngineAppSpecs& specs);
		virtual ~EngineApp();

		//Initializes engine singletons. Loads models and textures into memory and gpu
		void Init();
		void Shutdown();
		void AttachWindow(std::unique_ptr<Win32Window>&& window);
		void ToggleLuminanceDebugging();
		void SetLuminanceDebugging(bool debugLuminance);

	private:
		void ShutdownWindow();

	public:
		void Run(EngineApp* IApp);


	public:
	
		//Make any change right before engine starts looping
		virtual void PostInit() = 0;

		virtual void Update(float DeltaTime) = 0;
		virtual void OnRender() = 0;
		virtual void OnShutdown() = 0;


		static inline Win32Window&  GetBaseWindow() { return *m_window.get(); }
		//Called when attached Window gets resized
		virtual void OnResize(uint16_t newWidth, uint16_t newHeight) = 0;

	private:
		void UpdateInternal(float DeltaTime);
		void StartFrame();
		void EndFrame();
		void CalculateFrameStatistics();
		PER_FRAME_CONSTANT_BUFFER UpdatePerFrameConstantBuffers(float deltaTime);


		static std::unique_ptr<Win32Window>            m_window;
		EngineAppSpecs                                 m_appSpecs;
		std::unique_ptr<ParallelExecutor>              m_executor;

		Timer                                          m_Tick;
	};

	std::unique_ptr<fth::EngineApp> CreateApp(const fth::EngineAppSpecs& specs);
}