#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::IIRLowPass2ndOrder::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	IIRLowPass2ndOrder
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::IIRLowPass2ndOrder::IIRLowPass2ndOrder( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::eIIRLowPass2ndOrder;

	m_fTargetCutoff = fSampleRate * 0.5f;
    m_fPrevCutoff = -1.f;
    m_fPrevQ = -1.f;
    m_fPrevGain = -1.f;
    m_fCutoff = fSampleRate * 0.5f;
    m_fQ = 0.f;
    m_fGain = 1.f;
	m_fClamp = Maths::fMax;

    for (U32 uCh = 0; uCh < uChannels; uCh++)
    {
        m_fPrevIn0[uCh] = 0.f;
        m_fPrevOut0[uCh] = 0.f;
        m_fPrevIn1[uCh] = 0.f;
        m_fPrevOut1[uCh] = 0.f;
    }

#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::IIRLowPass2ndOrder::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.IIRLowPass2ndOrder.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	//	limit freq sweep rate using Q
	F32 fCutoffDelta = (m_fTargetCutoff - m_fCutoff);
	F32 fThresh = Maths::RcpEst(Maths::Power(2.f, 3.f * m_fQ));	//	Q=0 -> 1oct, Q=1 -> 1/8oct per 256 samp frame
	F32 fStep = fThresh * m_fCutoff * fFrameLength * 0.001953125f; // 1/512
	fCutoffDelta = Maths::Clamp(fCutoffDelta, -fStep, fStep);
	m_fCutoff += fCutoffDelta;

    if ((m_fCutoff != m_fPrevCutoff) || (m_fQ != m_fPrevQ) || (m_fGain != m_fPrevGain))
    {
		F64 fRealQ = (F64)Maths::Power(2.f, 8.f * m_fQ);
		F32 fOmega = Maths::fPi * m_fCutoff * Core::fRcpSampleRate;
		F64 fSin = (F64)Maths::Sin(fOmega);
		F64 fCos = (F64)Maths::Cos(fOmega);
		F64 fAlpha = fSin / (2.0 * fRealQ);
		F64 fA0 = 1.0 / (1.0 + fAlpha);
        m_fA1 = (F32)((-2.0 * fCos) * fA0);
        m_fA2 = (F32)((1.0 - fAlpha) * fA0);
        m_fB1 = (F32)((1.0 - fCos) * fA0) * m_fGain;
        m_fB0 = m_fB1 * 0.5f;

        m_fPrevCutoff = m_fCutoff;
        m_fPrevQ = m_fQ;
        m_fPrevGain = m_fGain;
    }

    F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;


    F32 fPrevIn0;
    F32 fPrevOut0;
    F32 fPrevIn1;
    F32 fPrevOut1;

    //  each channel
    for (U32 uCh = 0; uCh < m_uChannels; uCh++)
    {
        fPrevIn0 = m_fPrevIn0[uCh];
        fPrevOut0 = m_fPrevOut0[uCh];
        fPrevIn1 = m_fPrevIn1[uCh];
        fPrevOut1 = m_fPrevOut1[uCh];

        //  each sample
        for (U32 i = 0; i < uFrameLength; i++)
        {
            U32 dwSample = uCh * uFrameLength + i;

            F32 fIn = aSample[dwSample];
            F32 fOut = m_fB0 * fIn + m_fB1 * fPrevIn0 + m_fB0 * fPrevIn1 - m_fA1 * fPrevOut0 - m_fA2 * fPrevOut1;

			fOut = Maths::Clamp(fOut, -m_fClamp, m_fClamp);

            fPrevIn1 = fPrevIn0;
            fPrevOut1 = fPrevOut0;
            fPrevIn0 = fIn;
            fPrevOut0 = fOut;

            aSample[dwSample] = fOut;
        }

        m_fPrevIn0[uCh] = fPrevIn0;
        m_fPrevOut0[uCh] = fPrevOut0;
        m_fPrevIn1[uCh] = fPrevIn1;
        m_fPrevOut1[uCh] = fPrevOut1;
    }


    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
