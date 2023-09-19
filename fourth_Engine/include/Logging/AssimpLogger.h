#pragma once

#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>


namespace fth
{
	struct AssimpLogger : public Assimp::Logger
	{
		AssimpLogger()
		{
		}

		virtual ~AssimpLogger()
		{
		}

		/**@brief Sets default AssimpLogger to this custom logger, which captures AssimpLogs into LogSystem stream. Needs to be cleared
		* by calling AssimpLogger::Clear()
		*/
		static void Create()
		{
			Assimp::DefaultLogger::set(new AssimpLogger());
		}


		static void Clear()
		{
			Assimp::DefaultLogger::set(nullptr);
		}


		virtual void OnVerboseDebug(const char* msg) override
		{
			LOG_ENGINE_TRACE("Assimp", msg);
		}

		virtual void OnInfo(const char* msg) override
		{
			LOG_ENGINE_INFO("Assimp", msg);
		}

		virtual void OnWarn(const char* msg) override
		{
			LOG_ENGINE_WARN("Assimp", msg);
		}

		virtual void OnError(const char* msg) override
		{
			LOG_ENGINE_CRITICAL("Assimp", msg);
		}

		virtual bool attachStream(Assimp::LogStream* pStream,
			unsigned int severity = Debugging | Err | Warn | Info) {
			return false;
		}
		virtual bool detachStream(Assimp::LogStream* pStream,
			unsigned int severity = Debugging | Err | Warn | Info) {
			return false;
		}
		virtual void OnDebug(const char* msg) {}


	};

}