#pragma once


#define LOG_ENGINE_TRACE(subsystemCaller, ...) ::fth::LogSystem::Get().LogMessage(::fth::LogSystem::LoggerType::Engine, fth::LogSystem::SeverityLevel::Trace, subsystemCaller, __VA_ARGS__)
#define LOG_ENGINE_INFO(subsystemCaller, ...) ::fth::LogSystem::Get().LogMessage(::fth::LogSystem::LoggerType::Engine, fth::LogSystem::SeverityLevel::Info, subsystemCaller, __VA_ARGS__)
#define LOG_ENGINE_WARN(subsystemCaller, ...) ::fth::LogSystem::Get().LogMessage(::fth::LogSystem::LoggerType::Engine, fth::LogSystem::SeverityLevel::Warn, subsystemCaller, __VA_ARGS__)
#define LOG_ENGINE_CRITICAL(subsystemCaller, ...) ::fth::LogSystem::Get().LogMessage(::fth::LogSystem::LoggerType::Engine, fth::LogSystem::SeverityLevel::Critical, subsystemCaller, __VA_ARGS__)
