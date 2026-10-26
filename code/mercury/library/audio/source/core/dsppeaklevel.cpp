#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::PeakLevel::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	PeakLevel
////////////////////////////////////////////////////////////////////////////////
Audio::Core::DSP::PeakLevel::PeakLevel( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
	m_eType = Core::DSP::Effect::ePeakLevel;

	m_fLevel = 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::PeakLevel::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.PeakLevel.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	F32* aSample = (F32*)pFrameData;

	//  each channel
    for (U32 uCh = 0; uCh < m_uChannels; uCh++ )
    {
		//each sample
        for (U32 uCount = 0; uCount < uFrameLength; uCount++)
        {
            U32 uSample = uCh * uFrameLength + uCount;

			//do peak
            m_fLevel = Maths::Max(m_fLevel, fabsf(aSample[uSample]));
        }
    }
	
    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
