#include "pch.h"

#pragma once
#include "include/Logging/LogSystem.h"


#include "vendor/spdlog/include/spdlog/sinks/stdout_color_sinks.h"
#include "vendor/spdlog/include/spdlog/sinks/basic_file_sink.h"

#include <filesystem>
#include <time.h>

#define SHOW_CONSOLE !ENGINE_DIST

namespace fth
{

	void LogSystem::AllocateConsole()
	{
		::AllocConsole();
		static FILE* dummy;
		auto s = freopen_s(&dummy, "CONOUT$", "w", stdout);// stdout will print to the newly created console
		auto sin = freopen_s(&dummy, "CONIN$", "r", stdin);
		if (s || sin)
			BREAK_NOLOG;

		std::cout.clear();
		std::cin.clear();

	}



	void LogSystem::Init()
	{
#if SHOW_CONSOLE
		if(!GetConsoleWindow())
		{
			AllocateConsole();
		}
#endif
		const std::string engineLogDirectory{ "logs/engine/" };
		const std::string clientLogDirectory{ "logs/client/" };

		const std::string engineLogFileName{ engineLogDirectory + GetCurrentDateTime() + "ENGINE_LOG.txt"};
		const std::string clientLogFileName{ clientLogDirectory + GetCurrentDateTime() + "CLIENT_LOG.txt"};


		if (!std::filesystem::exists(engineLogDirectory))
			std::filesystem::create_directories(engineLogDirectory);

		if (!std::filesystem::exists(clientLogDirectory))
			std::filesystem::create_directories(clientLogDirectory);

		std::vector<spdlog::sink_ptr> engineSinks;
		std::vector<spdlog::sink_ptr> clientSinks;

#if SHOW_CONSOLE
		engineSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		auto engineSink = static_cast<spdlog::sinks::stdout_color_sink_mt*>(engineSinks.back().get());
		engineSink->set_pattern("[%T][%n][%^%l%$]%v");


		clientSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		auto clientSink = static_cast<spdlog::sinks::stdout_color_sink_mt*>(clientSinks.back().get());
		clientSink->set_pattern("[%T][%n][%^%l%$]%v");
#endif

		engineSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(engineLogFileName, true) );
		engineSinks.back()->set_pattern("[%d-%m-%Y %T.%e][%n][%l]%v");
		clientSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(clientLogFileName, true) );
		clientSinks.back()->set_pattern("[%d-%m-%Y %T.%e][%n][%l]%v");

		m_engineLogger = std::make_shared<spdlog::logger>("fourthEngine", engineSinks.begin(), engineSinks.end());
		m_clientLogger = std::make_shared<spdlog::logger>("Client", clientSinks.begin(), clientSinks.end());

		m_engineLogger->set_level(spdlog::level::trace);
		m_clientLogger->set_level(spdlog::level::trace);

	}

	void LogSystem::FreeConsole()
	{
		::FreeConsole();
	}

	void LogSystem::Shutdown()
	{

		FlushLoggers();
#if SHOW_CONSOLE
		LogSystem::FreeConsole();
#endif
		spdlog::drop_all();

	}

	void LogSystem::FlushLoggers()
	{
		m_engineLogger->flush();
		m_clientLogger->flush();
	}

	std::string LogSystem::GetCurrentDateTime()
	{
		time_t now {time(nullptr)};
		tm     tStruct;
		char   buffer[80];
		errno_t err = localtime_s(&tStruct, &now);
		if (err)
			return "";

		strftime(buffer, sizeof(buffer), "%d-%m-%Y-%H_%M_%S-", &tStruct);

		return buffer;
	}

}
