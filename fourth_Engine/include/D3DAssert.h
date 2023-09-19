#pragma once
#include "include/Assert.h"
#include "include/Utils/DxRes.h"


struct IDXGIInfoQueue;
namespace fth
{

	class DxDebugParser
	{
	public:
		DxDebugParser();
		~DxDebugParser();
		DxDebugParser(const DxDebugParser&) = delete;
		DxDebugParser& operator=(const DxDebugParser&) = delete;

		static void Init() { Get().InitInternal(); };
		static void Shutdown() { Get().ShutdownInternal(); };
		static inline DxDebugParser& Get()
		{
			static DxDebugParser instance;
			return instance;
		};

		//Returns true if any message is critical to abort the program
		static bool QueryMessagesWithSeverity() { return Get().QueryMessagesWithSeverityInternal(); };

		static const std::vector<std::pair<fth::LogSystem::SeverityLevel, std::string>>& GetMessages() { return Get().m_messages; }
	private:
		void InitInternal();
		void ShutdownInternal();
		bool QueryMessagesWithSeverityInternal();

	private:
		DxResPtr<IDXGIInfoQueue>  m_dxgiInfoQueue;
		std::vector<std::pair<fth::LogSystem::SeverityLevel, std::string>> m_messages;
	};

}



//Seems like DXGI error and corruption also informs of some object creation parameters. Those are described using curly braces,
//which destroys fmt library format and causes an exeception. Right now I dont know how to work around it, so I just make 3 logs for a 
//DXGI critical error, directly flushing the message through the logger without formatting.
// Example: DXGI ERROR: IDXGIFactory::CreateSwapChain: Flip model swapchains (DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL and DXGI_SWAP_EFFECT_FLIP_DISCARD) require BufferCount to be between 2..... 
//DXGI_SWAP_CHAIN_DESC{ SwapChainType = ..._HWND, 
//BufferDesc = DXGI_MODE_DESC1{Width = 0, Height = 0, 
//RefreshRate = DXGI_RATIONAL{ Numerator = 0, Denominator = 1 }, 
//Format = R8G8B8A8_UNORM, ScanlineOrdering = ..._UNSPECIFIED, Scaling = ..._UNSPECIFIED, Stereo = FALSE }, 
//SampleDesc = DXGI_SAMPLE_DESC{ Count = 1, Quality = 0 }, BufferUsage = 0x60, BufferCount = 1,...
// It might be possible to dissable this extra information?
//

#define DEBUG_DX(call){\
bool shouldAbort = fth::DxDebugParser::QueryMessagesWithSeverity(); \
auto msgs = fth::DxDebugParser::GetMessages(); \
for (auto msg : msgs)\
{\
if (msg.first == fth::LogSystem::SeverityLevel::Critical)\
{\
LOG_ENGINE_CRITICAL(#call, ""); \
/*fth::LogSystem::GetEngineLogger()->critical(msg.second);*/\
LOG_ENGINE_CRITICAL("", "\n FILE: {0}\nLINE: {1}", __FILE__, __LINE__);\
continue; \
}\
fth::LogSystem::Get().LogMessage(fth::LogSystem::LoggerType::Engine, msg.first, #call, msg.second); \
}\
if (shouldAbort)\
{ \
BREAK \
} \
}\




#ifdef NDEBUG

    #define DX_HRCALL(call) FTH_VERIFY_HR(call)
    #define DX_CALL(call) call

#else

#define DX_HRCALL(call) {HRESULT hr = call; if(!SUCCEEDED(hr)){LOG_ENGINE_CRITICAL("", verifyHRText, TranslateHRESULT(hr), #call, __FILE__, __LINE__); }; LogSystem::Get().FlushLoggers(); DEBUG_DX(call)}
#define DX_CALL(call) {call; DEBUG_DX(call)}
 
#endif


