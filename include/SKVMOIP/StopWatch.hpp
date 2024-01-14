#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/debug.h>
#include <chrono>

namespace SKVMOIP
{
	class StopWatch
	{
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
		const char* m_desc;
		bool m_isStarted;

		void logTimeElapsed() noexcept
		{
			DEBUG_LOG_INFO("%s %lu ms", m_desc,
				std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_start).count());
		}

	public:
		StopWatch(const char* desc = "time elapsed: ") noexcept: m_desc(desc), m_isStarted(true)
		{
			m_start = std::chrono::high_resolution_clock::now();
		}

		~StopWatch() noexcept
		{
			if(m_isStarted)
			{
				logTimeElapsed();
				m_isStarted = false;
			}
		}

		void stop() noexcept
		{
			if(m_isStarted)
			{
				logTimeElapsed();
				m_isStarted = false;
			}
		}

		void start() noexcept
		{
			m_isStarted = true;
			m_start = std::chrono::high_resolution_clock::now();
		}
	};
}