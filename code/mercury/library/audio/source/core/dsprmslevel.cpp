#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::RMSLevel::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	RMSLevel
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::RMSLevel::RMSLevel( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::eRMSLevel;

	for (int uCh=0; uCh<6; uCh++)
		m_fMean[uCh] = 0.f;

	m_fLevel = 0.f;
	m_fLength = 0.1f;
	m_fPrevLength = 0.f;
	m_fA = 0.f;
	m_fB = 0.f;

    for (U32 uCh = 0; uCh < m_uChannels; uCh++)
		m_fPrevLevel[uCh] = 0.f;
#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::RMSLevel::Process( void* pFrameData, const ProcessInfo& info, bool bProcess )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.RMSLevel.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	if( m_fLength != m_fPrevLength )
	{
		m_fB = Maths::Exp( -1.f / ( m_fLength * fSampleRate ) );
		m_fA = 1.f - m_fB;

		m_fPrevLength = m_fLength;
	}

	F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;

	F32 fMeanTotal = 0.f;

	// each channel
	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
        F32 fMean = m_fMean[ uCh ];
        F32 fFrameIndex = 0.f;

		//	each sample
#ifdef PLATFORM_SPU
			for (U32 uSample = 0; uSample < uFrameLength; uSample+=4)
			{
				fFrameIndex += 4.f;

				//	mean square
				vector float fAbs = { ( aSample[ uCh * uFrameLength + uSample + 0] ),
									  ( aSample[ uCh * uFrameLength + uSample + 1] ),
									  ( aSample[ uCh * uFrameLength + uSample + 2] ),
									  ( aSample[ uCh * uFrameLength + uSample + 3] )};
				fAbs = (vector float)( spu_rlmask( spu_sl( (vector unsigned int)fAbs, 1 ), -1 ) );
				
				vector float fSquare =  spu_mul( fAbs, fAbs );
				
				fMean = m_fA * spu_extract( fSquare, 0 )+ m_fB * fMean;
				fMean = m_fA * spu_extract( fSquare, 1 )+ m_fB * fMean;
				fMean = m_fA * spu_extract( fSquare, 2 )+ m_fB * fMean;
				fMean = m_fA * spu_extract( fSquare, 3 )+ m_fB * fMean;
			}

#else	
		for( U32 uSample = 0; uSample < uFrameLength; uSample++ )
		{
			fFrameIndex += 1.f;

			//	mean square
			F32 fAbs = Maths::Abs( aSample[ uCh * uFrameLength + uSample ] );
			F32 fSquare = fAbs * fAbs;
            fMean = m_fA * fSquare + m_fB * fMean;
		}
#endif

		m_fMean[ uCh ] = fMean;
		fMeanTotal += fMean;
	}

	//	rms
	fMeanTotal /= (F32)m_uChannels;
	m_fLevel = Maths::Sqrt( fMeanTotal );
	
	m_fLeftLevel = Maths::Sqrt( m_fMean[ 0 ] );
	m_fRightLevel = Maths::Sqrt( m_fMean[ 1 ] );

	return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
