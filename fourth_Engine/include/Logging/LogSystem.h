#pragma once
#include "vendor/spdlog/include/spdlog/spdlog.h"
#include "vendor/spdlog/include/spdlog/fmt/ostr.h"



//Main idea is to create a system capable of tracing the code, printing to the console and LOG files.
//There are 4 levels:
//->Trace        :     just to show the loading/process stage
//->Info         :     outputs information, for example, number of adapters, information about displays...
//->Warn         :     outputs information about forced changes in the state of some resource, non-successful execution of functions...
//           the process can keep going but it's not behaving as expected
//->Critical     :    an error has ocurred and the program cannot continue
// 
//Also, my idea is to be able to capture DirectX messages and output them to the console and log files
//

namespace spdlog
{
	class logger;
}

namespace fth
{

	
	class LogSystem
	{
	public:

		enum class SeverityLevel
		{
			Trace = 0,
			Info,
			Warn,
			Critical,
			NUM
		};

		enum class LoggerType
		{
			Engine = 0,
			Client,
			NUM
		};

		LogSystem() = default;
		~LogSystem() = default;
		LogSystem(const LogSystem&) = delete;
		LogSystem& operator=(const LogSystem&) = delete;

	public://Public interface to interact with the singleton

		static inline LogSystem& Get()
		{
			static LogSystem instance;
			return instance;
		};

		void Init();
		void Shutdown();

		static std::string GetCurrentDateTime();

		template<typename... Args>
		static void LogMessage(LoggerType type, SeverityLevel severity, std::string_view subsystemCaller, Args&&... args);

		template<typename... Args>
		static void FormatMsg(Args&&... args);
	
		static std::shared_ptr<spdlog::logger>& GetEngineLogger() { return Get().m_engineLogger; };
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return Get().m_clientLogger; };

		void FlushLoggers();

	private://Actual methods

		static void AllocateConsole();
		static void FreeConsole();

	private:
		std::shared_ptr<spdlog::logger> m_engineLogger;
		std::shared_ptr<spdlog::logger> m_clientLogger;




		//void LogMessageVectorInternal(const std::vector<std::pair<LogSystem::SeverityLevel, std::string>>& vector)
		//{

		//}

	};

	template<typename... Args>
	void LogSystem::FormatMsg(Args&&... args)
	{

	}

	template<typename... Args>
	void LogSystem::LogMessage(LoggerType type, SeverityLevel severity, std::string_view subsystemCaller, Args&&... args)
	{
		auto& logger = (type == LoggerType::Engine) ? LogSystem::Get().GetEngineLogger(): LogSystem::Get().GetClientLogger();

		std::string logFormatString = subsystemCaller.empty() ? "{0}{1}" : "[{0}]{1}";
		switch (severity)
		{
		case SeverityLevel::Trace:
		{
			logger->trace(logFormatString, subsystemCaller, fmt::format(std::forward<Args>(args)...));
			break;
		}
		case SeverityLevel::Info:
		{
			logger->info(logFormatString, subsystemCaller, fmt::format(std::forward<Args>(args)...));
			break;
		}
		case SeverityLevel::Warn:
		{
			logger->warn(logFormatString, subsystemCaller, fmt::format(std::forward<Args>(args)...));
			break;
		}
		case SeverityLevel::Critical:
		{
			logger->critical(logFormatString, subsystemCaller, fmt::format(std::forward<Args>(args)...));

			break;
		}
		}
	}
}