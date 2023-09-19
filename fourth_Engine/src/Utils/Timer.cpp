#include "pch.h"

#pragma once
#include "include/Utils/Timer.h"

namespace fth
{
	Timer::Timer() :
		Timer(0.0)
	{
		
	}

	Timer::Timer(double targetPeriod):
		m_TotalTimePaused(0.0),
		m_Paused(true),
		m_Started(false),
		m_TargetPeriodSec(targetPeriod)
	{

	}

	Timer::~Timer()
	{

	}

	void Timer::Start()
	{
		if (!m_Started)
		{
			std::chrono::steady_clock::time_point  currentTime;
			currentTime = std::chrono::high_resolution_clock::now();

			m_StartTimePoint = currentTime;
			m_LastTimePoint = currentTime;

			m_Started = true;
			m_Paused = false;

		}

	}

	void Timer::Pause()
	{
		if (m_Started && !m_Paused)
		{
			m_PauseTimePoint = std::chrono::high_resolution_clock::now();

			m_Paused = true;

		}
	}

	void fth::Timer::Resume()
	{
		if (m_Started && m_Paused)
		{
			std::chrono::steady_clock::time_point  currentTime;
			
			currentTime = std::chrono::high_resolution_clock::now();

			m_CurrentTimePoint = currentTime;
			m_TotalTimePaused += std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_PauseTimePoint).count();
			m_LastTimePoint = currentTime;

			m_Paused = false;
		}
	}

	float Timer::TotalActiveTimeInSeconds()
	{
		std::chrono::steady_clock::time_point  currentTime;
		currentTime = std::chrono::high_resolution_clock::now();

		uint64_t currentTimeCounts{ static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - m_StartTimePoint).count()) };

		return  static_cast<float>(CountsFromNanosecondsToSeconds(currentTimeCounts));
	}

	

	void Timer::Step()
	{
		std::chrono::steady_clock::time_point  currentTime;
		currentTime = std::chrono::high_resolution_clock::now();
		m_LastTimePoint = currentTime;

	}

	float Timer::CalculateDeltaTimeSec()
	{
	

		if (!m_Started || m_Paused)
		{
			return 0.f;
		}

		namespace t = std::chrono;

		
		auto currentTime = std::chrono::high_resolution_clock::now();

		//return t::duration_cast<t::duration<float, std::milli>>(currentTime - m_LastTimePoint).count();

		return t::duration_cast<t::duration<float>>(currentTime - m_LastTimePoint).count(); // if returns seconds
	}

	bool Timer::IsPeriodCompleted()
	{
		float deltaTime = CalculateDeltaTimeSec();

		if (deltaTime > m_TargetPeriodSec)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	double Timer::CountsFromNanosecondsToMillisecons(uint64_t counts)
	{
		return counts * 1e-6;
	}

	double Timer::CountsFromNanosecondsToSeconds(uint64_t counts)
	{
		return counts * 1e-9;
	}

}


