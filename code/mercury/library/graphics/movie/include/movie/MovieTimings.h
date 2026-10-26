#ifndef MOVIE_MOVIE_TIMINGS_H
#define MOVIE_MOVIE_TIMINGS_H

#ifdef _MSC_VER
#pragma once
#endif

#if !CFG_GOLD && 0

namespace Movie
{

extern Debug::Logger& g_MovieLog;

class C_MovieTiming
{
public:

	C_MovieTiming(const char* name, const F32 threshold, const U32 frameIndex, const U32 frameCount)
	{
		strcpy_s(m_Name, sizeof(m_Name), name);
		m_OldTime = Time::GetCurrentTime();
		m_Threshold = threshold;
		m_FrameIndex = frameIndex;
		m_FrameCount = frameCount;
	}

	~C_MovieTiming()
	{
		Time::C_Time newTime = Time::GetCurrentTime();
		F32 time = (F32)(newTime - m_OldTime) / 1000.0f;
		if(time >= m_Threshold)
		{
			LOGINFO(g_MovieLog, "MOVIE: %.2f %s (%i / %i)\n", time, m_Name, m_FrameIndex, m_FrameCount);
		}
	}

private:

	char			m_Name[128];
	Time::C_Time	m_OldTime;
	F32				m_Threshold;
	U32				m_FrameIndex;
	U32				m_FrameCount;
};

}; // namespace Movie

#define MOVIE_TIMING(_name, _threshold, _frameIndex, _frameCount) Movie::C_MovieTiming movieTiming(_name, _threshold, _frameIndex, _frameCount)
#define MOVIE_DEFAULT_THRESHOLD 20.0f

#else

#define MOVIE_TIMING(_name, _threshold, _frameIndex, _frameCount)
#define MOVIE_DEFAULT_THRESHOLD

#endif

#endif // MOVIE_MOVIE_TIMINGS_H
