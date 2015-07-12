#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "timer.h"

namespace cyb {

uint64_t GetHPCounter() {
	uint64_t counter = 0;

#if _WIN32
	LARGE_INTEGER li;
	QueryPerformanceCounter( &li );
	counter = li.QuadPart;
#else
	struct timeval now;
	gettimeofday( &now, 0 );
	int64_t i64 = now.tv_sec*INT64_C( 1000000 ) + now.tv_usec;
#endif

	return counter;
}

uint64_t GetHPFrequency() {
#if _WIN32
	LARGE_INTEGER li;
	QueryPerformanceFrequency( &li );
	return li.QuadPart;
#else
	return UINT64_C( 1000000 );
#endif
}

FrameTimer::FrameTimer( bool start ) {
	m_offset = 0;
	m_frequency = static_cast<double>( GetHPFrequency() );
	m_frameTimeMs = 0.0;
	m_timeSec = 0.0;

	if ( start ) {
		Start();
	}
}

void FrameTimer::Start() {
	m_offset = GetHPCounter();
	m_lastFrame = m_offset;
}

double FrameTimer::Frame() {
	if ( !m_offset ) {
		Start();
	}

	const int64_t now = cyb::GetHPCounter();
	const int64_t frameTime = now - m_lastFrame;

	m_frameTimeMs = static_cast<double>( now - m_lastFrame ) / m_frequency * 1000.0;
	m_timeSec     = static_cast<double>( now - m_offset ) / m_frequency;
	m_lastFrame   = now;

	return m_timeSec;
}

double FrameTimer::GetTimeSec() const {
	return m_timeSec;
}

}	// namespace cyb