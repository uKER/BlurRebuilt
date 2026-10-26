#include <stdafx.h>
#include <Core/core.h>
#include "../Misc/NetDebug.h"
#include "NetTimer.h"

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	C_NetTimer::C_NetTimer() : m_Active(false)
	{
	}

	C_NetTimer::~C_NetTimer()
	{
		m_Active = false;
		m_Timer.reset();
	}

	void C_NetTimer::Start(void)
	{
		Stop();

		m_Timer.start();
		m_Active = true;
	}

	void C_NetTimer::Stop(void)
	{
		m_Timer.reset();
		m_Active = false;
	}

	F32 C_NetTimer::ElapsedTimeInSeconds(void)
	{
		if (!m_Active)
			return 0.0f;
		return m_Timer.getElapsedTimeInSeconds();
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
