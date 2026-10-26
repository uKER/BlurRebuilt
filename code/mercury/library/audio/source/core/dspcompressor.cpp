#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Compressor::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	Compressor
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::Compressor::Compressor( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP
	m_eType = Core::DSP::Effect::eCompressor;

	//	calc rms lp-filter coeffs
	const F32 fLength = 0.1f;
	m_fB = Maths::Exp( -1.f / ( fLength * fSampleRate ) );	//	needs hi-q
	m_fA = 1.f - m_fB;

	m_fPrev = 0.f;

	for (int uCh=0; uCh<6; uCh++)
		m_fMean[uCh] = 0.f;

	m_fSideChainRMS = 0.f;
	m_fRcpAttack = 1000.f;
    m_fRcpRelease = 2.f;
    m_fThreshold = 1.f;
    m_fRcpRatio = 1.f;
    m_fGain = 1.f;

	m_fComp = 1.f;

	m_fPrevComp = 0.f;

	m_bSideChainEnabled = false;
#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

//U32 g_this = NULL;
////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Compressor::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.Compressor.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	if ( m_bSideChainEnabled )
	{
		//	using side chain for RMS, just apply compressor gain
		F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;

		const F32 fPrevGain = m_fGain * m_fPrevComp;
		const F32 fGainDelta = ( m_fGain * m_fComp - fPrevGain ) * Core::fRcpFrameLength;

		// each channel
		for (U32 uCh = 0; uCh < m_uChannels; uCh++)
		{
			F32 fFrameIndex = 0.f;

			//	each sample
			for (U32 uSample = 0; uSample < uFrameLength; uSample++)
			{
				const F32 fGain = fPrevGain + fFrameIndex * fGainDelta;
				fFrameIndex += 1.f;

				aSample[ uCh * uFrameLength + uSample ] *= fGain;
			}
		}
	}
	else
	{
		//	not using side chain, apply compressor gain and calc rms
		F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;

		const F32 fPrevGain = m_fGain * m_fPrevComp;
		const F32 fGainDelta = ( m_fGain * m_fComp - fPrevGain ) * Core::fRcpFrameLength;

		F32 fMeanTotal = 0.f;

		// each channel
		for (U32 uCh = 0; uCh < m_uChannels; uCh++)
		{
			F32 fMean = m_fMean[ uCh ];
			F32 fFrameIndex = 0.f;

			//	each sample
#ifdef PLATFORM_SPU
			for (U32 uSample = 0; uSample < uFrameLength; uSample+=4)
			{
				const vector float fGain = {	fPrevGain + ((fFrameIndex		) * fGainDelta),
												fPrevGain + ((fFrameIndex + 1.0f	) * fGainDelta),
												fPrevGain + ((fFrameIndex + 2.0f	) * fGainDelta),
												fPrevGain + ((fFrameIndex + 3.0f	) * fGainDelta) };
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

				//	TODO: ramp it
				vector float* asmp = (vector float*)&aSample[ uCh * uFrameLength + uSample + 0];
				*asmp = spu_mul( *asmp, fGain );
			}

#else			
			for (U32 uSample = 0; uSample < uFrameLength; uSample++)
			{
				const F32 fGain = fPrevGain + fFrameIndex * fGainDelta;
				fFrameIndex += 1.f;

				//	mean square
				F32 fAbs = Maths::Abs( aSample[ uCh * uFrameLength + uSample ] );
				F32 fSquare = fAbs * fAbs;
				fMean = m_fA * fSquare + m_fB * fMean;

				//	TODO: ramp it
				aSample[ uCh * uFrameLength + uSample ] *= fGain;
			}
#endif
			m_fMean[ uCh ] = fMean;
			fMeanTotal += fMean;
		}

		//	calc rms
		fMeanTotal /= (F32)m_uChannels;
		m_fRMS = Maths::Sqrt( fMeanTotal );
	}

	//	get target gain
	F32 fTarget = Maths::Select( m_fRMS - m_fThreshold, ( m_fThreshold + m_fRcpRatio * ( m_fRMS - m_fThreshold ) ) * Maths::RcpEst( m_fRMS ), 1.f );

	//	envelope deltas
	F32 fAttackDelta = fTarget * m_fRcpAttack;
    F32 fReleaseDelta = fTarget * m_fRcpRelease;

    //  update compression
	m_fPrevComp = m_fComp;

	//	do envelope
    if ( fTarget < m_fComp )
    {
        m_fComp -= fAttackDelta;
		m_fComp = Maths::Max( m_fComp, fTarget );
    }
    else if ( fTarget > m_fComp )
    {
        m_fComp += fReleaseDelta;
		m_fComp = Maths::Min( m_fComp, 1.f );
    }

	return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
