#include "AudioPch.h"
#include "coreheaders.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#endif

//using namespace Audio;
//using namespace Audio::Core;

#ifndef __STDC__
#define signed
#endif

#define USE_NEW_ADPCM

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


#ifdef PLATFORM_SPU
U32 umin(U32 a, U32 b) { return (a < b) ? a : b; }
#endif


#ifdef PLATFORM_PS3
#define BRANCH_HINT_LIKELY( condition )		__builtin_expect( condition, 1 )
#define BRANCH_HINT_UNLIKELY( condition )	__builtin_expect( condition, 0 )
#else
#define BRANCH_HINT_LIKELY( condition )		condition
#define BRANCH_HINT_UNLIKELY( condition )	condition
#endif


#define BATCH_SIZE_IN_SAMPLES 64
#define BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT 6
#define BATCH_SIZE_IN_BYTES 33
#define BATCH_SIZE_IN_BYTES_STEREO 66

//#define ADPCM_BREAK_ON_VALUE_CHECK
//#define ADPCM_SHUTDOWN_ON_ERROR

S32 Audio::Core::ADPCM4bitDecoder::ReadVagStreamMono( cDecodeVagInfo* decodeInfo, cDecodeVagWorkArea* workarea )
{
	F32 pcmBlockLeftData[ BATCH_SIZE_IN_SAMPLES ];
	F32* pcmBlockL;
	pcmBlockL = pcmBlockLeftData;

	U32 pcmSourcePositionCounter		= (U32)decodeInfo->mPcmReadOffset;
	U32 pcmSourcePositionLoopEndPos		= (U32)decodeInfo->mPCMLoopPosEnd;
	S32 pcmOutputSampleCount			= 0;
	S32 vagPositionInBytes				= (decodeInfo->mPcmReadOffset >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT);
	vagPositionInBytes					= (vagPositionInBytes << 5) + vagPositionInBytes;//		vagPositionInBytes *= BATCH_SIZE_IN_BYTES;
	S32 vagOffsetIntoPacketInSamples	= (decodeInfo->mPcmReadOffset & (BATCH_SIZE_IN_SAMPLES-1));
	S32 pcmOutputSampleCountL			= 0;
	const S32 vagFetchSize = ((BATCH_SIZE_IN_BYTES * 1) + 63) & ~63;

	if (BRANCH_HINT_LIKELY(pcmSourcePositionCounter < pcmSourcePositionLoopEndPos))
	{

		// maximum pitch is 4.0f
		decodeInfo->mPcmReadOffset			= decodeInfo->mPcmReadOffset;
		decodeInfo->mReachedEndOfStream		= false;
		// Fetch Vag Blocks
		U8 vagBlock[ vagFetchSize ] __attribute__((aligned(128)));
		while ( BRANCH_HINT_LIKELY(pcmOutputSampleCount < decodeInfo->mDestPCMSamples) )
		{
			S32 blockNr = vagPositionInBytes / BATCH_SIZE_IN_BYTES;
			if (BRANCH_HINT_UNLIKELY(workarea->mPrevBlockNr == blockNr))
			{
				workarea->mPreviousL[0] = workarea->mPreviousL[2];
				workarea->mPreviousL[1] = workarea->mPreviousL[3];
			}
			workarea->mPrevBlockNr = blockNr;

			U8* vagSourcePtr = decodeInfo->mSourceVag + vagPositionInBytes;

	#ifdef PLATFORM_PC
			U32 align = (U32)(U64)vagSourcePtr & 0xf;
			vagSourcePtr = (U8*)((U64)vagSourcePtr & ~0xf);
	#else
			U32 align = (U32)vagSourcePtr & 0xf;
			vagSourcePtr = (U8*)((U32)vagSourcePtr & ~0xf);
	#endif
			Spu::Fetch( vagBlock, vagSourcePtr, vagFetchSize );

			workarea->mPreviousL[2] = workarea->mPreviousL[0];
			workarea->mPreviousL[3] = workarea->mPreviousL[1];
			if (BRANCH_HINT_UNLIKELY(!DecodeBlock3( vagBlock + align, pcmBlockL, &workarea->mPreviousL[0], &workarea->mPreviousL[1], &workarea->mDeltaFader )))
			{
				decodeInfo->mAbortStream = true;
				return 0;
			}
			U32 pcmIndex = vagOffsetIntoPacketInSamples;
			vagOffsetIntoPacketInSamples = 0;
			for (; pcmIndex < BATCH_SIZE_IN_SAMPLES ; pcmIndex++)
			{
				pcmSourcePositionCounter ++;
				decodeInfo->mSamplesConsumed ++;
				F32 left  = pcmBlockL[ pcmIndex ];
				decodeInfo->mpDestPCML[ pcmOutputSampleCountL ] = left;
				pcmOutputSampleCountL++;
				pcmOutputSampleCount++;
				
				if (BRANCH_HINT_UNLIKELY(pcmOutputSampleCount >= decodeInfo->mDestPCMSamples))
				{
					break;
				}
				if (BRANCH_HINT_UNLIKELY(pcmSourcePositionCounter >= pcmSourcePositionLoopEndPos))
				{
					if (BRANCH_HINT_UNLIKELY(decodeInfo->mLoop))
					{
						vagPositionInBytes				= (decodeInfo->mPCMLoopPosStart >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT);
						vagPositionInBytes				= (vagPositionInBytes << 5) + vagPositionInBytes;//		vagPositionInBytes *= BATCH_SIZE_IN_BYTES;
						vagOffsetIntoPacketInSamples	= (decodeInfo->mPCMLoopPosStart & (BATCH_SIZE_IN_SAMPLES-1));
						vagPositionInBytes -= BATCH_SIZE_IN_BYTES; // added back on below when 'broken' out of for-loop
						pcmSourcePositionCounter = decodeInfo->mPCMLoopPosStart;
						workarea->ClearPrevious();
						break;
					}
					else
					{
						goto exit;
					}
				}
			}
			vagPositionInBytes += BATCH_SIZE_IN_BYTES;
		}
	}
exit:	
	for (U32 x = pcmOutputSampleCount ; x < decodeInfo->mDestPCMSamples ; x++)
	{
		decodeInfo->mpDestPCML[ x ] = 0;
	}
	return pcmSourcePositionCounter;
}



S32 Audio::Core::ADPCM4bitDecoder::ReadVagStreamStereo( cDecodeVagInfo* decodeInfo, cDecodeVagWorkArea* workarea )
{
	F32 pcmBlockLeftData[ BATCH_SIZE_IN_SAMPLES ];
	F32 pcmBlockRightData[ BATCH_SIZE_IN_SAMPLES ];
	F32* pcmBlockL = pcmBlockLeftData;
	F32* pcmBlockR = pcmBlockRightData; 


	U32 pcmSourcePositionCounter		= (U32)decodeInfo->mPcmReadOffset;
	U32 pcmSourcePositionLoopEndPos		= (U32)decodeInfo->mPCMLoopPosEnd;
	S32 pcmOutputSampleCount			= 0;
	S32 vagPositionInBytes				= (decodeInfo->mPcmReadOffset >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT);
	vagPositionInBytes					= (( vagPositionInBytes << 5 ) + vagPositionInBytes) << 1; // * 66
	S32 vagOffsetIntoPacketInSamples	= (decodeInfo->mPcmReadOffset & (BATCH_SIZE_IN_SAMPLES-1));
	S32 pcmOutputSampleCountL			= 0;
	const S32 vagFetchSize = ((BATCH_SIZE_IN_BYTES * 2) + 63) & ~63;

	if (BRANCH_HINT_LIKELY(pcmSourcePositionCounter < pcmSourcePositionLoopEndPos))
	{
		// maximum pitch is 4.0f
		decodeInfo->mPcmReadOffset			= (decodeInfo->mPcmReadOffset);
		decodeInfo->mReachedEndOfStream		= false;
		// Fetch Vag Blocks
		U8 vagBlock[ vagFetchSize ] __attribute__((aligned(128)));
		while ( BRANCH_HINT_LIKELY(pcmOutputSampleCount < decodeInfo->mDestPCMSamples))
		{
			S32 blockNr = vagPositionInBytes / BATCH_SIZE_IN_BYTES_STEREO;
			if (BRANCH_HINT_UNLIKELY(workarea->mPrevBlockNr == blockNr))
			{
				workarea->mPreviousL[0] = workarea->mPreviousL[2];
				workarea->mPreviousL[1] = workarea->mPreviousL[3];
				workarea->mPreviousR[0] = workarea->mPreviousR[2];
				workarea->mPreviousR[1] = workarea->mPreviousR[3];
			}
			workarea->mPrevBlockNr = blockNr;

			U8* vagSourcePtr = decodeInfo->mSourceVag + vagPositionInBytes;

	#ifdef PLATFORM_PC
			U32 align = (U32)(U64)vagSourcePtr & 0xf;
			vagSourcePtr = (U8*)((U64)vagSourcePtr & ~0xf);
	#else
			U32 align = (U32)vagSourcePtr & 0xf;
			vagSourcePtr = (U8*)((U32)vagSourcePtr & ~0xf);
	#endif
			Spu::Fetch( vagBlock, vagSourcePtr, vagFetchSize );

			workarea->mPreviousL[2] = workarea->mPreviousL[0];
			workarea->mPreviousL[3] = workarea->mPreviousL[1];
			workarea->mPreviousR[2] = workarea->mPreviousR[0];
			workarea->mPreviousR[3] = workarea->mPreviousR[1];
			
			F32 deltaFader = workarea->mDeltaFader;
			bool decodeResultL = DecodeBlock3( vagBlock + align, pcmBlockL, &workarea->mPreviousL[0], &workarea->mPreviousL[1], &deltaFader );
			bool decodeResultR = DecodeBlock3( vagBlock + align + BATCH_SIZE_IN_BYTES, pcmBlockR, &workarea->mPreviousR[0], &workarea->mPreviousR[1], &workarea->mDeltaFader );
			if (BRANCH_HINT_UNLIKELY(!(decodeResultL && decodeResultR)))
			{
				decodeInfo->mAbortStream = true;
				return 0;
			}
			U32 pcmIndex = vagOffsetIntoPacketInSamples;
			vagOffsetIntoPacketInSamples = 0;
			for (; pcmIndex < BATCH_SIZE_IN_SAMPLES ; pcmIndex++)
			{
				pcmSourcePositionCounter ++;
				decodeInfo->mSamplesConsumed ++;
				F32 left  = pcmBlockL[ pcmIndex ];
				F32 right = pcmBlockR[ pcmIndex ];

				decodeInfo->mpDestPCML[ pcmOutputSampleCountL ] = left; 
				decodeInfo->mpDestPCMR[ pcmOutputSampleCountL ] = right;
				pcmOutputSampleCountL++;
				
				pcmOutputSampleCount++;
				
				if (BRANCH_HINT_UNLIKELY(pcmOutputSampleCount >= decodeInfo->mDestPCMSamples))
				{
					break;
				}
				if (BRANCH_HINT_UNLIKELY(pcmSourcePositionCounter >= pcmSourcePositionLoopEndPos))
				{
					if (BRANCH_HINT_UNLIKELY(decodeInfo->mLoop))
					{
						vagPositionInBytes				= (decodeInfo->mPCMLoopPosStart >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT);
						vagPositionInBytes				= (( vagPositionInBytes << 5 ) + vagPositionInBytes) << 1; 
						vagOffsetIntoPacketInSamples	= (decodeInfo->mPCMLoopPosStart & (BATCH_SIZE_IN_SAMPLES-1));
						vagPositionInBytes -= BATCH_SIZE_IN_BYTES_STEREO; // added back on below when 'broken' out of for-loop
						pcmSourcePositionCounter = decodeInfo->mPCMLoopPosStart;
						workarea->ClearPrevious();
						break;
					}
					else
					{
						// end of data
						goto exit;
					}
				}
			}
			vagPositionInBytes += BATCH_SIZE_IN_BYTES_STEREO;
		}
	}
exit:
	for (U32 x = pcmOutputSampleCount ; x < (U32)decodeInfo->mDestPCMSamples ; x++)
	{
		decodeInfo->mpDestPCML[ x ] = decodeInfo->mpDestPCMR[ x ] = 0;
	}
	return pcmSourcePositionCounter;
}

Audio::Core::ADPCM4bitDecoder::ADPCM4bitDecoder(bool bLoop, bool bResample)
{
	m_baseState.bLoop = bLoop;
	m_baseState.bResample = bResample;
    m_baseState.uSampleIndex = 0;
    m_baseState.uSampleCounter = 0;
    m_baseState.fPrevPitch = 1.f;
    m_baseState.fPitch = 1.f;
    m_baseState.fNewPitch = 1.f;
    m_baseState.m_fA = 0.f;
    m_baseState.m_fB = 0.f;
	m_baseState.bPlaying = false;
	m_baseState.bTriggerCallback = false;

	m_eType = uADPCM4Bit;
	m_bPrevActive = false;
	m_bCallbackFired = false;

	m_baseState.workarea.ClearPrevious();	
}


Audio::Core::ADPCM4bitDecoder::~ADPCM4bitDecoder()
{
}

F32 Audio::Core::ADPCM4bitDecoder::GetRescaledPitch(F32 f32Pitch)
{
	F32 fRescaled = f32Pitch * (F32)m_waveData.uSampleRate * fRcpSampleRate;

    //  max shift up +2 oct above sample rate
    if (fRescaled > 4.f)
        fRescaled = 4.f;

    //  max shift down -8 oct above sample rate
    else if (fRescaled < 0.00390625f)
        fRescaled = 0.00390625f;

	return fRescaled;
}


////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::Init(const DecoderDataInfo& info)
{
	m_baseState.m_fB = Maths::ArcTan(Maths::fPi * 0.005f);
	m_baseState.m_fB = -(m_baseState.m_fB - 1.0f) * Maths::RcpEst(1.0f + m_baseState.m_fB);
	m_baseState.m_fA = 0.5f * (1.0f - m_baseState.m_fB);

	m_baseState.bPlaying = true;
	m_baseState.bPaused = false;

	ADPCM4bitDecoder::m_uTotalADPCM++;

	m_baseState.fPrevPitch = GetRescaledPitch(1.f);
	m_baseState.fPitch = GetRescaledPitch(1.f);
	m_baseState.workarea.ClearPrevious();

#ifndef PLATFORM_SPU
	U8 * __restrict asDataIn = (U8*)m_waveData.psData;
	if (!asDataIn) return;
#else
	m_bInitedWave = false;
//	m_baseState.headerSize = 0;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	Shutdown
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::Shutdown(const DecoderDataInfo& info)
{
//	if (GetStreamHandle() != -1)
//	{
//#ifdef USER_JONC
//#ifdef PLATFORM_PS3
////printf("in ADPCM4bitDecoder::Shutdown(), shutting down stream %d\n", m_waveData.hStream.u32Handle);
//#endif
//#endif
//		Audio::System::Get()->GetWaveServer()->ReleaseStream(GetStreamHandle());
//	}

	if (m_waveData.hWaveServerIndex.IsValid() && GetWaveData(info)) GetWaveData(info)->SetPlaying(false, (U32*)(info.pWaveServerData));

	BecomeInactive(info);
	m_baseState.bPlaying = false;
	ReleaseResources(info);
}

////////////////////////////////////////////////////////////////////////////////
//	BecomeActive
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::BecomeActive(const DecoderDataInfo& info)
{
}

////////////////////////////////////////////////////////////////////////////////
//	BecomeInactive
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::BecomeInactive(const DecoderDataInfo& info)
{

	//if (m_waveData.hWaveServerIndex.IsValid())
	//	SetPlaying(info, false);

	ADPCM4bitDecoder::m_uTotalADPCM--;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseResources
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::ReleaseResources(const DecoderDataInfo& info)
{
}

////////////////////////////////////////////////////////////////////////////////
//	Position
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Core::ADPCM4bitDecoder::Position(const DecoderDataInfo& info)
{
	// Gets the position expressed 0.0f to 1.0f
	if (m_waveData.uChannels)
	{
		U32 nrSamples = GetNumSamples();
		if (nrSamples)
		{
			F32 fNumSamples = (F32)GetNumSamples() / m_waveData.uChannels;
			F32 fSampleCounter = (F32)(m_baseState.uSampleCounter % nrSamples);
			return fSampleCounter / fNumSamples;
		}
	}
	return 0.f;
}
////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::Reset(const DecoderDataInfo& info, U32 uOffset)
{
	if (m_waveData.psData)
	    SetPosition(uOffset);
	else if (m_waveData.hWaveServerIndex.IsValid())
		GetWaveData(info)->SetSourceDataPosition(0, (U32*)(info.pWaveServerData));

#ifdef ADPCM_BREAK_ON_VALUE_CHECK
	if ((S32)uOffset < 0) DebugBreak();
#endif
	m_baseState.workarea.ClearPrevious();
	m_baseState.uSampleIndex = uOffset;
	m_baseState.uSampleCounter = uOffset;
	m_baseState.fPitch = m_baseState.fNewPitch;
	m_baseState.fPrevPitch = m_baseState.fPitch;

	m_baseState.bPaused = false;
}

////////////////////////////////////////////////////////////////////////////////
//	SetPitch
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::SetPitch(F32 fPitch)
{
	m_baseState.fNewPitch = GetRescaledPitch(fPitch);
}
////////////////////////////////////////////////////////////////////////////////
//	Finished
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::ADPCM4bitDecoder::Finished(const DecoderDataInfo& info)
{
	// Has the sample finished i.e. play-position >= number of samples.
    S32 sNumSamples = GetNumSamples() / m_waveData.uChannels;

    if ((m_baseState.uSampleCounter) >= (U32)sNumSamples)
        return true;
    else
        return false;
}
////////////////////////////////////////////////////////////////////////////////
//	SetPosition
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::SetPosition(U32 uPosition)
{
	// Set the play-position
    S32 sNumSamples = GetNumSamples();
#ifdef ADPCM_BREAK_ON_VALUE_CHECK
	if ((S32)uPosition < 0) DebugBreak();
#endif
	if (uPosition < (U32)sNumSamples)
	{
		m_baseState.uSampleIndex = uPosition;
		m_baseState.uSampleCounter = uPosition;
		m_baseState.workarea.ClearPrevious();	
	}
		
}
////////////////////////////////////////////////////////////////////////////////
//	UpdateFilterCoeffs
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::UpdateFilterCoeffs(F32 fPitch, const ProcessInfo& info)
{
	//	only update filter if using SRC and LP
	if (m_baseState.bResample)
	{
		//	pitch changing across subframes or new subframe set, recalc filter coeffs
		if ((m_baseState.fPitch != m_baseState.fPrevPitch) ||
			(info.uFrameMod == 0))
		{
			F32 fCutoff;
			if (fPitch > 0.5f)
				fCutoff = 0.5f / fPitch;
			else
				fCutoff = 1.f;

			m_baseState.m_fB = Maths::ArcTan(Maths::fPi * fCutoff);
			m_baseState.m_fB = -(m_baseState.m_fB - 1.0f) * Maths::RcpEst(1.0f + m_baseState.m_fB);
			m_baseState.m_fA = 0.5f * (1.0f - m_baseState.m_fB);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	GetNumSamples
////////////////////////////////////////////////////////////////////////////////
S32 Audio::Core::ADPCM4bitDecoder::GetNumSamples()
{
	return m_waveData.uPCMLoopPosEnd;
}

////////////////////////////////////////////////////////////////////////////////
//	PostUpdatePitch
////////////////////////////////////////////////////////////////////////////////
F32 Audio::Core::ADPCM4bitDecoder::GetSubFramePitch(const ProcessInfo& info)
{
    //  scale pitch by sample rate ratio
    F32 fPitch = m_baseState.fPrevPitch;

    //  interpolate between audio subframes
	fPitch += info.fWidth * (m_baseState.fPitch - m_baseState.fPrevPitch);

	if (fPitch > 4.0f)
	{
		fPitch = 4.0f;
	} else if (fPitch < 0.0078125f )
	{
		fPitch = 0.0078125f;
	}

	return fPitch;
}
////////////////////////////////////////////////////////////////////////////////
//	PostUpdatePitch
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::PostUpdatePitch(const ProcessInfo& info)
{
	//	 if last sub frame, shift along pitch values
	if (info.uFrameMod == info.uFrameInt - 1)
	{
		m_baseState.fPrevPitch = m_baseState.fPitch;
		m_baseState.fPitch = m_baseState.fNewPitch;
	}
}
////////////////////////////////////////////////////////////////////////////////
//	CrossingBoundary
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ADPCM4bitDecoder::CrossingBoundary(const ProcessInfo& info, S32 sNumSamples)
{
	//  fire callback
	if (m_baseState.bTriggerCallback)
		m_bCallbackFired = true;


	if (/*GetStreamHandle() != -1 && */(! m_baseState.bLoop))
		Shutdown(info.decoderDataInfo);

}

////////////////////////////////////////////////////////////////////////////////
//	IsDataInAvailable
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::ADPCM4bitDecoder::IsDataInAvailable(const DecoderDataInfo& info)
{
	// for streaming data- if no data in available, return a buffer full of 0's
	if (! m_waveData.hWaveServerIndex.IsValid())
		return true;

	WaveStreamData* pWaveStreamData = GetWaveData(info);

	// GetBytesConsumed() a bit of a hack to deal with when we've already requested data in GetData() and data available has been set to 0
	return (pWaveStreamData->IsPrimed() && (pWaveStreamData->GetBytesAvailable() || pWaveStreamData->GetBytesConsumed()));
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::MonoADPCM4bitDecoder::Init(const DecoderDataInfo& info)
{
	m_state.fPrevSample = 0.f;

	m_state.fPrevIn = 0.f;
	m_state.fPrevOut = 0.f;

	ADPCM4bitDecoder::Init(info);
	m_baseState.workarea.ClearPrevious();	

}

////////////////////////////////////////////////////////////////////////////////
//	Shutdown
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::MonoADPCM4bitDecoder::Shutdown(const DecoderDataInfo& info)
{
	ADPCM4bitDecoder::Shutdown(info);
}

////////////////////////////////////////////////////////////////////////////////
//	FillBuffer
////////////////////////////////////////////////////////////////////////////////
U8* Audio::Core::ADPCM4bitDecoder::GetData( const DecoderDataInfo& info )
{
	if (m_waveData.psData)
	{
		return (U8*)(m_waveData.psData);
	}
	else
	{
		if (m_waveData.hWaveServerIndex.IsValid())
		{
			if (GetWaveData(info)->IsPrimed())
			{
				WaveStreamData* pWaveStreamData = GetWaveData(info);
				if (pWaveStreamData->GetBytesAvailable())
				{
					U8* ppuAddr = pWaveStreamData->GetData(0, (U32*)info.pWaveServerData);
					if (!ppuAddr) return NULL;
		
#ifdef ADPCM_BREAK_ON_VALUE_CHECK
					U8* p = ppuAddr;
					U32 align = (U32)(U64)p & 0xf;
					p = (U8*)((U64)p & ~0xf);
					
					U8 temp[ 512 ] __attribute__((aligned(128)));
					Spu::Fetch(temp,p,sizeof(temp));
					for (U32 x = 0 ; x < 512 ; x+=33)
					{
						if (((temp[x+align] & 0xf0) >> 4) > 4)
						{
							DebugBreak();
							break;
						}
					}
#endif
					m_baseState.uSampleIndex = m_baseState.uSampleIndex & ((BATCH_SIZE_IN_SAMPLES)-1);
					if (m_baseState.uSampleIndex == 0)
					{
						m_baseState.workarea.mPrevBlockNr = -1;
					}
					else
					{
						m_baseState.workarea.mPrevBlockNr = 0;
					}
					return ppuAddr;
				}
			}
			else
			{
				GetWaveData(info)->Prime((U32*)(info.pWaveServerData));
				return 0;
			}
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
//	FillBuffer
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::MonoADPCM4bitDecoder::FillBuffer(F32* afData, const ProcessInfo& info)
{
#ifdef ADPCM_BREAK_ON_VALUE_CHECK
	if ((!m_waveData.hWaveServerIndex.IsValid()) && (m_baseState.uSampleCounter != m_baseState.uSampleIndex)) { DebugBreak(); }
#endif	
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.MonoADPCM4bitDecoder.FillBuffer");

	bool bActive = true;

	//  get subframe pitch
    F32 fPitch = ADPCM4bitDecoder::GetSubFramePitch(info);
#ifdef PLATFORM_PC
    fPitch *= fOutputRateRatio;	// [Blur2PC] T144: retail scales by the output rate (see audio_core.h)
#endif
	//	recalc filter coeffs, if req
//	ADPCM4bitDecoder::UpdateFilterCoeffs(fPitch, info);

	if(!m_baseState.bPaused)
	{
		cDecodeVagInfo	decodeInfo;
		decodeInfo.mSourceVag		= GetData( info.decoderDataInfo );

		WaveStreamData* pWaveStreamData = GetWaveData(info.decoderDataInfo );
		U32 uSamplesRequired = (U32)((fPitch * fFrameLength) + 0.5f);
		if (m_waveData.hWaveServerIndex.IsValid())
		{
			bool zeroAndReturn = false;
			U32 uBytesRequired = (uSamplesRequired >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT) * BATCH_SIZE_IN_BYTES;
			U32 nrBytesAvailable  = pWaveStreamData->GetBytesAvailable();
			U32 nrSamplesToEnd = m_waveData.uPCMLoopPosEnd - m_baseState.uSampleCounter;
			U32 nrBytesToEnd = (nrSamplesToEnd >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT) * BATCH_SIZE_IN_BYTES;
			if (nrBytesToEnd < uBytesRequired) 
			{
				uBytesRequired = nrBytesToEnd;
			}

			if (!IsDataInAvailable(info.decoderDataInfo)) zeroAndReturn = true;
			else if (nrBytesAvailable < uBytesRequired)
			{
				{
					zeroAndReturn = true;
				}
			}
		
			if (zeroAndReturn)
			{
				for (U32 i = 0; i < uFrameLength; i++)
					afData[i] = 0.f;
				return true;
			}
		}
		
		F32 afShiftBufferL[(uFrameLength + 1) * 4];		//	1 extra sample, stereo, 2 oct up
		F32* afRenderBufferL = &afShiftBufferL[1];

#ifdef ADPCM_BREAK_ON_VALUE_CHECK
		if ((S32)m_baseState.uSampleIndex < 0) DebugBreak();
#endif

		decodeInfo.mVagSizeInBytes	= m_waveData.uLength;
		decodeInfo.mPcmReadOffset	= m_baseState.uSampleIndex;
		decodeInfo.mpDestPCML		= afRenderBufferL;
		decodeInfo.mpDestPCMR		= afRenderBufferL;
		decodeInfo.mbStereo			= true;
		decodeInfo.mDestPCMSamples	= uSamplesRequired;
		decodeInfo.mLoop			= m_baseState.bLoop;
		// WARNING : loop end in header has been Rounded up to the nearest pair.
		decodeInfo.mPCMLoopPosEnd	= m_waveData.uPCMLoopPosEnd;
		decodeInfo.mPCMLoopPosStart = 0;
		decodeInfo.mSamplesConsumed = 0;
		decodeInfo.mAbortStream	    = false;
		
		afShiftBufferL[ 0 ] = m_state.fPrevSample;
		decodeInfo.mPCMLoopPosEnd	= m_waveData.uPCMLoopPosEnd;

		U32 uSampleIndex = m_baseState.uSampleIndex;
		m_baseState.uSampleIndex = ReadVagStreamMono( &decodeInfo, &m_baseState.workarea );
		m_baseState.uSampleCounter += (decodeInfo.mSamplesConsumed);
		m_state.fPrevSample = afRenderBufferL[ uSamplesRequired - 1 ];

#ifdef ADPCM_SHUTDOWN_ON_ERROR
		if (decodeInfo.mAbortStream)
		{
			Shutdown(info.decoderDataInfo);
			for (U32 i = 0; i < uFrameLength; i++)
				afData[i] = 0.f;
			return false;
			
		}
#endif	
		Resample(fPitch, uSamplesRequired, afShiftBufferL, afData );
		
		if (m_baseState.uSampleCounter >= m_waveData.uPCMLoopPosEnd)
		{
			if (!m_baseState.bLoop)
			{
				CrossingBoundary(info, 0);
				bActive = false;
			}
			else
			{
				m_baseState.uSampleCounter %= m_waveData.uPCMLoopPosEnd;
				m_baseState.uSampleIndex = m_baseState.uSampleCounter;
			}
		}

#ifdef ADPCM_BREAK_ON_VALUE_CHECK
		if ((!m_waveData.hWaveServerIndex.IsValid()) && (m_baseState.uSampleCounter != m_baseState.uSampleIndex)) { DebugBreak(); }
#endif
		if (m_waveData.hWaveServerIndex.IsValid())
		{
			S32 consumedBytes = (((decodeInfo.mSamplesConsumed + uSampleIndex) >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT) * BATCH_SIZE_IN_BYTES);
			GetWaveData(info.decoderDataInfo)->GetData( consumedBytes, (U32*)info.decoderDataInfo.pWaveServerData );
		}
	}
	else
	{
		BecomeInactive(info.decoderDataInfo);
		ReleaseResources(info.decoderDataInfo);
	}

	ADPCM4bitDecoder::PostUpdatePitch(info);

	return bActive;
}



////////////////////////////////////////////////////////////////////////////////
//	SkipBuffer
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::MonoADPCM4bitDecoder::SkipBuffer(const ProcessInfo& info)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.MonoADPCM4bitDecoder.SkipBuffer");

	if (m_waveData.hWaveServerIndex.IsValid())
	{
		return;
	}

#ifdef ADPCM_BREAK_ON_VALUE_CHECK
	if (m_baseState.uSampleCounter != m_baseState.uSampleIndex) {DebugBreak();}
#endif
	//  get subframe pitch
    F32 fPitch = ADPCM4bitDecoder::GetSubFramePitch(info);
#ifdef PLATFORM_PC
    fPitch *= fOutputRateRatio;	// [Blur2PC] T144: retail scales by the output rate (see audio_core.h)
#endif

	if(!m_baseState.bPaused)
	{
		// for streaming data- if no data in available, return a buffer full of 0's
		if (! IsDataInAvailable(info.decoderDataInfo))
			return;
			
		S32 bytePosB4 = (((m_baseState.uSampleCounter)>>BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT) * BATCH_SIZE_IN_BYTES);
		S32 index = (S32)(fPitch * (F32)uFrameLength);
		m_baseState.uSampleIndex += index;
		m_baseState.uSampleCounter += index;
		S32 sampleCounter = m_baseState.uSampleCounter;
		if (m_baseState.uSampleCounter >= (S32)m_waveData.uPCMLoopPosEnd)
		{
			if (m_waveData.bLoop)
			{
				m_baseState.uSampleCounter %= m_waveData.uPCMLoopPosEnd;
				m_baseState.uSampleIndex = m_baseState.uSampleCounter;
			}
			else
			{
				CrossingBoundary(info, 0);
				m_baseState.uSampleCounter = m_waveData.uPCMLoopPosEnd;
				m_baseState.uSampleIndex = m_baseState.uSampleCounter;
			}
		}

		// Streaming
		if (m_waveData.hWaveServerIndex.IsValid())
		{
			S32 consumedBytes = (((sampleCounter) >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT) * BATCH_SIZE_IN_BYTES) - bytePosB4;
			GetWaveData(info.decoderDataInfo)->OverrideBytesConsumed( consumedBytes );
		}
		m_baseState.workarea.ClearPrevious();	
	}
	
#ifdef ADPCM_BREAK_ON_VALUE_CHECK
	if (m_baseState.uSampleCounter != m_baseState.uSampleIndex) { DebugBreak(); }
#endif
}


////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::StereoADPCM4bitDecoder::Init(const DecoderDataInfo& info)
{
	m_state.fPrevSampleL = 0.f;

	m_state.fPrevInL = 0.f;
	m_state.fPrevOutL = 0.f;

	m_state.fPrevSampleR = 0.f;

	m_state.fPrevInR = 0.f;
	m_state.fPrevOutR = 0.f;
	ADPCM4bitDecoder::Init(info);

	m_baseState.workarea.ClearPrevious();	

}

////////////////////////////////////////////////////////////////////////////////
//	Shutdown
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::StereoADPCM4bitDecoder::Shutdown(const DecoderDataInfo& info)
{
	ADPCM4bitDecoder::Shutdown(info);
}

////////////////////////////////////////////////////////////////////////////////
//	FillBuffer
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::StereoADPCM4bitDecoder::FillBuffer(F32* afData, const ProcessInfo& info)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.StereoADPCM4bitDecoder.FillBuffer");

	bool bActive = true;
	
	//  get subframe pitch
    F32 fPitch = ADPCM4bitDecoder::GetSubFramePitch(info);
#ifdef PLATFORM_PC
    fPitch *= fOutputRateRatio;	// [Blur2PC] T144: retail scales by the output rate (see audio_core.h)
#endif
	//	recalc filter coeffs, if req
//	ADPCM4bitDecoder::UpdateFilterCoeffs(fPitch, info);

	if(!m_baseState.bPaused)
	{
		cDecodeVagInfo	decodeInfo;
		decodeInfo.mSourceVag		= GetData( info.decoderDataInfo );

		WaveStreamData* pWaveStreamData = GetWaveData(info.decoderDataInfo );
		U32 uSamplesRequired = (U32)((fPitch * fFrameLength) + 0.5f);
		if (m_waveData.hWaveServerIndex.IsValid())
		{
			bool zeroAndReturn = false;
			U32 uBytesRequired = (uSamplesRequired >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT) * BATCH_SIZE_IN_BYTES_STEREO;
			U32 nrBytesAvailable  = pWaveStreamData->GetBytesAvailable();
			U32 nrSamplesToEnd = m_waveData.uPCMLoopPosEnd - m_baseState.uSampleCounter;
			U32 nrBytesToEnd = (nrSamplesToEnd >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT) * BATCH_SIZE_IN_BYTES_STEREO;
			if (nrBytesToEnd < uBytesRequired) 
			{
				uBytesRequired = nrBytesToEnd;
			}

			if (!IsDataInAvailable(info.decoderDataInfo)) zeroAndReturn = true;
			else if (nrBytesAvailable < uBytesRequired)
			{
				{
					zeroAndReturn = true;
				}
			}
			
			if (zeroAndReturn)
			{
				for (U32 i = 0; i < uFrameLength * 2; i++)
					afData[i] = 0.f;
				return true;
			}
		}
		
		F32 afShiftBufferL[(uFrameLength + 1) * 4];		//	1 extra sample, stereo, 2 oct up
		F32 afShiftBufferR[(uFrameLength + 1) * 4];		//	1 extra sample, stereo, 2 oct up
		F32* afRenderBufferL = &afShiftBufferL[1];
		F32* afRenderBufferR = &afShiftBufferR[1];

#ifdef ADPCM_BREAK_ON_VALUE_CHECK
		if ((S32)m_baseState.uSampleIndex < 0) DebugBreak();
#endif
		decodeInfo.mVagSizeInBytes	= m_waveData.uLength;
		decodeInfo.mPcmReadOffset	= m_baseState.uSampleIndex;
		decodeInfo.mpDestPCML		= afRenderBufferL;
		decodeInfo.mpDestPCMR		= afRenderBufferR;
		decodeInfo.mbStereo			= true;
		decodeInfo.mDestPCMSamples	= uSamplesRequired;
		decodeInfo.mLoop			= m_baseState.bLoop;
		// WARNING : loop end in header has been Rounded up to the nearest pair.
		decodeInfo.mPCMLoopPosEnd	= m_waveData.uPCMLoopPosEnd;
		decodeInfo.mPCMLoopPosStart = 0;
		decodeInfo.mSamplesConsumed = 0;
		decodeInfo.mAbortStream		= false;
		
		afShiftBufferL[ 0 ] = m_state.fPrevSampleL;
		afShiftBufferR[ 0 ] = m_state.fPrevSampleR;
		
		decodeInfo.mPCMLoopPosEnd	= m_waveData.uPCMLoopPosEnd;

		U32 uSampleIndex = m_baseState.uSampleIndex;
		m_baseState.uSampleIndex = ReadVagStreamStereo( &decodeInfo, &m_baseState.workarea );
		m_baseState.uSampleCounter += (decodeInfo.mSamplesConsumed);
		m_state.fPrevSampleL = afRenderBufferL[ uSamplesRequired - 1 ];
		m_state.fPrevSampleR = afRenderBufferR[ uSamplesRequired - 1 ];

#ifdef ADPCM_SHUTDOWN_ON_ERROR
		if (decodeInfo.mAbortStream)
		{
			Shutdown(info.decoderDataInfo);
			for (U32 i = 0; i < uFrameLength * 2; i++)
				afData[i] = 0.f;
			return false;
			
		}
#endif

		Resample(fPitch, uSamplesRequired, afShiftBufferL, afShiftBufferR, afData, afData+ uFrameLength);
		
		if (m_baseState.uSampleCounter >= m_waveData.uPCMLoopPosEnd)
		{
			if (!m_baseState.bLoop)
			{
				CrossingBoundary(info, 0);
				bActive = false;
			}
			else
			{
				m_baseState.uSampleCounter %= m_waveData.uPCMLoopPosEnd;
				m_baseState.uSampleIndex = m_baseState.uSampleCounter;
			}
		}
		
#ifdef ADPCM_BREAK_ON_VALUE_CHECK
		if ((!m_waveData.hWaveServerIndex.IsValid()) && (m_baseState.uSampleCounter != m_baseState.uSampleIndex)) { DebugBreak(); }
#endif
		if (m_waveData.hWaveServerIndex.IsValid())
		{
			S32 consumedBytes = (((decodeInfo.mSamplesConsumed + uSampleIndex) >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT) * BATCH_SIZE_IN_BYTES);
			GetWaveData(info.decoderDataInfo)->GetData( consumedBytes<<1, (U32*)info.decoderDataInfo.pWaveServerData );
		}
	}
	else
	{
		BecomeInactive(info.decoderDataInfo);
		ReleaseResources(info.decoderDataInfo);
	}

	ADPCM4bitDecoder::PostUpdatePitch(info);

	return bActive;
}



////////////////////////////////////////////////////////////////////////////////
//	SkipBuffer
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::StereoADPCM4bitDecoder::SkipBuffer(const ProcessInfo& info)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.StereoADPCM4bitDecoder.SkipBuffer");

	if (m_waveData.hWaveServerIndex.IsValid())
	{
		return;
	}

#ifdef ADPCM_BREAK_ON_VALUE_CHECK
	if (m_baseState.uSampleCounter != m_baseState.uSampleIndex) { DebugBreak(); }
#endif
	//  get subframe pitch
    F32 fPitch = ADPCM4bitDecoder::GetSubFramePitch(info);
#ifdef PLATFORM_PC
    fPitch *= fOutputRateRatio;	// [Blur2PC] T144: retail scales by the output rate (see audio_core.h)
#endif

	if(!m_baseState.bPaused)
	{
		// for streaming data- if no data in available, return a buffer full of 0's
		if (! IsDataInAvailable(info.decoderDataInfo))
			return;

		S32 bytePosB4 = (((m_baseState.uSampleCounter)>>BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT) * BATCH_SIZE_IN_BYTES);
		S32 index = (S32)(fPitch * (F32)uFrameLength);
		m_baseState.uSampleIndex += index;
		m_baseState.uSampleCounter += index;
		S32 sampleCounter = m_baseState.uSampleCounter;
		if (m_baseState.uSampleCounter >= (S32)m_waveData.uPCMLoopPosEnd)
		{
			if (m_waveData.bLoop)
			{
				m_baseState.uSampleCounter %= m_waveData.uPCMLoopPosEnd;	
				m_baseState.uSampleIndex = m_baseState.uSampleCounter;
			}
			else
			{
				CrossingBoundary(info, 0);
				m_baseState.uSampleCounter = m_waveData.uPCMLoopPosEnd;	
				m_baseState.uSampleIndex = m_baseState.uSampleCounter;
			}
		}

		// Streaming
		if (m_waveData.hWaveServerIndex.IsValid())
		{
			S32 consumedBytes = (((sampleCounter) >> BATCH_SIZE_IN_SAMPLES_DIVIDE_SHIFT) * BATCH_SIZE_IN_BYTES_STEREO) - bytePosB4;
			GetWaveData(info.decoderDataInfo)->OverrideBytesConsumed( consumedBytes );
		}
		m_baseState.workarea.ClearPrevious();	
	}

#ifdef ADPCM_BREAK_ON_VALUE_CHECK
	if (m_baseState.uSampleCounter != m_baseState.uSampleIndex) { DebugBreak(); }
#endif
}

__forceinline bool Audio::Core::ADPCM4bitDecoder::DecodeBlock( U8* source, F32* dest, F32* prev1, F32* prev2, F32* deltaFader )
{
	static const float f[5][2] = { { 0.0, 0.0 }, { 60.0 / 64.0,  0.0 }, {  115.0 / 64.0, -52.0 / 64.0 }, {   98.0 / 64.0, -55.0 / 64.0 }, {  122.0 / 64.0, -60.0 / 64.0 } };
	static const float predict[] = {	0.0000305175781250000000000f,	0.0000152587890625000000000f,	0.0000076293945312500000000f,	0.0000038146972656250000000f,
										0.0000019073486328125000000f,	0.0000009536743164062500000f,	0.0000004768371582031250000f,	0.0000002384185791015625000f,
										0.0000001192092895507812500f,	0.0000000596046447753906250f,	0.0000000298023223876953130f,	0.0000000149011611938476560f,
										0.0000000074505805969238281f,	0.0000000037252902984619141f,	0.0000000018626451492309570f,	0.0000000009313225746154785f };

	static const float sample[] = { 0.0f,		4096.0f,	8192.0f,	12288.0f,	16384.0f,	20480.0f,	24576.0f, 28672.0f,
									-32768.0f,	-28672.0f,	-24576.0f,	-20480.0f,	-16384.0f,	-12288.0f,	-8192.0f, -4096.0f };
	bool result = true;
	const float fade = 0.95f;
	float d = *deltaFader;
	d = (fade * fade * d);
	float vol = 1.0f - d;
	*deltaFader = d;
	
	U16 predict_nrL		= source[ 0 ];
	float shift_factorL	= predict[(predict_nrL & 0xf)];
	predict_nrL		>>= 4;
#if defined(ADPCM_BREAK_ON_VALUE_CHECK) 
	if ( predict_nrL > 4 ) DebugBreak();
#endif
#if defined( ADPCM_SHUTDOWN_ON_ERROR )
	if (BRANCH_HINT_UNLIKELY( predict_nrL > 4 )) 
	{
		return false;
	}
#endif
	const float predictA = f[predict_nrL][0];
	const float predictB = f[predict_nrL][1];
	float pprev1L = *prev1;
	float pprev2L = *prev2;
	U16 outCount = 0;
	U16 inCount = 1;

	for ( U16 i = 0; (BRANCH_HINT_LIKELY( i < BATCH_SIZE )); i += 2 )
	{
		U16 d;
		float s;
		d = source[ inCount++ ];
		s = sample[ d & 0xf ];
		float destA = ( s * shift_factorL  );
		s = sample[ (d &0xf0) >> 4 ];
		float destB = ( s * shift_factorL   );

		destA = destA + pprev1L * predictA + pprev2L * predictB;
//		if (!_finite(dest[outCount])) DebugBreak();
		pprev2L = pprev1L;
		pprev1L = destA;
		dest[outCount] = destA * vol;
#if defined(ADPCM_BREAK_ON_VALUE_CHECK) || defined( ADPCM_SHUTDOWN_ON_ERROR )
		if (BRANCH_HINT_UNLIKELY((dest[outCount] > 2.1f) || (dest[outCount] < -2.1f))) result = false;
#endif
		outCount++;

		destB = destB + pprev1L * predictA + pprev2L * predictB;
//		if (!_finite(dest[outCount])) DebugBreak();
		pprev2L = pprev1L;
		pprev1L = destB;
		dest[outCount] = destB * vol;
#if defined(ADPCM_BREAK_ON_VALUE_CHECK) || defined( ADPCM_SHUTDOWN_ON_ERROR )
		if (BRANCH_HINT_UNLIKELY((dest[outCount] > 2.1f) || (dest[outCount] < -2.1f))) result = false;
#endif
		outCount++;
	}
	*prev1 = pprev1L;
	*prev2 = pprev2L;
	return result;
}

__forceinline bool Audio::Core::ADPCM4bitDecoder::DecodeBlock3( U8* source, F32* dest, F32* prev1, F32* prev2, F32* deltaFader )
{
	return DecodeBlock( source, dest, prev1, prev2, deltaFader );
#ifdef PLATFORM_SPUA
	vector float* vpdest = (vector float*)dest;
	// 1 block == 33 bytes
	// therefore 3 vectors can hold a whole block and alignment

	// Get data into vdataA, vdataB, vdataC
	U16 alignShiftL = (U32)source & 0xf;
	U16 alignShiftR = (U32)15 - alignShiftL;
	vector unsigned char* vSrcPtr = (vector unsigned char*)source;
	vector unsigned char vsourceA = vSrcPtr[0];
	vector unsigned char vsourceB = vSrcPtr[1];
	vector unsigned char vsourceC = vSrcPtr[2];
	vector unsigned char vsourceTemp;
	
	vector unsigned char vdataA = spu_rlmaskqwbyte( vsourceA, -alignShiftR ); // shift bytes
	vdataA = spu_slqwbyte(vdataA, 15);
	alignShiftL += 1;
	vector unsigned char vdataB = spu_slqwbyte( vsourceA, alignShiftL ); // shift bytes
	vsourceTemp = spu_rlmaskqwbyte( vsourceB, -alignShiftR );
	vdataB = spu_or( vdataB, vsourceTemp );
	vector unsigned char vdataC = spu_slqwbyte( vsourceB, alignShiftL ); // shift bytes
	vsourceTemp = spu_rlmaskqwbyte( vsourceC, -alignShiftR );
	vdataC = spu_or( vdataC, vsourceTemp );

	// calculate predict, vf0 and vf1 have f[predict_nr][0] and f[predict_nr][1] respectively
	vector float vf0 = { 1.90625f,	1.53125f,	1.796875f,	0.9375f, };
	vector float vf1 = {-0.9375f,  -0.859375f,	-0.8125f,	0.0f, };
	vector unsigned char vpredict;
	vector unsigned char mask = spu_splats( (unsigned char)0 );
	vector unsigned int sixteen = spu_splats( (unsigned int)16 );
	mask = spu_insert( 0xf0, mask, 0 );
	vpredict = spu_and( vdataA, mask );
	vpredict = spu_rlmaskqw( vpredict, 6 ); // rotate  by bits
	vpredict = spu_rlmaskqwbyte( vpredict, -3 ); // shift by bytes
	vector unsigned int ivpredict = (vector unsigned int)vpredict;
	ivpredict = spu_sub( sixteen, ivpredict );
	unsigned int ipredict = spu_extract( ivpredict, 0 );
	vf0 = spu_slqwbyte( vf0, ipredict );
	vf1 = spu_slqwbyte( vf1, ipredict );
	vector unsigned char zero = spu_splats( (unsigned char) 0 );
	vector unsigned char vpattern = { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3 };
	vf0 = spu_shuffle( vf0, (vector float)zero, vpattern );
	vf1 = spu_shuffle( vf1, (vector float)zero, vpattern );
	
	// Calculate shift-factor
	vector unsigned char vshiftfactor;
	mask = spu_splats( (unsigned char)0 );
	mask = spu_insert( 0x0f, mask, 0 );
	vshiftfactor = spu_and( vdataA, mask );
	unsigned int ishift = spu_extract( vshiftfactor, 0 );
	vector unsigned int ivshiftfactor;
	ivshiftfactor = spu_splats( (unsigned int)1 );
	ivshiftfactor = spu_rl( ivshiftfactor, ishift );
	vector float fvshiftfactor = spu_convtf( ivshiftfactor, 0 );
	vector float vreciprocal = spu_re( fvshiftfactor );
	vector float vreciprocalSample = spu_splats( 1.0f / 32768.0f );
	fvshiftfactor = spu_mul( vreciprocal, vreciprocalSample );
	// Process Samples

	U16 outCount = 0;
	vector float vprevB = (vector float)zero;
	vprevB = spu_insert( *prev1, vprevB, 1 );
	vprevB = spu_insert( *prev2, vprevB, 0 );
	vector float vprevA = spu_slqwbyte( vprevB, 4 );
	vector float* vdest = (vector float*)dest;
	vector float vd0a;
	vector float vd0b;
	vector float vd0c;
	static const vector unsigned char samplePattern 		= { 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x01, 0x80, 0x80, 0x80, 0x01, 0x80 };
	static const vector unsigned int maskPattern    		= { 0x00000f00, 0x0000f000, 0x00000f00, 0x0000f000 };
	static const vector unsigned int shiftPattern   		= { 0x00000004, 0x00000000, 0x00000004, 0x00000000 };

	static const vector unsigned char zero_to_two_pattern	= { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x01, 0x02, 0x03, 0x80, 0x80, 0x80, 0x80 };
	static const vector unsigned char one_to_three_pattern	= { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x04, 0x05, 0x06, 0x07 };
	static const vector unsigned char two_to_three_pattern	= { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x08, 0x09, 0x0a, 0x0b };

	static const vector unsigned char two_mask  	= { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff };
	static const vector unsigned char three_mask  	= { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 };
	// 1. shuffle short[0], mask and shift
	for (U32 x = 0 ; x < 2 ; x++)
	{

		for (U32 word = 0 ; word < 8 ; word ++)
		{
			vector unsigned int vsample;
			vector signed int visample ;
			vector float fvsample;
			vector float fvsampleShift;
			float temp;
			vector float vtemp;
			// vdataB & vdataC have sample data in
			vsample = (vector unsigned int)vdataB;
			vsample = spu_shuffle( vsample, (vector unsigned int)zero, samplePattern );
			vsample = spu_and( vsample, maskPattern );
			vsample = spu_sl( vsample, shiftPattern );
			visample = spu_extend((vector signed short)vsample);
			fvsample = spu_convtf(visample, 0);
			// 2. Multiply by the shift-factor
			fvsampleShift = spu_mul( fvsample, fvshiftfactor );
			vd0a = spu_mul( vprevA, vf0 );
			vd0b = spu_madd( vprevB, vf1, vd0a );
			vd0c = spu_add( fvsampleShift, vd0b );

			vtemp = spu_shuffle( vd0c, (vector float)zero, zero_to_two_pattern ); 
			vprevB = spu_and( (vector float)two_mask, vprevB );
			vprevB = spu_or( vtemp, vprevB );

			vprevA = spu_slqwbyte( vprevB, 4 );
			vd0a = spu_mul( vprevA, vf0 );
			vd0b = spu_madd( vprevB, vf1, vd0a );
			vd0c = spu_add( fvsampleShift, vd0b );

			vtemp = spu_shuffle( vd0c, (vector float)zero, one_to_three_pattern ); 
			vprevB = spu_and( (vector float)three_mask, vprevB );
			vprevB = spu_or( vtemp, vprevB );
			vprevA = spu_slqwbyte( vprevB, 4 );
			vd0a = spu_mul( vprevA, vf0 );
			vd0b = spu_madd( vprevB, vf1, vd0a );
			vd0c = spu_add( fvsampleShift, vd0b );

			vtemp = spu_shuffle( vd0c, (vector float)zero, two_to_three_pattern ); 
			vprevA = spu_and( (vector float)three_mask, vprevA );
			vprevA = spu_or( vtemp, vprevA );
			vd0a = spu_mul( vprevA, vf0 );
			vd0b = spu_madd( vprevB, vf1, vd0a );
			vd0c = spu_add( fvsampleShift, vd0b );

			vdest[ outCount++ ] = spu_add( vd0c, (vector float)zero );
			vprevB = spu_slqwbyte( vd0c, 8 );
			vprevA = spu_slqwbyte( vprevB, 4 );
			vdataB = spu_slqwbyte( vdataB, 2 );
		}
		vdataB = vdataC;
	}	
	*prev1 = spu_extract( vd0c, 3 );
	*prev2 = spu_extract( vd0c, 2 );
	return true;
#endif
}


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
