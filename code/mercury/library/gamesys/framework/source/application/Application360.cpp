#include "FrameworkPch.h"
#include "Framework/Application/Application360.h"
#include "Framework/Application/TimingColours.h"
#include "Framework/Application/PerformanceLogging.h"
#include <System/Serialise.h>
#include <System/perf/ProfilerBackend.h>

#include "lua_shared/LuaHelper.h"
#include "UI/Game/UILua.h"

// If the frame takes less than 30ms, run the Lua garbage collector for 1ms.
#define LUA_AUTOMATIC_GARBAGE_COLLECT_THRESHOLD_USEC	30000
#define LUA_AUTOMATIC_GARBAGE_COLLECT_USEC				1000

bool C_GameApp_IsBootLayerActive();

namespace Framework
{

C_Application* C_Application::s_pInstance = NULL;

C_Application::C_Application()
	: m_LoadingThread(this), m_cachedLoadingScreenEnabled(false)
{
	assert(s_pInstance == NULL);
	s_pInstance = this;
}

C_Application::~C_Application()
{
	assert(s_pInstance == this);
	s_pInstance = NULL;
}

HRESULT C_Application::Run()
{
	m_LoadingThread.Start(0,0,"LoadingScreenThread");

	Display::Device()->ReleaseThreadOwnership();

	while(m_RunState == E_RunState_Run)
	{
		{
			CPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "CPU");

			m_startFrameTime = Time::GetPerformanceCounter();

			PerformanceLogging::UpdateCounters(PerformanceLogging::kGameBeginFrame);

			m_cachedLoadingScreenEnabled = m_LoadingScreenEnabled;
			m_LoadingThread.SyncLoadingScreen(m_cachedLoadingScreenEnabled);

			CHECK_MEMORY_INTEGRITY();
			CHECK_FOR_GRAPHICS_MEMORY_STOMPING();

			{
				CPU_TIMING(1, g_TimingColour_Update, "CPU.Update");

				OnUpdate();
			}

			PerformanceLogging::UpdateCounters(PerformanceLogging::kGameUpdateFinished);

			if(!m_cachedLoadingScreenEnabled)
			{
				while(!Display::Device()->AcquireThreadOwnership(2 * 1000))
				{
					Sleep(30);
				}

#ifdef CFG_DEBUG
				//_ASSERTE(_CrtCheckMemory());
#endif

				{
					GPU_TIMING_BEGIN_FRAME();

					{
						GPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "GPU");

						{
							CPU_TIMING(1, g_TimingColour_Render, "CPU.Render");

							//GPU_TIMING(1, g_TimingColour_Render, "GPU.Render"); // Moved to CommonRender in ApplicationBase

							OnRender();
						}
					}

					GPU_TIMING_END_FRAME();
				}

				Display::Device()->ReleaseThreadOwnership();
			}

			PerformanceLogging::UpdateCounters(PerformanceLogging::kGameEndFrame);
		}
		Perf::ProfilerCommon::Instance().EndFrame();

		GPU_TIMING_UPDATE();

		#ifdef USE_CPU_TIMINGS
		Perf::C_CPUTimingStats::UpdateStats();
		#endif
	}
	
	m_LoadingThread.SendQuitMessage();
	Display::Device()->AcquireThreadOwnership();

	return S_OK;
}

void C_Application::PrePresent()
{
	// Don't run Lua GC if the loading screen has been enabled or disabled on the same frame, or the boot sequence is playing
	if ((!m_cachedLoadingScreenEnabled) && (!m_LoadingScreenEnabled) && (!C_GameApp_IsBootLayerActive()))
	{
		Time::C_Time total = Time::GetPerformanceCounter() - m_startFrameTime;

		if (total < LUA_AUTOMATIC_GARBAGE_COLLECT_THRESHOLD_USEC)
		{
			Lua::TimedGarbageCollect( UI::g_pUILua->GetState(), LUA_AUTOMATIC_GARBAGE_COLLECT_USEC );
		}
	}
}

void C_Application::ProcessLoadingScreen()
{	
	{
		{
			CPU_TIMING(4, g_TimingColour_Update, "CPU.UpdateLoadingScreen");

			OnLoadingScreenUpdate();
		}

		PerformanceLogging::UpdateCounters(PerformanceLogging::kGameUpdateFinished);
	}
	
	{
		while(!Display::Device()->AcquireThreadOwnership(2 * 1000))
		{
			Sleep(30);
		}

#ifdef CFG_DEBUG
		//_ASSERTE(_CrtCheckMemory());
#endif

		{
			GPU_TIMING_BEGIN_FRAME();

			{
				GPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "GPURenderLoadingScreen");

				{
					CPU_TIMING(4, g_TimingColour_Render, "CPU.RenderLoadingScreen");

					OnLoadingScreenRender();
				}
			}

			GPU_TIMING_END_FRAME();
		}

		Display::Device()->ReleaseThreadOwnership();
	}	
}

bool C_Application::LoadingScreenEnabledState() const
{
	return m_LoadingThread.IsLoadingScreenActive();
}


}; // namespace Framework
