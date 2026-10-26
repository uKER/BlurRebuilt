#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Send::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	Send
////////////////////////////////////////////////////////////////////////////////
Audio::Core::DSP::Send::Send( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::eSend;

    m_fPrevLevel = 0.f;
	m_fLevel = 1.f;
#endif	//	#ifndef AUDIO_DISABLE_DSP
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Send::Process( void* pFrameData, const ProcessInfo& info, bool bProcess )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.Send.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	F32* aSample = (F32*)pFrameData;

	bool bActive = false;

	Return* pReturn = ( Return* )&info.pCoreData[ m_uReturn ];

	if( m_fLevel > 0.f)
	{
		for( U32 uCh = 0; uCh < ( U32 )m_uChannels; uCh++ )
		{
			//	TODO: add gain ramping
			F32* pReturnBuffer = pReturn->OutputBuffer( info ) + uCh * uFrameLength;
			Frame::Gain( pReturnBuffer, aSample + uCh * uFrameLength, m_fLevel );
			bActive = true;
		}
	}

	pReturn->SetActive( bActive );

    return true;

#else // AUDIO_DISABLE_DSP
      return false;
#endif // AUDIO_DISABLE_DSP
}


