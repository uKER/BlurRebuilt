#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::StereoLoopingSound::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

	m_uEffectOffset = m_uInitOffset + sizeof(Data);

	U32 uNext = m_uEffectOffset + pInitData->uEffects * sizeof(U32);

	m_uNumEffects = pInitData->uEffects;

	WaveExtendedInfo exInfo;

    //	load bank
	if (Success != m_pObject->GetWaveManager()->Prime(m_pObject->GetString(pInitData->strBank), m_pObject->GetString(pInitData->strWave), m_hWave, &exInfo))
		COMPONENT_ERROR("cannot get wave '%s' from bank '%s'", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));

	if (exInfo.bCreatedStream)
		m_pObject->IncrementNumberOfStreams();

    return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoLoopingSound::CoreMemory(U32 uOffset)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Com* aEffects = (Com*)m_pObject->GetInitData(m_uEffectOffset);

	m_uDecoder = uOffset;

    //  get format of wave
	S32 sFormat = m_pObject->GetWaveManager()->GetFormat(m_hWave);

    switch (sFormat)
    {
    case Core::uMP3:
    case Core::uPCM16BitSignedInt:
		{
			uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::StereoPCMS16Decoder));
#ifdef PLATFORM_PS3
			m_pObject->SetDmaBufferSize(sizeof(S16) * Core::uFrameLength * 2 * 2);
#endif
		}
        break;

    case Core::uADPCM4Bit:
		{
			uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::StereoADPCM4bitDecoder));
#ifdef PLATFORM_PS3
			m_pObject->SetDmaBufferSize(Core::uFrameLength * 2);	// 1 8ve pitch shift up
#endif
			m_pObject->IncrementNumberOfAdpcmWaves();
		}
        break;


	case Core::uInvalidFormat:
		COMPONENT_ERROR("could not get wave format for wave '%s' from bank '%s'. %s", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank), m_hWave==INVALID_HANDLE?"Wave handle does not exist.":"");
		m_uDecoder = ixDecoderNone;
		break;

	default:
		COMPONENT_ERROR("unknown/incompatible wave data type %d for '%s' from bank '%s'", sFormat, m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
		m_uDecoder = ixDecoderNone;
        break;
    }

	m_uWave = (ixCore)uOffset;

	//  create wave
    uOffset += Core::Wave::WaveSize( 2, GetNumDSPSlots() );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoLoopingSound::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);
	Core::Decoder* pDecoder = Decoder(pInstance);
	Core::Wave* pWave = Wave(pInstance);

    //  get format of wave
	pState->sFormat = m_pObject->GetWaveManager()->GetFormat(m_hWave);

	if (pDecoder)
	{
		//  and create decoder
		switch (pState->sFormat)
		{
	    case Core::uMP3:
		case Core::uPCM16BitSignedInt:
			{
				//  16 bit signed PCM wave data
				Core::PCMS16Decoder::WaveData waveData;

				//  get wave
				//#ifdef PLATFORM_PS3
				if (Success != m_pObject->GetWaveManager()->GetWave(m_hWave, &waveData, pInstance->GetNumStreamsCreatedPointer(), pInstance->GetNextStreamInfoSlot()))
				//#else
				//if (Success != m_pObject->GetWaveManager()->GetWave(m_hWave, &waveData))
//				#endif
				{
					COMPONENT_ERROR("cannot get wave '%s' from bank '%s'", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
					m_uDecoder = ixDecoderNone;
					break;
				}

				#ifndef CFG_GOLD
				//  check data
				if (waveData.uChannels != 2)
				{
					COMPONENT_ERROR("wave '%s' from bank '%s' is not stereo", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
					m_uDecoder = ixDecoderNone;
					break;
				}

				if (waveData.bLoop == false)
				{
					COMPONENT_ERROR("wave '%s' from bank '%s' is not a loop", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
					m_uDecoder = ixDecoderNone;
					break;
				}
				#endif

				//  16 bit signed PCM decoder
				Core::StereoPCMS16Decoder* pPCMDecoder = NULL;
				if ((waveData.uSampleRate == Core::uSampleRate) &&
					(pInitData->optFrequency.ctrl.sCom == -1) &&
					(pInitData->optFrequency.fVal == 1.f) &&
					(pInitData->optPitch.ctrl.sCom == -1) &&
					(pInitData->optPitch.fVal == 0.f))
				{
					//	assume no SRC
					pPCMDecoder = new(pDecoder) Core::StereoPCMS16Decoder(true, false);
				}
				else
					pPCMDecoder = new(pDecoder) Core::StereoPCMS16Decoder(true, true);

				//  set wave data
				pPCMDecoder->SetWaveData(waveData);

				//  reset wave context
				pPCMDecoder->Reset(pInstance->GetDecoderDataInfo());

				//  set decoder callback
//				pPCMDecoder->SetCallbackId(m_uStateOffset, Component::eStereoLoopingSound);
				pPCMDecoder->SetHasCallback();
			}
			break;

		case Core::uADPCM4Bit:
		   {
				Core::ADPCM4bitDecoder::WaveData waveData;

				//  get wave
				//#ifdef PLATFORM_PS3
				if (Success != m_pObject->GetWaveManager()->GetWave(m_hWave, &waveData, pInstance->GetNumStreamsCreatedPointer(), pInstance->GetNextStreamInfoSlot()))
				//#else
				//if (Success != m_pObject->GetWaveManager()->GetWave(m_hWave, &waveData))
//				#endif
				{
					COMPONENT_ERROR("cannot get wave '%s' from bank '%s'", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
					m_uDecoder = ixDecoderNone;
					break;
				}

				#ifndef CFG_GOLD
				//  check data
				if (waveData.uChannels != 2)
				{
					COMPONENT_ERROR("wave '%s' from bank '%s' is not stereo", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
					m_uDecoder = ixDecoderNone;
					break;
				}

				if (waveData.bLoop == false)
				{
					COMPONENT_ERROR("wave '%s' from bank '%s' is not a loop", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
					m_uDecoder = ixDecoderNone;
					break;
				}
				#endif

				//  4 bit stereo APCM decoder
				Core::StereoADPCM4bitDecoder* pAPCMDecoder = NULL;
				if ((waveData.uSampleRate == Core::uSampleRate) &&
					(pInitData->optFrequency.ctrl.sCom == -1) &&
					(pInitData->optFrequency.fVal == 1.f) &&
					(pInitData->optPitch.ctrl.sCom == -1) &&
					(pInitData->optPitch.fVal == 0.f))
				{
					//	assume no SRC
					pAPCMDecoder = new(pDecoder) Core::StereoADPCM4bitDecoder(true, false);
				}
				else
					pAPCMDecoder = new(pDecoder) Core::StereoADPCM4bitDecoder(true, true);

				pAPCMDecoder->SetAdpcmWaveIndex(pInstance->GetNumberOfAdpcmWavesCreated());
				pInstance->IncrementNumberOfAdpcmWavesCreated();

				//  set wave data
				pAPCMDecoder->SetWaveData(waveData);

				//  reset wave context
				pAPCMDecoder->Reset(pInstance->GetDecoderDataInfo());

				//  set decoder callback
//				pAPCMDecoder->SetCallbackId(m_uStateOffset, Component::eStereoLoopingSound);
				pAPCMDecoder->SetHasCallback();
			}
			break;


		case Core::uInvalidFormat:
			COMPONENT_ERROR("could not get format for wave '%s' from bank. '%s'", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank), m_hWave==INVALID_HANDLE?"Wave handle does not exist.":"");
			m_uDecoder = ixDecoderNone;
			break;

		default:
			COMPONENT_ERROR("unknown/incompatible wave data type %d for '%s' from bank '%s'", pState->sFormat, m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
			break;
		}
	}

    //  create wave
	if (pWave)
	{	
		new( pWave ) Core::Wave( Name(), 2, GetNumDSPSlots(), false, false );

		//	set decoder
		pWave->SetDecoder(0, m_uDecoder);
	}

    //  for callback
    pState->u32ComponentOffset = (U32)((U8*)this - (U8*)(m_pObject->GetComponents()));	// [Blur2PC][x64] byte offset

    pState->fFrequency = -1.f;
    pState->fLevel = -1.f;
    pState->fPosition = 0.f;
	pState->uNumActive = 0;

    pState->bActive = false;
    pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoLoopingSound::ReleaseContext(Instance* pInstance)
{
	Core::Decoder* pDecoder = Decoder(pInstance);
	Core::Wave* pWave = Wave(pInstance);

    if (pDecoder)
    {
        pDecoder->Shutdown(pInstance->GetDecoderDataInfo());
		pDecoder->~Decoder();
	}

    if (pWave)
		pWave->~Wave();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoLoopingSound::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoLoopingSound");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::Wave* pWave = Wave(pInstance);
	Core::Decoder* pDecoder = Decoder(pInstance);

	if ( pWave && pDecoder )
	{
		if (pDecoder->HasCallbackFired())
			Finished(pInstance);

		//  get frequency
		F32 fFrequency = GetFrequency( pInstance, pInitData->optFrequency, pInitData->optPitch );

		//  has changed
		if ( pState->fFrequency != fFrequency )
		{
			pState->fFrequency = fFrequency;
			pDecoder->SetPitch( fFrequency );
		}

		//  get level
		F32 fLevel = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

		//  has changed
		if ( pState->fLevel != fLevel )
		{
			pState->fLevel = fLevel;
			pWave->SetVolume( fLevel );
		}

		//  set position
		if (pDecoder)
			pState->fPosition = pDecoder->Position(pInstance->GetDecoderDataInfo());

		//  set active flag
		pState->bActive = pWave->IsPlaying();
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::StereoLoopingSound::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

    switch(uOutput)
    {
    case PositionCtrl:
        return pState->fPosition;
        break;

    case ActiveCtrl:
        return (F32)pState->uNumActive;
        break;

    default:
        Assert(0);
        break;
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoLoopingSound::DoMessage(Instance* pInstance, U32 uMessage)
{
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

    switch (uMessage)
    {
    case PlayMsg:
        Play(pInstance);
        break;

    case StopMsg:
        Stop(pInstance);
        break;

    case PrimeMsg:
        Prime(pInstance);
        break;

    case FlushMsg:
        Flush(pInstance);
        break;

    default:
        Assert(0);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	ConfigureAudio
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::StereoLoopingSound::ConfigureAudio(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Com* aEffects = (Com*)m_pObject->GetInitData(m_uEffectOffset);
	U8* pStateData = pInstance->GetStateData();
	Core::Wave* pWave = Wave(pInstance);

	U32 uSlot = 0;
    for (U32 uEffect = 0; uEffect < pInitData->uEffects; uEffect++)
    {
        if (aEffects[uEffect] != sUnused)
		{
			U32 uNumUnits = m_pObject->GetComponent(aEffects[uEffect])->DSPEffectCount( );

			for (U32 uUnit = 0; uUnit < uNumUnits; uUnit++)
			{
				if (pWave)
				{
					pWave->SetEffect(uSlot + uUnit, m_pObject->GetComponent(aEffects[uEffect])->DSPEffectType(pStateData, uUnit), 
											m_pObject->GetComponent(aEffects[uEffect])->DSPEffectUnit(pStateData, uUnit));
				}

				uSlot += uNumUnits;
			}
		}
    }
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::StereoLoopingSound::AudioOutType(Instance* pInstance) const
{
	return Core::eWave;
}

////////////////////////////////////////////////////////////////////////////////
//	Play
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoLoopingSound::Play(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::Wave* pWave = Wave(pInstance);
	Core::Decoder* pDecoder = Decoder(pInstance);

	if ( pWave && pDecoder )
	{
		//	get offset and set position
		pState->fPosition = GetOptionValue( pInstance, pInitData->optOffset );

		pState->fPosition = Maths::Clamp(pState->fPosition, 0.f, 1.f);
		
		pWave->Reset( pInstance->m_coreInfo, (U32)( pState->fPosition * (F32)pDecoder->Length() ) );
		
		//  get frequency
		pState->fFrequency = GetFrequency( pInstance, pInitData->optFrequency, pInitData->optPitch );

		//	set frequency
		pDecoder->SetPitch( pState->fFrequency );

		//  get level
		pState->fLevel = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

		//	set level
		pWave->SetVolume( pState->fLevel );

		if ( !pWave->IsPlaying() )
			pState->uNumActive = 1;

		pWave->Start( pInstance->m_coreInfo );
	}

	//  set active flag
	pState->bActive = true;
}

////////////////////////////////////////////////////////////////////////////////
//	Stop
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoLoopingSound::Stop(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::Wave* pWave = Wave(pInstance);

	if (pWave)
		pWave->Stop(pInstance->m_coreInfo);

	pState->uNumActive = 0;
}


////////////////////////////////////////////////////////////////////////////////
//	Prime
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoLoopingSound::Prime(Instance* pInstance)
{
	Core::Decoder* pDecoder = Decoder(pInstance);

	if (pDecoder)
		pDecoder->Prime(pInstance->GetDecoderDataInfo());

	//Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	//U8* pObjContext = pInstance->GetNewStateData();
 //   State* pState = (State*)&pObjContext[m_uStateOffset];
	//Core::Wave* pWave = WaveBase(pInstance);

	//for (U32 uCount = 0; uCount < pInitData->uInstances; uCount++)
	//{
	//	Core::Wave* pWaveInst = WaveInstance(pWave, uCount);
	//	if (pWaveInst)
	//		pWaveInst->Prime(pInstance->m_coreInfo);
	//}

	//pState->uNumActive = 0;
}

////////////////////////////////////////////////////////////////////////////////
//	Flush
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoLoopingSound::Flush(Instance* pInstance)
{
	Core::Decoder* pDecoder = Decoder(pInstance);

	if (pDecoder)
		pDecoder->Flush(pInstance->GetDecoderDataInfo());

	//Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	//U8* pObjContext = pInstance->GetNewStateData();
 //   State* pState = (State*)&pObjContext[m_uStateOffset];
	//Core::Wave* pWave = WaveBase(pInstance);

	//for (U32 uCount = 0; uCount < pInitData->uInstances; uCount++)
	//{
	//	Core::Wave* pWaveInst = WaveInstance(pWave, uCount);
	//	if (pWaveInst)
	//		pWaveInst->Flush(pInstance->m_coreInfo);
	//}

	//pState->uNumActive = 0;
}
////////////////////////////////////////////////////////////////////////////////
//	Finished
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoLoopingSound::Finished(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	DoEvent(pInstance, pInitData->evLooped);

	pState->uNumActive = 0;
}

////////////////////////////////////////////////////////////////////////////////
//	Release
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::StereoLoopingSound::Release()
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	m_pObject->GetWaveManager()->Release(m_hWave);
}
#endif

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::StereoLoopingSound::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoLoopingSound::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
