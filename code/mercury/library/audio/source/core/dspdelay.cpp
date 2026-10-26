#include "AudioPch.h"
#include "coreheaders.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

//#define AUDIO_DO_FILE

#ifdef AUDIO_DO_FILE
#include "../common/file.h"
static int g_iExit = 0;
static Audio::File g_File;
static int g_iFrames = 768;
#endif

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Delay::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	Delay
////////////////////////////////////////////////////////////////////////////////
Audio::Core::DSP::Delay::Delay( const char* pchInfo, U16 uChannels, U32 uNumFrames, U32 uDownSample ) : Effect( pchInfo, uChannels )
{
	m_eType = Core::DSP::Effect::eDelay;

	const U32 uDelayFrameLength = uFrameLength >> uDownSample;
	m_uBufferLength = uNumFrames * uDelayFrameLength;

	m_uDownSample = uDownSample;

	m_fTargetLength = 0.f;
    m_sCurrentLength = 0;
    m_fPrevSample = 0.f;
    m_fWet = 1.f;
    m_fDry = 0.f;
    m_uRead = 0;
	m_uWrite = 0;

	m_bReset = true;

	m_fPrevIn = 0.f;	//	TODO: sort it out, this is only mono
	m_fPrevOut = 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	Process
//		Test stereo
//		pFrameData to F32 for all fx
//		sort restricts
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::Delay::Process( void* pFrameData, const ProcessInfo& info, bool bProcess )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.Delay.Process");

	if( !m_bActive || bDisableAll )
		return true;

	//static F32 fSin = 0.f;
	//static F32 fInc = 0.01f; // 1/0.2/0.01 (0->1)
	//static F32 fMul = 0.01f; // 0.1/0.01 (0->0.01)
	//m_fTargetLength = 0.5f + 0.5f * Maths::Sin( fSin );
	//static F32 fPow = 2.f;
	//m_fTargetLength = fMul * Maths::Power( m_fTargetLength, fPow );
	//fSin += fInc;

	//	Flanger:
	//	LFODepth/Width
	//	LFOSpeed/Rate
	//	Delay

	if( m_bReset )
	{
		F32* pfDelayBuffer = &info.pStaticBuffers[ m_uBuffer ];
		memset( pfDelayBuffer, 0, m_uBufferLength * sizeof(F32) );
		m_bReset = false;
	}

	// downsampled frame size
	const U32 uDelayFrameLength = uFrameLength >> m_uDownSample;

	// get input buffer
	F32* pfFrameBuffer = (F32*)pFrameData;

	// copy into delay for each channel
	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
		// get delay buffer
		F32* pfDelayBuffer = &info.pStaticBuffers[ m_uBuffer ];

		// copy frame into delay buffer
        Resample( &pfDelayBuffer[ m_uWrite + uCh * m_uBufferLength ], uDelayFrameLength, 0.f, &pfFrameBuffer[ uCh * uFrameLength ], uFrameLength, 1.f, m_fPrevIn);
		m_fPrevIn = pfFrameBuffer[ uCh * uFrameLength + uFrameLength - 1 ];
	}

	// inc write index
	m_uWrite += uDelayFrameLength;
	if( m_uWrite >= m_uBufferLength )
		m_uWrite -= m_uBufferLength;

	// space in buffer to deal with overlap required at max length
	S32 sOverlap = uDelayFrameLength * info.uFrameInt;
	
	// target delay length in samples
	S32 sTargetLength = (S32)( m_fTargetLength * fSampleRate );

	// max delay length available
	S32 sMaxLength = (S32)( m_uBufferLength - sOverlap );

	// clamp delay length to max
	sTargetLength = sTargetLength < sMaxLength ? sTargetLength : sMaxLength;

	// get number of samples to use from the delay buffer for this output frame
	F32 fReadSize = (F32)sOverlap - ( sTargetLength - m_sCurrentLength );

	// clamp to buffer length (minus overlap frames)
	S32 sReadSize = (S32)Maths::Max( fReadSize * info.fWidth, 0.f );

	// copy from delay for each channel
	for( U32 uCh = 0; uCh < m_uChannels; uCh++ )
	{
		// only read if samples required this frame
		if( sReadSize != 0 )
		{
			// get delay buffer
			F32* pfDelayBuffer = &info.pStaticBuffers[ m_uBuffer ];

			// crossing circular buffer boundary?
			if( ( m_uRead + sReadSize ) <= m_uBufferLength )
			{
				// copy delayed frame from delay buffers
				Resample( &pfFrameBuffer[ uCh * uFrameLength ], uFrameLength, m_fDry, &pfDelayBuffer[ m_uRead + uCh * m_uBufferLength ], sReadSize, m_fWet, m_fPrevOut );
				m_fPrevOut = pfDelayBuffer[ m_uRead + uCh * m_uBufferLength + sReadSize - 1 ];
			}
			else
			{
				// deal with wrap around
				U32 u1stIn = m_uBufferLength - m_uRead;
				U32 u1stOut = (U32)( (F32)( u1stIn * uFrameLength ) / (F32)sReadSize );

				// copy 1st half of delayed frame from delay buffers
				Resample( &pfFrameBuffer[ uCh * uFrameLength ], u1stOut, m_fDry, &pfDelayBuffer[ m_uRead + uCh * m_uBufferLength ], u1stIn, m_fWet, m_fPrevOut );

				U32 u2ndIn = (U32)sReadSize - u1stIn;
				U32 u2ndOut = uFrameLength - u1stOut;

				// copy 2nd half of delayed frame from delay buffers
				Resample( &pfFrameBuffer[ u1stOut + uCh * uFrameLength ], u2ndOut, m_fDry, &pfDelayBuffer[ uCh * m_uBufferLength ], u2ndIn, m_fWet, pfDelayBuffer[ m_uBufferLength + uCh * m_uBufferLength - 1 ] );
				m_fPrevOut = pfDelayBuffer[ uCh * m_uBufferLength + u2ndIn - 1 ];
			}
		}
		else
			Frame::Zero( pfFrameBuffer );
	}

	// inc read index
	m_uRead += sReadSize;
	if( m_uRead >= m_uBufferLength )
		m_uRead -= m_uBufferLength;

	if( info.uFrameMod == info.uFrameInt - 1 )
	{
		// store current length
		m_sCurrentLength = m_uWrite - m_uRead;
		if( m_sCurrentLength < 0 )
			m_sCurrentLength += m_uBufferLength;
	}

#ifdef AUDIO_DO_FILE

    if (g_iExit == 0)
    {
        g_File.OpenWrite("game:\\audio.raw");
        g_iExit = 1;
    }
    if (g_iExit == 1)
    {
        if (g_iFrames)
        {
            for (int i=0; i<256; i++)
            {
                U8* adw = (U8*)&pfFrameBuffer[i];
                U8 uTemp = adw[0];
                adw[0] = adw[3];
                adw[3] = uTemp;
                uTemp = adw[1];
                adw[1] = adw[2];
                adw[2] = uTemp;
            }
            g_File.Write(pfFrameBuffer, uFrameLength * sizeof(F32));
            for (int i=0; i<256; i++)
            {
                U8* adw = (U8*)&pfFrameBuffer[i];
                U8 uTemp = adw[0];
                adw[0] = adw[3];
                adw[3] = uTemp;
                uTemp = adw[1];
                adw[1] = adw[2];
                adw[2] = uTemp;
            }
            g_iFrames--;
        }
        else
            g_iExit = 2;
    }
    if (g_iExit == 2)
    {
        g_File.Close();
        g_iExit = 3;
    }
#endif

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	Resample
//		Simple linear SRC
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::DSP::Delay::Resample( F32* afDst, U32 uOutSize, F32 fDry, F32* afSrc, U32 uInSize, F32 fWet, F32 fPrev )
{
	if( uInSize != uOutSize )
	{
		F32 fPitch = (F32)uInSize / (F32)uOutSize;

		for( U32 uCount = 0; uCount < uOutSize; uCount++ )
		{
			F32 fSubSample = (F32)uCount * fPitch;
			F32 fIndex = Audio::Maths::Floor( fSubSample );
			F32 fFraction = fSubSample - fIndex;
			S32 sIndex = (S32)fIndex;

			F32 fSampleA;
			F32 fSampleB;
			if( sIndex == 0 )
			{
				fSampleA = fPrev;
				fSampleB = afSrc[ sIndex ];
			}
			else
			{
				fSampleA = afSrc[ sIndex - 1];
				fSampleB = afSrc[ sIndex ];
			}

			F32 fDiff = fSampleB - fSampleA;
			F32 fDelta = fDiff * fFraction;
			afDst[ uCount ] = fDry * afDst[ uCount ] + fWet * fSampleA + fDelta;

#ifdef AUDIO_CHECK_SAMPLES
			
			if (fabs(afDst[uCount]) > 100.f || !_finite(afDst[uCount]))
				AUDIO_CORE_ERROR(DBSTR" Invalid sample of value %.2f in effect '%s'", afDst[uCount], m_pchInfo);
				
#endif // AUDIO_CHECK_SAMPLES  
		}
	}
	else
	{
		for( U32 uCount = 0; uCount < uOutSize; uCount++ )
		{
			afDst[ uCount ] = fDry * afDst[ uCount ] + fWet * afSrc[ uCount ];

#ifdef AUDIO_CHECK_SAMPLES
			
			if (fabs(afDst[uCount]) > 100.f || !_finite(afDst[uCount]))
				AUDIO_CORE_ERROR(DBSTR" Invalid sample of value %.2f in effect '%s'", afDst[uCount], m_pchInfo);
#endif // AUDIO_CHECK_SAMPLES  

		}
	}
}

