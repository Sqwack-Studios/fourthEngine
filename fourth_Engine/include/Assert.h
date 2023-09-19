#pragma once

namespace fth
{
	std::string TranslateHRESULT(HRESULT hr);
}

static constexpr const char* assertionText = "Assertion failed on expression {0} with description {1}. This occurred in: \nFILE: {2}.\nLINE: {3}.";
static constexpr const char* assertionHRText = "HR Assertion failed with error {0}. This occurred when calling {1}.\nFILE: {2}.\nLINE: {3}.";
static constexpr const char* verifyText = "Verification failed on expression {0} with description {1}. This occurred in: \nFILE: {2}.\nLINE: {3}.";
static constexpr const char* verifyHRText = "HR Verification failed with error {0}. This occurred when calling {1}.\nFILE: {2}.\nLINE: {3}.";


#ifdef  NDEBUG
#else
#define ENABLE_ASSERT
#endif

#define ENABLE_VERIFY
#define BREAK_NOLOG __debugbreak();
#define BREAK fth::LogSystem::Get().FlushLoggers(); __debugbreak(); 
#define BREAK_AND_LOG_CLIENT(subsystemCaller,...) LOG_CLIENT_CRITICAL(subsystemCaller, __VA_ARGS__); BREAK; std::abort()
#define BREAK_AND_LOG_ENGINE(subsystemCaller,...) LOG_ENGINE_CRITICAL(subsystemCaller, __VA_ARGS__); BREAK; std::abort()


#ifdef ENABLE_ASSERT

    #define FTH_ASSERT_ENGINE(expression, msg) { if(!(expression)) { BREAK_AND_LOG_ENGINE("", assertionText, #expression, msg, __FILE__, __LINE__); } }
    #define FTH_ASSERT_CLIENT(expression, msg) { if(!(expression)) { BREAK_AND_LOG_CLIENT("", assertionText, #expression, msg, __FILE__, __LINE__); } }
    #define FTH_ASSERT_HR(call)   { HRESULT _hr = call; if(!SUCCEEDED(_hr)){ BREAK_AND_LOG_ENGINE("", assertionHRText, TranslateHRESULT(_hr), #call, __FILE__, __LINE__); } }
#else

    #define FTH_ASSERT_ENGINE(expression, msg)
    #define FTH_ASSERT_CLIENT(expression, msg)
    #define FTH_ASSERT_HR(call) call

#endif

//Verified even in dist mode == ALWAYS_ASSERT macro
#ifdef ENABLE_VERIFY

     #define FTH_VERIFY_ENGINE(expression, msg) { if(!(expression)) { BREAK_AND_LOG_ENGINE("", verifyText, #expression, msg, __FILE__, __LINE__); } }
     #define FTH_VERIFY_CLIENT(expression, msg) { if(!(expression)) { BREAK_AND_LOG_CLIENT("", verifyText, #expression, msg, __FILE__, __LINE__); } }
     #define FTH_VERIFY_HR(call) { HRESULT _hr = call; if(!SUCCEEDED(_hr)){ BREAK_AND_LOG_ENGINE("", verifyHRText, TranslateHRESULT(_hr), #call, __FILE__, __LINE__); } }

#else 
 
#define FTH_VERIFY_HR(call) call
#define FTH_VERIFY_ENGINE(expression, msg)
#define FTH_VERIFY_CLIENT(expression, msg)

#endif
