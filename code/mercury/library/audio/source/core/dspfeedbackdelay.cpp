#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::FeedbackDelay::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	FeedbackDelay
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::FeedbackDelay::FeedbackDelay( const char* pchInfo, U16 uChannels, U32 u32NumFrames ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::eFeedbackDelay;

    m_uBuffer = 0;

    m_fFeedback = 0.f;
    m_fWet = 1.f;
    m_fDry = 1.f;

	m_uLength = 1;

	m_uCurr = 0;

	m_u32NumFrames = u32NumFrames;

	m_uPrevFrameNum = (U32)-1;
#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::FeedbackDelay::Process( void* pFrameData, const ProcessInfo& info, bool bProcess )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.FeedbackDelay.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	F32* pDelayBuffers = &info.pStaticBuffers[ m_uBuffer ];

	F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;

	//	if delay length changed or effect was disabled for 1 or more frames
	if( ( m_uLength != m_uPrevLength ) || ( info.uFrameNum != m_uPrevFrameNum + 1 ) )
	{
		//	clear buffer
		memset( pDelayBuffers, 0, m_uChannels * ( m_u32NumFrames + 1 ) * uFrameLength * sizeof(F32) );
		m_uPrevLength = m_uLength;
	}

	//	store prev frame num for above compare
	m_uPrevFrameNum = info.uFrameNum;

	//	inc write cursor
	U32 uWrite = m_uCurr + m_uLength;

	//	deal with wrap around
    if( uWrite >= m_u32NumFrames )
        uWrite -= m_u32NumFrames;

	//  for each channel
	for( U32 uChannel = 0; uChannel < m_uChannels; uChannel++ )
	{
		F32* pChBuffer = &pDelayBuffers[uChannel * (m_u32NumFrames + 1) * uFrameLength];

		//	copy frame into delay buffer at feedback level, or zero
		if ( m_fFeedback == 1.f )
			Frame::Copy( &pChBuffer[ uWrite * uFrameLength ], &aSample[ uChannel * uFrameLength ] );
		else if ( m_fFeedback != 0.f )
			Frame::Gain( &pChBuffer[ uWrite * uFrameLength ], &aSample[ uChannel * uFrameLength ], m_fFeedback );
		else
			Frame::Zero( &pChBuffer[ uWrite * uFrameLength ] );

		//	mix delay buffer into delay buffer at feedback level
		if ( m_fFeedback == 1.f )
			Frame::DoAdd( &pChBuffer[ uWrite * uFrameLength ], &pChBuffer[ uWrite * uFrameLength ], &pChBuffer[ m_uCurr * uFrameLength ] );
		else if ( m_fFeedback != 0.f )
			Frame::AddGain( &pChBuffer[ uWrite * uFrameLength ], &pChBuffer[ uWrite * uFrameLength ], &pChBuffer[ m_uCurr * uFrameLength ], m_fFeedback );

		//	apply dry gain to frame
		if ( m_fDry == 0.f )
			Frame::Zero( &aSample[ uChannel * uFrameLength ] );
		else if ( m_fDry != 1.f )
			Frame::Gain( &aSample[ uChannel * uFrameLength ], &aSample[ uChannel * uFrameLength ], m_fDry );

		//	mix delay buffer into frame
		if ( m_fWet == 1.f )
			Frame::DoAdd( &aSample[ uChannel * uFrameLength ], &aSample[ uChannel * uFrameLength ], &pChBuffer[ m_uCurr * uFrameLength ] );
		else
			Frame::AddGain( &aSample[ uChannel * uFrameLength ], &aSample[ uChannel * uFrameLength ], &pChBuffer[ m_uCurr * uFrameLength ], m_fWet );
	}

    //  inc read offset
    if( ++m_uCurr >= m_u32NumFrames )
        m_uCurr = 0;

    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
