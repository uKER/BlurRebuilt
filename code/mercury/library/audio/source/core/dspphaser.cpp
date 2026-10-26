#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Phaser::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	Phaser
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::Phaser::Phaser( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::ePhaser;

	m_fDry = 1;
	m_fWet = -1;
	m_fAngle = 0;;
	m_fSpeed = 100;
	m_fWidth = 0.5;
	m_fFeedback = 0;

	m_uCurrOffset = 0;
#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Phaser::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.Phaser.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	F32* aSample = (F32*)pFrameData;

	F32 f2Pi = 2.f * Maths::fPi;
		m_fAngle += m_fSpeed * f2Pi / 48000.f;

		if (m_fAngle > f2Pi)
			m_fAngle -= f2Pi;

		F32 fPhase = m_fWidth * (0.5f + 0.5f * Maths::Sin(m_fAngle));

		U32 uCurrOffset = 0;

        //  each channel
        for (U32 ch = 0; ch < m_uChannels; ch++)
        {
			uCurrOffset = m_uCurrOffset;

            //  each sample
            for (U32 i = 0; i < uFrameLength; i++)
            {
				F32 fDelay = 2.f + fPhase * ((F32)PHASER_DELAY_BUFFER_LENGTH - 5.f);
				U32 uCurrDelay = (U32)fDelay;
				F32 fFrac;

				fFrac = 1.f - (fDelay - (F32)uCurrDelay);

				uCurrDelay += 3;

				S32 sReadOffset1 = uCurrOffset - uCurrDelay;
				if (sReadOffset1 < 0)
					sReadOffset1 += PHASER_DELAY_BUFFER_LENGTH;

				S32 sReadOffset2 = sReadOffset1 + 1;
				if (sReadOffset2 == PHASER_DELAY_BUFFER_LENGTH)
					sReadOffset2 = 0;

				F32 fDiff = m_afDelayBuffer[ch][sReadOffset2] - m_afDelayBuffer[ch][sReadOffset1];

				F32 fWet = m_fWet * (m_afDelayBuffer[ch][sReadOffset1] + fFrac * fDiff);

				m_afDelayBuffer[ch][uCurrOffset] = aSample[ch * uFrameLength + i] + m_fFeedback * fWet;

				F32 fDry = m_fDry * aSample[ch * uFrameLength + i];

				uCurrOffset++;
				if (uCurrOffset == (U32)PHASER_DELAY_BUFFER_LENGTH)
					uCurrOffset = 0;

				aSample[ch * uFrameLength + i] = fDry + fWet;
            }
        }

		m_uCurrOffset = uCurrOffset;
 //   }

    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
