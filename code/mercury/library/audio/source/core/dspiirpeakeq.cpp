#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::IIRPeakEQ::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	IIRPeakEQ
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::IIRPeakEQ::IIRPeakEQ( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::eIIRPeakEQ;

    m_fPrevCentre = -1.f;
    m_fPrevQ = -1.f;
    m_fPrevGain = -1.f;
    m_fCentre = 440.f;
    m_fQ = 0.f;
    m_fGain = 1.f;
	m_fClamp = Maths::fMax;
    
    for (U32 uCh = 0; uCh < uChannels; uCh++)
    {
        m_fIn1[uCh] = 0.f;
        m_fOut1[uCh] = 0.f;
        m_fIn2[uCh] = 0.f;
        m_fOut2[uCh] = 0.f;
    }
#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::IIRPeakEQ::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.IIRPeakEQ.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	if ((m_fCentre != m_fPrevCentre) || (m_fQ != m_fPrevQ) || (m_fGain != m_fPrevGain))
	{
        F32 fRealQ = powf(2, 8.f * m_fQ);
		F32 fOmega = Maths::f2Pi * m_fCentre * Core::fRcpSampleRate;
		F32 fSin = Maths::Sin(fOmega);
		F32 fCos = Maths::Cos(fOmega);
        F32 fAlpha = fSin / (2.f * fRealQ);
		F32 fTemp0 = fAlpha * m_fGain;
		F32 fTemp1 = fAlpha / m_fGain;
        F32 fA0 = 1.f / (1.f + fTemp1);
        m_fA1 = (-2.f * fCos) * fA0;
        m_fA2 = (1.f - fTemp1) * fA0;
        m_fB0 = (1.f + fTemp0) * fA0;
        m_fB1 = (1.f - fTemp0) * fA0;

        m_fPrevCentre = m_fCentre;
        m_fPrevQ = m_fQ;
        m_fPrevGain = m_fGain;
    }

    F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;

    //  each channel
    for (U32 uCh = 0; uCh < m_uChannels; uCh++)
    {
        F32 fIn1 = m_fIn1[uCh];
        F32 fOut1 = m_fOut1[uCh];
        F32 fIn2 = m_fIn2[uCh];
        F32 fOut2 = m_fOut2[uCh];

        //  each sample
        for (U32 uSample = 0; uSample < uFrameLength; uSample++)
        {
            F32 fIn  = aSample[uCh * uFrameLength + uSample];
            F32 fOut = m_fB0 * fIn + m_fA1 * fIn1 + m_fB1 * fIn2 - m_fA1 * fOut1 - m_fA2 * fOut2;

			fOut = Maths::Clamp(fOut, -m_fClamp, m_fClamp);

            fIn2 = fIn1;
            fOut2 = fOut1;
            fIn1 = fIn;
            fOut1 = fOut;

			aSample[uCh * uFrameLength + uSample] = fOut;
        }

        //  store previous samples
        m_fIn1[uCh] = fIn1;
        m_fOut1[uCh] = fOut1;
        m_fIn2[uCh] = fIn2;
        m_fOut2[uCh] = fOut2;
    }

    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
