
#include "AudioPch.h"
#include "coreheaders.h"
//#include "alloca.h"
#include "stdlib.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
//#include <System\SPU\AutoPointer.h>
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	MixerSize (static)
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
U32 Audio::Core::Mixer::MixerSize( U16 uInputs, U16 uChannelsIn, U16 uChannelsOut, U16 uEffects )
{
	U32 uSize = sizeof( Mixer );

	//	effects
	uSize += sizeof( DSP::Element ) * uEffects;

	//	inputs
	uSize += uInputs * sizeof( Item );

	//	channel map
	uSize += 2 * uInputs * uChannelsIn * uChannelsOut * sizeof( F32 );

	return AUDIO_CORE_ALIGN_16( uSize );
}

////////////////////////////////////////////////////////////////////////////////
//	Mixer
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Mixer::Mixer( const char* pchInfo, U16 uInputs, U16 uChannelsIn, U16 uChannelsOut, U16 uEffects, bool bAlwaysOn ) : Generator( pchInfo, uChannelsOut, bAlwaysOn )
{
	m_uClassSize = sizeof( Mixer );

	m_uEffects = uEffects;

	U8* pEnd = (U8*)this + m_uClassSize;
	DSP::Element* aEffects = (DSP::Element*)pEnd;

	Generator::ResetEffects( aEffects, m_uEffects );

	m_uInputs = uInputs;

	m_uInputOffset = sizeof( DSP::Element ) * m_uEffects;

	m_uChannelsIn = uChannelsIn;
	m_uChannelsOut = uChannelsOut;
	
	m_uPrevGainOffset = m_uInputOffset + m_uInputs * sizeof( Item );
	m_uGainOffset = m_uPrevGainOffset + m_uInputs * m_uChannelsIn * m_uChannelsOut * sizeof( F32 );
	
	Item* aInputs = (Item*)&pEnd[ m_uInputOffset ];
	F32* afPrevGain = (F32*)&pEnd[ m_uPrevGainOffset ];
	F32* afGain = (F32*)&pEnd[ m_uGainOffset ];

	Generator::m_eType = eMixer;

	//	initialise gains
	for (U32 uInput = 0; uInput < m_uInputs; uInput++)
	{
		for (U32 uChIn = 0; uChIn < m_uChannelsIn; uChIn++)
		{
			for (U32 uChOut = 0; uChOut < Generator::m_u32NumChannels; uChOut++)
				afGain[uInput * m_uChannelsIn * m_uChannelsOut + uChIn * m_uChannelsOut + uChOut] = 0.f;
		}

        //  null inputs
		aInputs[uInput].eType = eUndefined;
		aInputs[uInput].uOffset = 0;
	}

	memcpy(afPrevGain, afGain, m_uInputs * m_uChannelsIn * m_uChannelsOut * sizeof(F32));

#ifndef CFG_GOLD
	Generator::m_uTotalMix++;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	~Mixer
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Mixer::~Mixer()
{
#ifndef CFG_GOLD
	if ( IsActive() )
		Generator::m_uActiveMix--;

	Generator::m_uTotalMix--;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	DynamicBufferDepths
//		pass in buffer stack class...
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Mixer::DynamicBufferDepths(const ProcessInfo& info, BufferStack& buffers)
{
	U8* pThis = (U8*)this + m_uClassSize;
	Item* aInputs = (Item*)&pThis[ m_uInputOffset ];

	if (!Generator::m_bDynamicBuffMem)
    {
		buffers.SetDepth( buffers.GetDepth() + 1 );

	    for (U32 uInput = 0; uInput < m_uInputs; uInput++)
		{
			switch( aInputs[ uInput ].eType )
			{
			case eMixer:
				( (Mixer*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DynamicBufferDepths( info, buffers );
				break;
			case eWave:
				( (Wave*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DynamicBufferDepths( info, buffers );
				break;
			case eInput:
				( (Input*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DynamicBufferDepths( info, buffers );
				break;
			case eReturn:
				( (Return*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DynamicBufferDepths( info, buffers );
				break;
			}
		}

		buffers.SetDepth( buffers.GetDepth() - 1 );

		Generator::DynamicBufferDepths( info, buffers );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DynamicBufferOffsets
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Mixer::DynamicBufferOffsets(const ProcessInfo& info, BufferStack& buffers)
{
	U8* pThis = (U8*)this + m_uClassSize;
	Item* aInputs = (Item*)&pThis[ m_uInputOffset ];

	if (!Generator::m_bDynamicBuffSizes)
    {
		buffers.SetDepth( buffers.GetDepth() + 1 );

	    for (U32 uInput = 0; uInput < m_uInputs; uInput++)
		{
			switch( aInputs[ uInput ].eType )
			{
			case eMixer:
				( (Mixer*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DynamicBufferOffsets( info, buffers );
				break;
			case eWave:
				( (Wave*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DynamicBufferOffsets( info, buffers );
				break;
			case eInput:
				( (Input*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DynamicBufferOffsets( info, buffers );
				break;
			case eReturn:
				( (Return*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DynamicBufferOffsets( info, buffers );
				break;
			}
		}

		buffers.SetDepth( buffers.GetDepth() - 1 );

		Generator::DynamicBufferOffsets( info, buffers );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	StaticBufferOffsets
////////////////////////////////////////////////////////////////////////////////
U32 Audio::Core::Mixer::StaticBufferOffsets(const ProcessInfo& info, U32 uSize)
{
	U8* pThis = (U8*)this + m_uClassSize;
	Item* aInputs = (Item*)&pThis[ m_uInputOffset ];

	if (!Generator::m_bStaticBuffMem)
    {
	    for (U32 uInput = 0; uInput < m_uInputs; uInput++)
		{
			switch( aInputs[uInput].eType )
			{
			case eMixer:
				uSize = ( (Mixer*)&info.pCoreData[ aInputs[uInput].uOffset ] )->StaticBufferOffsets( info, uSize );
				break;
			case eWave:
				uSize = ( (Wave*)&info.pCoreData[ aInputs[uInput].uOffset ] )->StaticBufferOffsets( info, uSize );
				break;
			case eOutput:
				uSize = ( (Output*)&info.pCoreData[ aInputs[uInput].uOffset ] )->StaticBufferOffsets( info, uSize );
				break;
			case eInput:
				uSize = ( (Input*)&info.pCoreData[ aInputs[uInput].uOffset ] )->StaticBufferOffsets( info, uSize );
				break;
			case eReturn:
				uSize = ( (Return*)&info.pCoreData[ aInputs[uInput].uOffset ] )->StaticBufferOffsets( info, uSize );
				break;
			}
		}
    }

	return Generator::StaticBufferOffsets(info, uSize);
}

////////////////////////////////////////////////////////////////////////////////
//	SetInput
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Mixer::SetInput(U32 uInput, U32 uType, U32 uOffset)
{
	U8* pThis = (U8*)this + m_uClassSize;
	Item* aInputs = (Item*)&pThis[ m_uInputOffset ];

	Assert(uInput < m_uInputs);

	aInputs[uInput].eType = (Type)uType;
	aInputs[uInput].uOffset = uOffset;

    return true;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Mixer::SetProcessFlag( const ProcessInfo& info, bool bProcess )
{
	U8* pThis = (U8*)this + m_uClassSize;
	Item* aInputs = (Item*)&pThis[ m_uInputOffset ];
	F32* afPrevGain = (F32*)&pThis[ m_uPrevGainOffset ];
	F32* afGain = (F32*)&pThis[ m_uGainOffset ];

	bool bProcessing = bProcess;
#ifdef PLATFORM_SPU
	void* pMemBase = Spu::GetMemBase();
	F32* afMaxGain = (F32*)(Spu::Alloc( ((sizeof(F32) * m_uInputs) + 15) & ~15) );
#else
	F32* afMaxGain = (F32*)alloca( m_uInputs * sizeof(F32) );
#endif

	//	if not always on, check volumes to see if processing can be skipped
	if( !Generator::m_bAlwaysOn )
	{
		//  if all gains zero, don't process
		F32 maxGain = 0.f;

		//	for each input
		for( U32 uInput = 0; uInput < m_uInputs; uInput++ )
		{
			afMaxGain[ uInput ] = 0.f;

			//  for each channel in
			for( U32 uChIn = 0; uChIn < m_uChannelsIn; uChIn++ )
			{
				//  and each channel out
				for( U32 uChOut = 0; uChOut < m_uChannelsOut; uChOut++ )
					afMaxGain[ uInput ] = Maths::Max( afMaxGain[ uInput ], afGain[ uInput * m_uChannelsIn * m_uChannelsOut + uChIn * m_uChannelsOut + uChOut ] );
			}

			maxGain = Maths::Max( maxGain, afMaxGain[ uInput ] );
		}

		//	if all silent gain values, don't process this mixer
		if( maxGain < Maths::fEpsilon )
			bProcessing = false;
	}

	//	test inputs to see if processing is required
	bool bInputProcessing = false;

	//	are any inputs processing?
	for( U32 uInput = 0; uInput < m_uInputs; uInput++ )
	{
		bool bProcessInput = bProcess && ( afMaxGain[ uInput ] >= Maths::fEpsilon );

		switch( aInputs[ uInput ].eType )
		{
		case eMixer:
			bInputProcessing = ( (Mixer*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->SetProcessFlag( info, bProcessInput ) ? true : bInputProcessing;
			break;
		case eWave:
			bInputProcessing = ( (Wave*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->SetProcessFlag( info, bProcessInput ) ? true : bInputProcessing;
			break;
		case eInput:
			bInputProcessing = ( (Input*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->SetProcessFlag( info, bProcessInput ) ? true : bInputProcessing;
			break;
		case eReturn:
			bInputProcessing = ( (Return*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->SetProcessFlag( info, bProcessInput ) ? true : bInputProcessing;
			break;
		case eUndefined:
		default:
			break;
		}
	}

	//	if no inputs are processing, disable processing on this mixer (unless always on)
	if( !bInputProcessing )
		bProcessing = Generator::m_bAlwaysOn;

#ifndef CFG_GOLD
	if( !Generator::m_bProcessing && bProcessing )
	{
		Generator::m_uActiveMix++;
		Generator::m_uActiveGen++;
	}
	else if( Generator::m_bProcessing && !bProcessing )
	{
		Generator::m_uActiveMix--;
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

#ifdef PLATFORM_SPU
	Spu::SetMemBase(pMemBase);
#endif

	return Generator::m_bProcessing;
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Mixer::Process( const ProcessInfo& info )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.Mixer.Process");

	U8* pThis = (U8*)this + m_uClassSize;
	Item* aInputs = (Item*)&pThis[ m_uInputOffset ];
	F32* afPrevGain = (F32*)&pThis[ m_uPrevGainOffset ];
	F32* afGain = (F32*)&pThis[ m_uGainOffset ];

	if (Generator::m_uFrameCount != info.uFrameNum)
	{
		if (Generator::m_uBuffer != (U32)-1)
		{
			F32* AUDIO_RESTRICT pDataOut = (F32*)&info.pDynamicBuffers[ Generator::m_uBuffer ];

			//  clear output buffer
			for (U32 uCh = 0; uCh < Generator::m_u32NumChannels; uCh++)
				Frame::Zero( &pDataOut[ uCh * uFrameLength ] );
	 
			//	process all valid inputs
			for (U32 uInput = 0; uInput < m_uInputs; uInput++)
			{
				//  process each input...
				switch ( aInputs[ uInput ].eType )
				{
				case eMixer:
					( (Mixer*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->Process( info );
					break;
				case eWave:
					( (Wave*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->Process( info );
					break;
				case eInput:
					( (Input*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->Process( info );
					break;
				case eReturn:
					( (Return*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->Process( info );
					break;
				case eUndefined:
				default:
					AUDIO_CORE_ERROR( "Mixer::Process() - Unknown input type" );
					break;
				}

				//  if processing active
				if( Generator::m_bProcessing )
				{
					F32* AUDIO_RESTRICT pDataIn = NULL;
					switch( aInputs[ uInput ].eType )
					{
					case eMixer:
						pDataIn = ( ( Mixer* )&info.pCoreData[ aInputs[ uInput ].uOffset ] )->OutputBuffer( info );
						break;
					case eWave:
						pDataIn = ( ( Wave* )&info.pCoreData[ aInputs[ uInput ].uOffset ] )->OutputBuffer( info );
						break;
					case eInput:
						pDataIn = ( ( Input* )&info.pCoreData[ aInputs[ uInput ].uOffset ] )->OutputBuffer( info );
						break;
					case eReturn:
						pDataIn = ( ( Return* )&info.pCoreData[ aInputs[ uInput ].uOffset ] )->OutputBuffer( info );
						break;
					}

					//	for each input channel
					for (U32 uChIn = 0; uChIn < m_uChannelsIn; uChIn++)
					{
						F32* AUDIO_RESTRICT pfFrom = &pDataIn[uFrameLength * uChIn];

#ifdef AUDIO_CHECK_SAMPLES
						if (!Frame::IsValid((F32*)pfFrom))
							AUDIO_CORE_FATAL("Mixer::Process() Invalid sample in pfFrom frame in '%s'. chanIn = %d", m_pchInfo, uChIn);
#endif // AUDIO_CHECK_SAMPLES

						//	for each output channel
						for (U32 uChOut = 0; uChOut < Generator::m_u32NumChannels; uChOut++)
						{
							if (info.bMutePrev)
								afPrevGain[uInput * m_uChannelsIn * m_uChannelsOut + uChIn * m_uChannelsOut + uChOut] = 0.f;

							if (info.bMuteNext)
								afGain[uInput * m_uChannelsIn * m_uChannelsOut + uChIn * m_uChannelsOut + uChOut] = 0.f;

							F32 fPrevGain = afPrevGain[uInput * m_uChannelsIn * m_uChannelsOut + uChIn * m_uChannelsOut + uChOut];
							F32 fGain = afGain[uInput * m_uChannelsIn * m_uChannelsOut + uChIn * m_uChannelsOut + uChOut];

							F32* AUDIO_RESTRICT pfTo = &pDataOut[uFrameLength * uChOut];

#ifdef AUDIO_CHECK_SAMPLES
							if (!Frame::IsValid((F32*)pfTo))
								AUDIO_CORE_FATAL("Mixer::Process() Invalid sample in pfTo frame (before gain) in '%s'. fGain = %.2f. prevgain is %.2f. chanIn = %d. chanOut = %d", m_pchInfo, fGain, fPrevGain, uChIn, uChOut);
#endif // AUDIO_CHECK_SAMPLES

							//	sum for these channels if gain not zero
							if( fPrevGain == fGain )
							{
								if (fGain == 1.f)
									Frame::DoAdd(pfTo, pfTo, pfFrom);
								else if (fGain != 0.f)
									Frame::AddGain(pfTo, pfTo, pfFrom, fGain);
							}
							else
							{
								F32 fDelta = fGain - fPrevGain;
								F32 fStartGain = fPrevGain + info.fFraction * fDelta; 
								F32 fEndGain = fPrevGain + (info.fFraction + info.fWidth) * fDelta;

								Frame::AddGainRamp(pfTo, pfTo, pfFrom, fStartGain, fEndGain);
							}
#ifdef AUDIO_CHECK_SAMPLES
							if (!Frame::IsValid((F32*)pfTo))
								AUDIO_CORE_FATAL("Mixer::Process() Invalid sample in pfFrom frame (after gain) in '%s'. fGain = %.2f. prevgain is %.2f. chanIn = %d. chanOut = %d", m_pchInfo, fGain, fPrevGain, uChIn, uChOut);
#endif // AUDIO_CHECK_SAMPLES
						}
					}
				}
			}

			if ( Generator::m_bProcessing )	//	TODO: do we run effects anyway when not processing audio?
			{
#ifdef AUDIO_CHECK_SAMPLES
				if (!Frame::IsValid((F32*)pDataOut))
					AUDIO_CORE_DEBUG("Mixer::PreDSP()");
#endif // AUDIO_CHECK_SAMPLES

				//  process effects
				Generator::Process( info );

#ifdef AUDIO_CHECK_SAMPLES
				if (!Frame::IsValid((F32*)pDataOut))
					AUDIO_CORE_DEBUG("Mixer::PostDSP()");
#endif // AUDIO_CHECK_SAMPLES
			}

			if (info.uFrameMod == info.uFrameInt - 1)
				memcpy( afPrevGain, afGain, m_uInputs * m_uChannelsIn * m_uChannelsOut * sizeof(F32) );
		}
		else
			AUDIO_CORE_WARN("Mixer in '%s' not connected to audio graph", m_pchInfo);
	}
}

#ifdef DUMP_PROCESSING_STATS
////////////////////////////////////////////////////////////////////////////////
//	DumpProcessingStatus
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Mixer::DumpProcessingStatus( const ProcessInfo& info, const char* pchInstance, U32 uDepth )
{
	U8* pThis = (U8*)this + m_uClassSize;
	Item* aInputs = (Item*)&pThis[ m_uInputOffset ];

	Generator::DumpProcessingStatus( info, pchInstance, uDepth );

	for( U32 uInput = 0; uInput < m_uInputs; uInput++ )
	{
		switch ( aInputs[ uInput ].eType )
		{
		case eMixer:
			( (Mixer*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DumpProcessingStatus( info, pchInstance, uDepth + 2 );
			break;
		case eWave:
			( (Wave*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DumpProcessingStatus( info, pchInstance, uDepth + 2 );
			break;
		case eInput:
			( (Input*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DumpProcessingStatus( info, pchInstance, uDepth + 2 );
			break;
		case eReturn:
			( (Return*)&info.pCoreData[ aInputs[ uInput ].uOffset ] )->DumpProcessingStatus( info, pchInstance, uDepth + 2 );
			break;
		case eUndefined:
		default:
			AUDIO_CORE_ERROR( "Mixer::DumpProcessingStatus() - Unknown input type" );
			break;
		}
	}
}
#endif

#ifdef ENABLE_DEBUG_EXTENSIONS

#include "stdio.h"
////////////////////////////////////////////////////////////////////////////////
//	DumpStats
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Mixer::DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals)
{
	U8* pThis = (U8*)this + m_uClassSize;
	Item* aInputs = (Item*)&pThis[ m_uInputOffset ];

	if (pInternals)
    {
        pInternals->uTotalNumGenerators++;
        pInternals->uTotalNumMixers++;

        if (Generator::m_bActive)
        {
            pInternals->uNumGeneratorsActive++;
            pInternals->uNumMixersActive++;
        }
    }

	F32* aFrameData = (F32*)&info.pDynamicBuffers[Generator::m_uBuffer];

	bool bHasSignal = false;
	for (U32 uCh = 0; uCh < Generator::m_u32NumChannels; uCh++)
	{
		if (!Frame::IsZero(&aFrameData[uCh * uFrameLength]))
		{
			bHasSignal = true;
			break;
		}
	}

    if (Generator::m_pchInfo)
		AUDIO_SYS_DEBUG("%s[Mixer (%d in) %dch->%dch (%d fx)] \"%s\" <%s> F:0x%.8x (%s)", pchPrefix, m_uInputs, m_uChannelsIn, Generator::m_u32NumChannels, 8, Generator::m_pchInfo, Generator::m_bActive ? "active" : "inactive", aFrameData, bHasSignal ? "true" : "false");
    else
        AUDIO_SYS_DEBUG("%s[Mixer (%d in) %dch->%dch (%d fx)] <%s> F:0x%.8x (%s)", pchPrefix, m_uInputs, m_uChannelsIn, Generator::m_u32NumChannels, 8, Generator::m_bActive ? "active" : "inactive", aFrameData, bHasSignal ? "true" : "false");

	for (U32 uInput = 0; uInput < m_uInputs; uInput++)
    {
        char pch[256];

		if (aInputs[uInput].eType != eUndefined)
		{
            sprintf(pch, "  |%s", pchPrefix);
			switch( aInputs[uInput].eType )
			{
			case eMixer:
				((Mixer*)&info.pCoreData[aInputs[uInput].uOffset])->Mixer::DumpStats(info, pch, pInternals);
				break;

			case eWave:
				((Wave*)&info.pCoreData[aInputs[uInput].uOffset])->DumpStats(info, pch, pInternals);
				break;

			//case eInput:
			//	((Input*)pGenerator)->DumpStats(info, pch, pInternals);
			//	break;

			case eOutput:
				((IOutput*)&info.pCoreData[aInputs[uInput].uOffset])->DumpStats(info, pch, pInternals);
				break;

			case eOscillator:
				((IOscillator*)&info.pCoreData[aInputs[uInput].uOffset])->DumpStats(info, pch, pInternals);
				break;

			case eReturn:
				((Return*)&info.pCoreData[aInputs[uInput].uOffset])->DumpStats(info, pch, pInternals);
				break;
			}
		}
        else
        {
            sprintf(pch, "  |%s[None]", pchPrefix);
            AUDIO_SYS_DEBUG(pch);
        }
    }
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
