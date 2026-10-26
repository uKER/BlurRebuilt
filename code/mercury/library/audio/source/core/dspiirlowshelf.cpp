#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::IIRLowShelf::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	IIRLowShelf
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::IIRLowShelf::IIRLowShelf( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::eIIRLowShelf;

	m_fTargetCutoff = 440.f;
    m_fPrevCutoff = -1.f;
    m_fPrevSlope = -1.f;
    m_fPrevGain = -1.f;
    m_fCutoff = 440.f;
    m_fSlope = 1.f;
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
bool Audio::Core::DSP::IIRLowShelf::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	//	limit freq slew rate
	F32 fCutoffDelta = (m_fTargetCutoff - m_fCutoff);
	F32 fStep = m_fCutoff * fFrameLength * 0.001953125f; // 1/512 - 1oct per 256 samp frame
	fCutoffDelta = Maths::Clamp(fCutoffDelta, -fStep, fStep);
	m_fCutoff += fCutoffDelta;

	if ((m_fCutoff != m_fPrevCutoff) || (m_fSlope != m_fPrevSlope) || (m_fGain != m_fPrevGain))
	{
		F32 fOmega = (Maths::f2Pi * m_fCutoff * Core::fRcpSampleRate);
		F64 fSin = (F64)Maths::Sin(fOmega);
		F64 fCos = (F64)Maths::Cos(fOmega);
		F64 fBeta = fSin * (F64)Maths::SlowSqrt(m_fGain) / (F64)m_fSlope;
		F64 fTemp0 = m_fGain + 1.0;
		F64 fTemp1 = m_fGain - 1.0;
		F64 fTemp2 = fTemp0 * fCos;
		F64 fTemp3 = fTemp1 * fCos;
		F64 fA0 = 1.0 / (fTemp0 + fTemp3 + fBeta);
		m_fA1 = (F32)((-2.0 * (fTemp1 + fTemp2)) * fA0);
		m_fA2 = (F32)((fTemp0 + fTemp3 - fBeta) * fA0);
		m_fB0 = (F32)((m_fGain * (fTemp0 - fTemp3 + fBeta)) * fA0);
		m_fB1 = (F32)((2.0 * m_fGain * (fTemp1 - fTemp2)) * fA0);
		m_fB2 = (F32)((m_fGain * (fTemp0 - fTemp3 - fBeta)) * fA0);

		m_fPrevCutoff = m_fCutoff;
        m_fPrevSlope = m_fSlope;
        m_fPrevGain = m_fGain;
    }

	if (bProcess)
	{
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
				F32 fOut = m_fB0 * fIn + m_fB1 * fIn1 + m_fB2 * fIn2 - m_fA1 * fOut1 - m_fA2 * fOut2;
#ifdef PLATFORM_PC
				static const F32 fTinyVal = 1e-18;
				fOut += fTinyVal;
#endif
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
	}

    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
