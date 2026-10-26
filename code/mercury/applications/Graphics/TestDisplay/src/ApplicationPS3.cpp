#include "StdAfx.h"
#include "ApplicationPS3.h"

C_ApplicationParams::C_ApplicationParams()
{
}

C_Application::C_Application()
{
	m_Quit = false;
}

C_Application::~C_Application()
{
}

bool C_Application::Create(C_ApplicationParams& params)
{
	m_DeviceParams.m_BackBufferWidth		= params.m_BackBufferWidth;
	m_DeviceParams.m_BackBufferHeight		= params.m_BackBufferHeight;
	m_DeviceParams.m_CmdBufMemorySize		= 1024*256;
	m_DeviceParams.m_SystemMemorySize		= 1024*1024*45;
	m_DeviceParams.m_MainDeviceMemorySize	= 1024*1024*5;
	m_DeviceParams.m_LocalDeviceMemorySize	= 1024*1024*50;

	if(!C_ApplicationBase::Create())
		return false;

	m_pJoypad = new C_Joypad(0, &m_InputManager);

	Display::RingBuffer()->Initialise(1024*1024*4, 1024*1024*4, 1024*32);

	return true;
}

void C_Application::Destroy()
{
	SAFE_DELETE_PTR(m_pJoypad);

	C_ApplicationBase::Destroy();
}

void C_Application::Update()
{
	m_pJoypad->Update();

	m_Camera.Update(m_pJoypad, m_pJoypad, m_Timer.m_FrameTime);

	C_ApplicationBase::Update();
}

bool C_Application::Run()
{
	while(!m_Quit)
	{
		{
			CPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "CPU");
			
			{
				CPU_TIMING(1, Maths::RGBA(255, 255, 255, 255), "CPU.Update");

				Update();
			}

			{
				GPU_TIMING_BEGIN_FRAME();

				{
					GPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "GPU");

					{
						CPU_TIMING(1, Maths::RGBA(255, 255, 255, 255), "CPU.Render");

						GPU_TIMING(1, Maths::RGBA(255, 255, 255, 255), "Render");

						Render();

						Display::RingBuffer()->EndFrame();
					}
				}

				GPU_TIMING_END_FRAME();
			}
		}

		#ifdef USE_CPU_TIMINGS
		Perf::C_CPUTimingStats::UpdateStats();
		#endif

		#ifdef USE_GPU_TIMINGS
		Display::C_GPUTimingStats::UpdateStats();
		#endif

		#ifdef USE_GPU_TIMINGS
		Display2::C_TimingManager::GetInstance().Update();
		#endif
	}

	return true;
}
