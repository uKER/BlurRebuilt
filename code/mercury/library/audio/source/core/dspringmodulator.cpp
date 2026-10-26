#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::RingModulator::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	RingModulator
////////////////////////////////////////////////////////////////////////////////
Audio::Core::DSP::RingModulator::RingModulator( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
	m_eType = Core::DSP::Effect::eRingModulator;

	Assert( m_uChannels <= sMaxChannels );

	m_fAngle = 0.f;

    m_fDelta = 0.f;
    m_fWet = 1.f;
    m_fDry = 1.f;

	m_bSync = true;
	m_bPhaseLocked = false;
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::RingModulator::Process( void* pFrameData, const ProcessInfo& info, bool bProcess )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.RingModulator.Process");

	if( !m_bActive || bDisableAll )
		return true;

	if( m_bSync )
	{
		F32 fChannelPhase = m_bPhaseLocked ? 0.f : Maths::f2Pi / m_uChannels;

		// get initial phases
		for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
		{
			F32 fCh = (F32)uCh;
			m_afSin[ uCh ] = Maths::Sin( fCh * fChannelPhase );
			m_afCos[ uCh ] = Maths::Cos( fCh * fChannelPhase );
		}

		m_bSync = false;
	}

	// get input buffer
	F32* pfFrameBuffer = (F32*)pFrameData;

	// for each channel
	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
		F32* pfChannelBuffer = &pfFrameBuffer[ uCh * uFrameLength ];

		F32 fSin = m_afSin[ uCh ];
		F32 fCos = m_afCos[ uCh ];

		// for each sample
		for( U32 uSample = 0; uSample < uFrameLength; uSample++ )
		{
			fSin -= m_fDelta * fCos;
			fCos += m_fDelta * fSin;
			pfChannelBuffer[ uSample ] *= m_fDry + m_fWet * fSin;
		}

		m_afSin[ uCh ] = fSin;
		m_afCos[ uCh ] = fCos;
	}

#ifndef CFG_GOLD
	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
		if( !Frame::IsValid( &pfFrameBuffer[ uCh * Core::uFrameLength ] ) )
			AUDIO_CORE_ERROR("Bad samples in ring modulator effect");
	}
#endif

	return true;
}
