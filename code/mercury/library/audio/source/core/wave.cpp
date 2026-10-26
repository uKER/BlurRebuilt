#include "AudioPch.h"
#include "coreheaders.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
//#include <System\SPU\AutoPointer.h>
#endif


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#if !defined PLATFORM_PS3 && !defined PLATFORM_SPU
Debug::Bool gbLogWavesWhenPlayed("Audio.System.Logging.LogWavesWhenPlayed", false);
#endif

const U32 Audio::Core::Wave::u32DecoderNone = 0xffffffff;

////////////////////////////////////////////////////////////////////////////////
//	WaveSize (static)
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Core::Wave::WaveSize( U16 uChannelsOut, U16 uEffects )
{
	U32 uSize = sizeof( Wave );

	//	effects
	uSize += sizeof( DSP::Element ) * uEffects;

	return AUDIO_CORE_ALIGN_16( uSize );
}

////////////////////////////////////////////////////////////////////////////////
//	Wave
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Wave::Wave( const char* pchInfo, U32 u32NumChannels, U16 uEffects, bool bUseFastRamp, bool bAlwaysOn ) : Generator( pchInfo, u32NumChannels, bAlwaysOn )
{
	m_uClassSize = sizeof( Wave );

	m_uEffects = uEffects;

	Generator::m_eType = eWave;

    m_bPlaying = false;
	m_bDone = false;
	m_bUseFastRamp = bUseFastRamp;

	m_fPrevSubVolume = 0.f;
	m_fPrevVolume = 0.f;
    m_fVolume = 0.f;
	m_uDecoderType = u32DecoderNone;
	m_uDecoder = u32DecoderNone;

#ifndef CFG_GOLD
	Generator::m_uTotalWave++;
#endif

#ifdef AUDIO_NEW_DECODERS
	m_pDecoder = NULL;
#endif // AUDIO_NEW_DECODERS
}

////////////////////////////////////////////////////////////////////////////////
//	~Wave
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Wave::~Wave( )
{
#ifndef CFG_GOLD
	if (IsActive())
		Generator::m_uActiveWave--;

	Generator::m_uTotalWave--;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Wave::SetProcessFlag( const ProcessInfo& info, bool bProcess )
{
	if (info.bMutePrev)
		m_fPrevVolume = 0.f;

	if (info.bMuteNext)
		m_fVolume = 0.f;

	bool bProcessing = m_bAlwaysOn;

	if( m_bPlaying && ( m_fVolume >= Maths::fEpsilon || m_fPrevVolume >= Maths::fEpsilon ) )
		bProcessing |= bProcess;

#ifndef CFG_GOLD
	if (!Generator::m_bProcessing && bProcessing)
	{
		Generator::m_uActiveWave++;
		Generator::m_uActiveGen++;
	}
	else if (Generator::m_bProcessing && !bProcessing)
	{
		Generator::m_uActiveWave--;
		Generator::m_uActiveGen--;
	}
#endif

	//	just stopped so set trail frames from dsp fx
	if( Generator::m_bProcessing && !bProcessing )
	{
		if( Generator::m_uTrailFrames > 0 )
		{
			Generator::m_uTrailFrames--;

			//	still has trailing frames so override procssing flag
			bProcessing = true;
		}
		else
		{
			//	no more trail frames so reset
			Generator::SetTrailFrames( info );
		}
	}

	Generator::m_bProcessing = bProcessing;

	return Generator::m_bProcessing;
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Wave::Process( const ProcessInfo& info )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.Wave.Process");

	if (Generator::m_uFrameCount != info.uFrameNum)
	{
		F32 fVolume;
		F32 fPrevVolume;

		if (m_bUseFastRamp)
		{
			//	volume uses audio frame length for ramp
			fVolume = m_fVolume;
			fPrevVolume = (info.uFrameMod == 0) ? m_fPrevVolume : m_fVolume;
		}
		else
		{
			//	volume uses component frame length for ramp
			fVolume = m_fPrevVolume + (info.fWidth + info.fFraction) * (m_fVolume - m_fPrevVolume);
			fPrevVolume = m_fPrevSubVolume;
		}

		if (Generator::m_uBuffer != (U32)-1)
		{
			F32* pFrameBuffers = (F32*)&info.pDynamicBuffers[Generator::m_uBuffer];

			//bool bActive = IsActive();

			F32* AUDIO_RESTRICT pfData = pFrameBuffers;

			Decoder* pDecoder = GetDecoder(info);

			//  started and has decoder
			if (m_bPlaying && pDecoder)
			{
				if (((fPrevVolume < Maths::fEpsilon) && (fVolume < Maths::fEpsilon)) || (m_bProcessing == false))
				{
					//  muted, clear frame
					Frame::Zero(pfData, Generator::m_u32NumChannels);

#ifdef AUDIO_NEW_DECODERS
					if ( m_pDecoder )
						m_pDecoder->GetDecodedFrame( NULL );
#else
					pDecoder->SkipBuffer(info);
#endif // AUDIO_NEW_DECODERS
					//bActive = false;
				}
				else
				{
					//  write data
#ifdef AUDIO_NEW_DECODERS
					if ( m_pDecoder )
					{
						m_pDecoder->GetDecodedFrame( pfData );

						//bActive = m_pDecoder->IsDecoding();
					}
#else
					pDecoder->FillBuffer(pfData, info);
#endif // AUDIO_NEW_DECODERS

					//	if this is last frame to be decoded, ramp to zero
					if (!pDecoder->IsActive(info.decoderDataInfo))
						fVolume = 0.f;

					//  apply gain
					if (/*bActive && */((fPrevVolume != 1.f) && (fVolume != 1.f)))
					{
						if (fPrevVolume != fVolume)
						{
							for (U32 uCh = 0; uCh < Generator::m_u32NumChannels; uCh++)
								Frame::GainRamp(&pfData[uCh * uFrameLength], &pfData[uCh * uFrameLength], fPrevVolume, fVolume);
						}
						else
						{
							for (U32 uCh = 0; uCh < Generator::m_u32NumChannels; uCh++)
								Frame::Gain(&pfData[uCh * uFrameLength], &pfData[uCh * uFrameLength], fVolume);
						}
					}
				}

				if (!pDecoder->IsActive(info.decoderDataInfo))
					m_bPlaying = false;
			}
			else
			{
				//  not started/no decoder so clear frame
				Frame::Zero(pfData, Generator::m_u32NumChannels);

				//bActive = false;
			}

			//  process effects
			Generator::Process( info );

#ifdef AUDIO_CHECK_SAMPLES
			for (U32 uCh = 0; uCh < Generator::m_u32NumChannels; uCh++)
			{
				if (!Frame::IsValid(&pfData[uCh * uFrameLength]))
					DebugBreak();
			}
#endif
		}
		else
			AUDIO_CORE_WARN("Wave '%s' not connected to audio graph", m_pchInfo);

		m_fPrevSubVolume = fVolume;

		if (info.uFrameMod == info.uFrameInt - 1)
			m_fPrevVolume = m_fVolume;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Start
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Wave::Start(const ProcessInfo& info)
{
	m_fPrevSubVolume = 0.f;
	
	Decoder* pDecoder = GetDecoder(info);
	if (pDecoder)
	{
		pDecoder->Init(info.decoderDataInfo);
		m_bPlaying = true;

		#if !defined CFG_GOLD && !defined PLATFORM_PS3 && !defined PLATFORM_SPU
		if (gbLogWavesWhenPlayed)
			AUDIO_CORE_DEBUG("Playing wave '%s'", m_pchInfo ? m_pchInfo : "unknown");
		#endif
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Stop
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Wave::Stop(const ProcessInfo& info)
{
	Decoder* pDecoder = GetDecoder(info);
	if (pDecoder)
		pDecoder->Shutdown(info.decoderDataInfo);

	//m_fVolume = 0.f;
    //m_bPlaying = false;
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Wave::Reset(const ProcessInfo& info, U32 uOffset)
{
	m_fPrevVolume = 0.f;
	Decoder* pDecoder = GetDecoder(info);
	if (pDecoder)
		pDecoder->Reset(info.decoderDataInfo, uOffset);
}

////////////////////////////////////////////////////////////////////////////////
//	SetDecoder
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Wave::SetDecoder(U32 uDecoderType, U32 uDecoder)
{
	m_uDecoderType = uDecoderType;
	m_uDecoder = uDecoder;
}

////////////////////////////////////////////////////////////////////////////////
//	DumpStats
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Wave::DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals)
{
    if (pInternals)
    {
        pInternals->uTotalNumGenerators++;
        pInternals->uTotalNumWaves++;

        if ( IsActive() )
        {
            pInternals->uNumGeneratorsActive++;
            pInternals->uNumWavesActive++;
        }
    }

	F32* pFrameData = (F32*)&info.pDynamicBuffers[Generator::m_uBuffer];

    if (Generator::m_pchInfo)
        AUDIO_SYS_DEBUG("%s[Wave %dch (%d fx)] \"%s\" <%s> F:0x%.8x (%.2f)", pchPrefix, Generator::m_u32NumChannels, m_uEffects, Generator::m_pchInfo, IsActive() ? "active" : "inactive", pFrameData, *pFrameData);
    else
        AUDIO_SYS_DEBUG("%s[Wave %dch (%d fx)] <%s> F:0x%.8x (%.2f)", pchPrefix, Generator::m_u32NumChannels, m_uEffects, IsActive() ? "active" : "inactive", pFrameData, *pFrameData);
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
