#pragma once
#include <cstdint>


namespace cyb {

uint64_t GetHPCounter();
uint64_t GetHPFrequency();

class FrameTimer {
public:
	explicit FrameTimer( bool start = false );
	~FrameTimer() = default;

	void Start();
	double Frame();
	double GetFrameTimeMs() const;
	double GetTimeSec() const;

public:
	double m_frequency;
	uint64_t m_offset;
	uint64_t m_lastFrame;
	uint64_t m_frameCount;
	double m_frameTimeMs;
	double m_timeSec;
};

}	// namespace cyb

