#include "FrameworkPch.h"
#include "Framework/Timer/Timer.h"

#if defined(PLATFORM_PC)
#include <mmsystem.h>	// [Blur2PC] timeBeginPeriod for the frame limiter
#pragma comment(lib, "winmm.lib")
#endif

// [Blur2PC] Optional cap on rendered frames per second (0 = uncapped, the default). The game
// simulation always runs at 60 Hz; the PC loop renders in between with interpolation (see
// C_Application::IdleLoop). BLUR2PC_FRAMELIMIT=<fps> sets it at startup.
Debug::Int g_debugBlur2PCFrameLimit("Framework.Timer.FrameLimit", 0, Debug::Range(0, 1000));

// [Blur2PC] waits until 1/limit s have passed since the previous call; called once per rendered frame
void Blur2PCFrameLimitWait()
{
	static bool s_envRead = false;
	if (!s_envRead)
	{
		s_envRead = true;
		if (const char* e = getenv("BLUR2PC_FRAMELIMIT"))
			g_debugBlur2PCFrameLimit = atoi(e);
	}
	static Time::C_Time s_prev = 0;
	if (g_debugBlur2PCFrameLimit > 0 && s_prev)
	{
#if defined(PLATFORM_PC)
		static bool s_timerPeriodSet = false;
		if (!s_timerPeriodSet) { timeBeginPeriod(1); s_timerPeriodSet = true; }
#endif
		const F32 target = 1.0f / (F32)(S32)g_debugBlur2PCFrameLimit;
		for (;;)
		{
			const F32 elapsed = (F32)(Time::GetPerformanceCounter() - s_prev) / (F32)Time::GetPerformanceFrequency();
			if (elapsed >= target)
				break;
#if defined(PLATFORM_PC)
			if (target - elapsed > 0.002f)
				Sleep(1);
#endif
		}
	}
	s_prev = Time::GetPerformanceCounter();
}

namespace Framework
{

//#pragma optimize("", off)

C_Timer::C_Timer() :
	m_Paused(false),
	m_frameAveragingCount(0.0f),
	m_FrameCycle(0.0f),
	m_FrameTime(0.0f),
	m_FrameRate(0.0f),
	m_ElapsedTime(0.0f),
	m_ForceFrameTime(-1.0f),
	m_TimerScale(1.0f)
{
	m_PrevTime = Time::GetPerformanceCounter();
}

C_Timer::~C_Timer()
{
}

void C_Timer::Update()
{
	const Time::C_Time currTime = Time::GetPerformanceCounter();
	m_FrameTime = (F32)(currTime - m_PrevTime) / (F32)Time::GetPerformanceFrequency();
	
	m_PrevTime = currTime;
	
	// update frame rate 4 times per second
	m_FrameCycle += m_FrameTime;
	m_frameAveragingCount += 1.f;
	if(m_FrameCycle > 0.25f)
	{
		m_FrameRate = (F32)m_frameAveragingCount / m_FrameCycle;
		m_frameAveragingCount = 0.f;
		m_FrameCycle = 0.f;
	}

	// [Blur2PC] the floor was 1/60 s: fine on consoles locked at <= 60 Hz, but on PC every frame above
	// 60 fps counted as a full 1/60 s step and the game ran fps/60 times too fast. C_GameWorld::CalcNumberOfSteps
	// accumulates fractional time into fixed 1/60 steps, so short frames are safe there. Remaining
	// frame-counted consumers are tracked by T050.
	m_FrameTime = Maths::Clamped(m_FrameTime, 1.0f / 1000.0f, 1.f / 15.f);
	{
		// [Blur2PC] frame-rate report every ~5 s (the MiniPerfMonitor FPS reads the profiler, compiled out in Speed)
		static F32 s_reportTimer = 0.f;
		s_reportTimer += m_FrameTime;
		if (s_reportTimer > 5.f)
		{
			s_reportTimer = 0.f;
			LOGINFO(Debug::GetLogger("blur2pc.timer"), "frame rate %.1f fps, frame time %.2f ms\n", m_FrameRate, m_FrameTime * 1000.f);
		}
	}

	// If we are forcing the frame time to be a specific value, use that value:
	if (m_ForceFrameTime >= 0.f)
	{
		m_FrameTime = m_ForceFrameTime;
	}

	// increment total elapsed time
	if(!m_Paused)
	{
		m_ElapsedTime += m_FrameTime;
	}
}

void C_Timer::SetTimerScale(F32 scale)
{
	m_TimerScale = Maths::Max(scale, 0.0f);
}

}; // namespace Framework
