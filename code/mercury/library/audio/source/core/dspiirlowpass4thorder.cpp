#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::IIRLowPass4thOrder::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	IIRLowPass4thOrder
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::IIRLowPass4thOrder::IIRLowPass4thOrder( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::eIIRLowPass4thOrder;

	m_fTargetCutoff = fSampleRate * 0.5f;
    m_fPrevCutoff = -1.f;
    m_fPrevQ = -1.f;
    m_fCutoff = fSampleRate * 0.5f;
    m_fQ = 0.f;
    m_fGain = 1.f;
	m_fClamp = Maths::fMax;

	m_fP = 0.f;
	m_fR = 0.f;
	m_fK = 0.f;

    for (U32 uCh = 0; uCh < uChannels; uCh++)
    {
		m_fY1[uCh] = 0.f;
		m_fY2[uCh] = 0.f;
		m_fY3[uCh] = 0.f;
		m_fY4[uCh] = 0.f;
		m_fPrevX[uCh] = 0.f;
		m_fPrevY1[uCh] = 0.f;
		m_fPrevY2[uCh] = 0.f;
		m_fPrevY3[uCh] = 0.f;
    }

#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::IIRLowPass4thOrder::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.IIRLowPass4thOrder.Process");

//temp, disable on spu for now, to save memory
#ifndef PLATFORM_SPU
#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	//	limit freq slew rate
	F32 fCutoffDelta = (m_fTargetCutoff - m_fCutoff);
	if (fCutoffDelta != 0.f)
	{
		F32 fStep = m_fCutoff * fFrameLength * 0.001953125f; // 1/512 - 1oct per 256 samp frame
		fCutoffDelta = Maths::Clamp(fCutoffDelta, -fStep, fStep);
		m_fCutoff += fCutoffDelta;
	}

    if ( ( m_fCutoff != m_fPrevCutoff ) || ( m_fQ != m_fPrevQ ) )
    {
		F32 fFreq = ( m_fCutoff + m_fCutoff ) / Core::uSampleRate;
		m_fP = fFreq * ( 1.8f - 0.8f * fFreq );
		m_fK = m_fP + m_fP - 1.f;
		F32 fTemp1 = ( 1.f - m_fP ) * 1.386249f;
		F32 fTemp2 = 12.f + fTemp1 * fTemp1;
		m_fR = m_fQ * ( fTemp2 + 6.f * fTemp1) / ( fTemp2 - 6.f * fTemp1 );

        m_fPrevCutoff = m_fCutoff;
        m_fPrevQ = m_fQ;
    }

    F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;
    
    //  each channel
    for ( U32 uCh = 0; uCh < m_uChannels; ++uCh )
	{
		F32 fY1 = m_fY1[uCh];
		F32 fY2 = m_fY2[uCh];
		F32 fY3 = m_fY3[uCh];
		F32 fY4 = m_fY4[uCh];
		F32 fPrevX = m_fPrevX[uCh];
		F32 fPrevY1 = m_fPrevY1[uCh];
		F32 fPrevY2 = m_fPrevY2[uCh];
		F32 fPrevY3 = m_fPrevY3[uCh];

        //  each sample
        for ( U32 uSample = 0; uSample < uFrameLength; uSample++ )
		{
            const S32 i = uCh * uFrameLength + uSample;
            const F32 fIn0  = aSample[i];
            
			F32 fX = fIn0 - m_fR * fY4;

			fY1 = m_fP * ( fX + fPrevX ) - m_fK * fY1;
			fY2 = m_fP * ( fY1 + fPrevY1 ) - m_fK * fY2;
			fY3 = m_fP * ( fY2 + fPrevY2 ) - m_fK * fY3;
			fY4 = m_fP * ( fY3 + fPrevY3 ) - m_fK * fY4;
			fY4 -= ( fY4 * fY4 * fY4 ) / 6.f;
			fPrevX = fX;

#ifdef PLATFORM_PC
			static const F32 fTinyVal = 1e-18;
			fPrevY1 = fY1 + fTinyVal;
			fPrevY2 = fY2 + fTinyVal;
			fPrevY3 = fY3 + fTinyVal;
#else
			fPrevY1 = fY1;
			fPrevY2 = fY2;
			fPrevY3 = fY3;
#endif
			aSample[i] = m_fGain * fY4;
		}

		m_fY1[uCh] = fY1;
		m_fY2[uCh] = fY2;
		m_fY3[uCh] = fY3;
		m_fY4[uCh] = fY4;
		m_fPrevX[uCh] = fPrevX;
		m_fPrevY1[uCh] = fPrevY1;
		m_fPrevY2[uCh] = fPrevY2;
		m_fPrevY3[uCh] = fPrevY3;
	}

#endif // AUDIO_DISABLE_DSP
#endif	//	#ifndef PLATFORM_SPU

	return true;
}
