#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Gain::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	Gain
////////////////////////////////////////////////////////////////////////////////
Audio::Core::DSP::Gain::Gain( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::eGain;

    m_fPrevGain = 0.f;
	m_fGain = 1.f;
#endif	//	#ifndef AUDIO_DISABLE_DSP
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Gain::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.Gain.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

    F32* aSample = (F32*)pFrameData;

    if (m_fGain == m_fPrevGain)
    {
		//  each channel
		for (U32 uChannel = 0; uChannel < m_uChannels; uChannel++)
		{
            //  same as last frame, just set gain
            for (U32 uSample = 0; uSample < m_uChannels * uFrameLength; uSample++)
                aSample[uSample] *= m_fGain;
        }
    }
    else
    {
		//  each channel
		for (U32 uChannel = 0; uChannel < m_uChannels; uChannel++)
		{
            //  gain changed, ramp it
            F32 fGainStep = (m_fGain - m_fPrevGain) * fRcpFrameLength;

            for (U32 uSample = 0; uSample < uFrameLength; uSample++)
                aSample[uChannel * uFrameLength + uSample] *= m_fPrevGain + ((F32)uSample * fGainStep);

            m_fPrevGain = m_fGain;
        }
    }

    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
