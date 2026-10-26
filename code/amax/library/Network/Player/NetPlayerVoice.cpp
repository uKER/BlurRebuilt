#include <stdafx.h>
#include <Core/core.h>
#include <network/Core.h>
#include "NetPlayerVoice.h"
#include "../Message/NetMessage.h"
#include "../Misc/NetDebug.h"

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{	
	C_NetPlayerVoice::C_NetPlayerVoice() : m_Initialised(false)
	{
	}

	C_NetPlayerVoice::~C_NetPlayerVoice()
	{
		Shutdown();
	}

	bool C_NetPlayerVoice::Initialise(const U32 requiredSize)
	{
		BIZ_ASSERT(requiredSize > 0);
		if (m_Initialised)
			return true;
		m_Initialised = m_Buffer.Initialise(requiredSize);
		return m_Initialised;
	}

	void C_NetPlayerVoice::Shutdown(void)
	{
		if (!m_Initialised)
			return;
		m_Buffer.Shutdown();
		m_Initialised = false;
		SetAvailable(true);
	}

	void C_NetPlayerVoice::Flush(void)
	{
		if (!m_Initialised)
			return;
		m_Buffer.Flush();
	}

	bool C_NetPlayerVoice::Append(const U8* pBuffer, const U32 bufferSize)
	{
		if (!m_Initialised)
			return false;
		return m_Buffer.Append(pBuffer, bufferSize);
	}

	bool C_NetPlayerVoice::WriteToMessage(C_NetMessage& msg)
	{
		if (!m_Initialised)
			return false;
		if (0 == m_Buffer.Size())
			return false;
		return msg.WriteBuffer(m_Buffer.Data(), m_Buffer.Size());
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
