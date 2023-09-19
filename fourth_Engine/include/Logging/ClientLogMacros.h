#pragma once


#define LOG_CLIENT_TRACE(subsystemCaller, ...) ::fth::LogSystem::Get().LogMessage(::fth::LogSystem::LoggerType::Client, fth::LogSystem::SeverityLevel::Trace, subsystemCaller, __VA_ARGS__)
#define LOG_CLIENT_INFO(subsystemCaller, ...) ::fth::LogSystem::Get().LogMessage(::fth::LogSystem::LoggerType::Client, fth::LogSystem::SeverityLevel::Info, subsystemCaller, __VA_ARGS__)
#define LOG_CLIENT_WARN(subsystemCaller, ...) ::fth::LogSystem::Get().LogMessage(::fth::LogSystem::LoggerType::Client, fth::LogSystem::SeverityLevel::Warn, subsystemCaller, __VA_ARGS__)
#define LOG_CLIENT_CRITICAL(subsystemCaller, ...) ::fth::LogSystem::Get().LogMessage(::fth::LogSystem::LoggerType::Client, fth::LogSystem::SeverityLevel::Critical, subsystemCaller, __VA_ARGS__)