#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Oscilloscope::bDisableAll = false;
F32 Audio::Core::DSP::Oscilloscope::fMinHigh = 0.f;
F32 Audio::Core::DSP::Oscilloscope::fMaxLow = 0.f;

////////////////////////////////////////////////////////////////////////////////
//	RMSLevel
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::Oscilloscope::Oscilloscope( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::eOscilloscope;

	m_bReset = true;

	m_fLevel = 1.f;
	m_uLength = 256;

	m_sCurr = 0;
	m_uCount = m_uLength;
#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Oscilloscope::Process( void* pFrameData, const ProcessInfo& info, bool bProcess )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.Oscilloscope.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;

	if( m_bReset )
	{
		//	zero buffer
		memset( m_afHigh, 0, AUDIO_OSC_STEP_SIZE * sizeof(F32) );
		memset( m_afLow, 0, AUDIO_OSC_STEP_SIZE * sizeof(F32) );

		m_bReset = false;
	}

	S32 sCurr = 0;
	U32 uCount = 0;

	// each channel
	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
		sCurr = m_sCurr;
		uCount = m_uCount;

		F32 fHigh = m_afHigh[ sCurr ];
		F32 fLow = m_afLow[ sCurr ];

		//	each sample
		for( U32 uSample = 0; uSample < uFrameLength; uSample++ )
		{
			fHigh = Maths::Max( fHigh, aSample [ uSample ] );
			fLow = Maths::Min( fLow, aSample [ uSample ] );

			if( --uCount == 0 )
			{
				uCount = m_uLength;

				//	commit these values
				m_afHigh[ sCurr ] = Maths::Max( fHigh, m_afHigh[ sCurr ] );
				m_afLow[ sCurr ] = Maths::Min( fLow, m_afLow[ sCurr ] );

				//	move to next bin
				if( ++sCurr == AUDIO_OSC_STEP_SIZE )
					sCurr = 0;

				if( uCh == 0)
				{
					//	reset next bin
					m_afHigh[ sCurr ] = fMinHigh;
					m_afLow[ sCurr ] = fMaxLow;
				}

				//	reset accumulators
				fHigh = fMinHigh;
				fLow = fMaxLow;
			}
		}

		//	update values for this channel
		m_afHigh[ sCurr ] = fHigh;
		m_afLow[ sCurr ] = fLow;
	}

	m_sCurr = sCurr;
	m_uCount = uCount;

	return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
