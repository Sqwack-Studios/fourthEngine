#pragma once
#include <chrono>

namespace fth
{
	class Timer
	{
	public:
		Timer();
		Timer(double targetPeriod);
		~Timer();

		void Start();
		void Pause();
		void Resume();
		void Step(); //Advance last count stamp to present
		float CalculateDeltaTimeSec(); //DeltaTime = (CurrentCountStamp - LastCountStamp) in milisec
		float TotalActiveTimeInSeconds(); 

		bool IsPeriodCompleted();

		inline void SetTargetPeriod(double period) { m_TargetPeriodSec = period; }
		inline const double& GetTargetPeriod() const { return m_TargetPeriodSec; }
		inline bool IsPaused() const { return m_Paused; }

		static double CountsFromNanosecondsToMillisecons(uint64_t counts);
		static double CountsFromNanosecondsToSeconds(uint64_t counts);

	private:

		

		std::chrono::steady_clock::time_point                                  m_StartTimePoint; //measured once, when clock starts to count.
		std::chrono::steady_clock::time_point                                  m_PauseTimePoint; //measured when the clock is paused
		std::chrono::steady_clock::time_point                                  m_CurrentTimePoint; //recently measured time point -> we can erase this member, as we are measuring it when we need it
		std::chrono::steady_clock::time_point                                  m_LastTimePoint;//last measured time point
		double                                                                 m_TotalTimePaused; //sum of the time when the clock was paused
		double                                                                 m_TargetPeriodSec;

		bool                                                                   m_Paused;
		bool                                                                   m_Started;

	};
}


