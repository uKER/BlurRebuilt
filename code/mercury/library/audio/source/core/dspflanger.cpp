#include "AudioPch.h"
#include "coreheaders.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Flanger::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	Flanger
////////////////////////////////////////////////////////////////////////////////
Audio::Core::DSP::Flanger::Flanger( const char* pchInfo, U16 uChannels, U32 uNumFrames ) : Effect( pchInfo, uChannels )
{
	m_eType = Core::DSP::Effect::eFlanger;

	Assert( m_uChannels <= sMaxChannels );

    m_uDelayBuffer = 0;

	m_uBufferLength = ( 2 + uNumFrames ) * uFrameLength;

	m_fAngle = 0.f;
	m_fPhaseShift = Maths::f2Pi / (F32)uChannels;

    m_fDry = 1.f;
    m_fWet = 1.f;
    m_fDelay = 0.0005f;
    m_fDepth = 0.005f;
    m_fDelta = 0.01f;
    m_fFeedback = -0.5f;

	m_uRead = 0;

	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
		m_uWrite[ uCh ] = uFrameLength;

	m_bReset = true;
}

////////////////////////////////////////////////////////////////////////////////
//	Process
//		Test stereo
//		pFrameData to F32 for all fx
//		sort restricts
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Flanger::Process( void* pFrameData, const ProcessInfo& info, bool bProcess )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.Flanger.Process");

	if( !m_bActive || bDisableAll )
		return true;

	if( m_bReset )
	{
		// clear delay buffer
		F32* pfDelayBuffer = &info.pStaticBuffers[ m_uDelayBuffer ];
		memset( pfDelayBuffer, 0, m_uChannels * m_uBufferLength * sizeof(F32) );
		m_bReset = false;
	}

	// get input buffer
	F32* AUDIO_RESTRICT pfFrameBuffer = (F32*)pFrameData;

	// get delay buffer
	F32* pfTempBuffer = &info.pDynamicBuffers[ m_uScratchBuffer ];

	// for each channel
	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
		F32* AUDIO_RESTRICT pfChannelBuffer = (F32*)&pfFrameBuffer[ uCh * uFrameLength ];

		//	get current length
		S32 sCurrLength = m_uWrite[ uCh ] - m_uRead;
		if( sCurrLength < 0 )
			sCurrLength += m_uBufferLength;

		// use exp curve for lfo shape on delay : TODO: not use Sin?
		F32 fPhase = Maths::Power( 0.5f + 0.5f * Maths::Sin( m_fAngle + (F32)uCh * m_fPhaseShift ), 2.f );

		// new target length
		F32 fTargetLength = Maths::Max( m_fDelay + m_fDepth * fPhase, 0.f );

		// target delay length in samples
		S32 sTargetLength = (S32)uFrameLength + (S32)( fTargetLength * fSampleRate ) - sCurrLength;

		// get space available
		S32 sSpace = m_uBufferLength - sCurrLength - (S32)uFrameLength;

		// clamp delay length to space available
		sTargetLength = sTargetLength < sSpace ? sTargetLength : sSpace;

		// resample length for one frame
		S32 sResampleLength = (S32)uFrameLength + sTargetLength;

		// clamp delay length to min
		sResampleLength = sResampleLength > 0 ? sResampleLength : 0;

		// copy input to temp buffer
		Frame::Copy( pfTempBuffer, pfChannelBuffer );

		// get channel delay buffer
		F32* AUDIO_RESTRICT pfDelayBuffer = &info.pStaticBuffers[ m_uDelayBuffer ] + uCh * m_uBufferLength;

		// read delay to temp buffer
		Read( info, pfTempBuffer, pfDelayBuffer, m_fFeedback );

		// write temp to delay buffer
		Write( info, m_src[ uCh ], pfTempBuffer, pfDelayBuffer, m_uWrite[ uCh ], sResampleLength );

		// set dry gain
		if( m_fDry != 1.f )
			Frame::Gain( pfChannelBuffer, pfChannelBuffer, m_fDry );

		// copy delay to temp buffer
		Read( info, pfChannelBuffer, pfDelayBuffer, m_fWet );

		// inc channel write index
		m_uWrite[ uCh ] += sResampleLength;
		if( m_uWrite[ uCh ] >= m_uBufferLength )
			m_uWrite[ uCh ] -= m_uBufferLength;
	}

	// inc read index
	m_uRead += uFrameLength;
	if( m_uRead >= m_uBufferLength )
		m_uRead -= m_uBufferLength;

	// inc phase angle
	m_fAngle += m_fDelta;
	if( m_fAngle > Maths::f2Pi )
		m_fAngle -= Maths::f2Pi;

#ifndef CFG_GOLD
	/*for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
		if( !Frame::IsValid( &pfFrameBuffer[ uCh * Core::uFrameLength ] ) )
			AUDIO_CORE_ERROR("Bad samples in flanger effect");
	}*/
#endif

	return true;
}

void Audio::Core::DSP::Flanger::Read( const ProcessInfo& info, F32* pFrameBuffer, F32* pfDelayBuffer, F32 fGain )
{
	//	note: currently no gain ramping
	if( fGain != 0.f )
		Frame::AddGain( pFrameBuffer, pFrameBuffer, &pfDelayBuffer[ m_uRead ], fGain );
}

void Audio::Core::DSP::Flanger::Write( const ProcessInfo& info, LinearSRC& src, const F32* pFrameBuffer, F32* pfDelayBuffer, U32 uWriteIndex, U32 uTargetLength )
{
	if( uWriteIndex + uTargetLength > m_uBufferLength )
	{
		U32 u1stOut = m_uBufferLength - uWriteIndex;
		U32 u1stIn = (U32)( (F32)( u1stOut * uFrameLength ) / (F32)uTargetLength );

		// copy 1st half of delayed frame from delay buffers
		src.Resample( &pfDelayBuffer[ uWriteIndex ], u1stOut, pFrameBuffer, u1stIn );

		U32 u2ndOut = (U32)uTargetLength - u1stOut;
		U32 u2ndIn = uFrameLength - u1stIn;

		// mix current output frame into delay
		src.Resample( pfDelayBuffer, u2ndOut, &pFrameBuffer[ u1stIn ], u2ndIn );
	}
	else
	{
		// mix current output frame into delay
		src.Resample( &pfDelayBuffer[ uWriteIndex ], uTargetLength, pFrameBuffer, uFrameLength );
	}
}

void Audio::Core::DSP::Flanger::LinearSRC::Resample( F32* afDst, U32 uOutSize, F32 fDry, const F32* afSrc, U32 uInSize, F32 fWet )
{
	if( uInSize != 0 && uOutSize != 0 )
	{
		if( uInSize != uOutSize )
		{
			//	get pitch step size
			F32 fPitch = (F32)uInSize / (F32)uOutSize;

			//	initialize values
			U32 uIndexOut = 0;
			F32 fIndexOut = 1.f;
			F32 fSubSample = fPitch;

			//	do interpolation using history
			for( ; fSubSample < 1.f; uIndexOut++, fIndexOut += 1.f, fSubSample = fIndexOut * fPitch )
			{
				afDst[ uIndexOut ] = fDry * afDst[ uIndexOut ] + fWet * Interpolate( m_fHistory, afSrc[ 0 ], fSubSample );
			}

			//	do interpolation across input
			for( ; uIndexOut < uOutSize; uIndexOut++, fIndexOut += 1.f )
			{
				fSubSample = fIndexOut * fPitch;

				F32 fIndexIn = Audio::Maths::Floor( fSubSample );
				F32 fFraction = fSubSample - fIndexIn;
				U32 uIndexIn = (U32)fIndexIn;

				afDst[ uIndexOut ] = fDry * afDst[ uIndexOut ] + fWet * Interpolate( afSrc[ uIndexIn - 1 ], afSrc[ uIndexIn ], fFraction );
			}

			m_fHistory = afSrc[ uInSize - 1 ];
		}
		else
		{
			for( U32 uCount = 0; uCount < uOutSize; uCount++ )
				afDst[ uCount ] = fDry * afDst[ uCount ] + fWet * afSrc[ uCount ];
		}
	}
}

//	no gain applied
void Audio::Core::DSP::Flanger::LinearSRC::Resample( F32* afDst, U32 uOutSize, const F32* afSrc, U32 uInSize )
{
	if( uInSize != 0 && uOutSize != 0 )
	{
		if( uInSize != uOutSize )
		{
			//	get pitch step size
			F32 fPitch = (F32)uInSize / (F32)uOutSize;

			//	initialize values
			U32 uIndexOut = 0;
			F32 fIndexOut = 1.f;
			F32 fSubSample = fPitch;

			//	do interpolation using history
			for( ; fSubSample < 1.f; uIndexOut++, fIndexOut += 1.f, fSubSample = fIndexOut * fPitch )
			{
				afDst[ uIndexOut ] = Interpolate( m_fHistory, afSrc[ 0 ], fSubSample );
			}

			//	do interpolation across input
			for( ; uIndexOut < uOutSize; uIndexOut++, fIndexOut += 1.f )
			{
				fSubSample = fIndexOut * fPitch;

				F32 fIndexIn = Audio::Maths::Floor( fSubSample );
				F32 fFraction = fSubSample - fIndexIn;
				U32 uIndexIn = (U32)fIndexIn;

				afDst[ uIndexOut ] = Interpolate( afSrc[ uIndexIn - 1 ], afSrc[ uIndexIn ], fFraction );
			}

			m_fHistory = afSrc[ uInSize - 1 ];
		}
		else
		{
			for( U32 uCount = 0; uCount < uOutSize; uCount++ )
				afDst[ uCount ] = afSrc[ uCount ];
		}
	}
}
