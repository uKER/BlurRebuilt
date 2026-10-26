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
void* Audio::StereoOneShotSound::Init(void* pData)
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
U32 Audio::StereoOneShotSound::CoreMemory(U32 uOffset)
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
			m_uDecoderSize = AUDIO_CORE_ALIGN_16(sizeof(Core::StereoPCMS16Decoder));
			uOffset += pInitData->uInstances * m_uDecoderSize;
#ifdef PLATFORM_PS3
			m_pObject->SetDmaBufferSize(sizeof(S16) * Core::uFrameLength * 2 * 2);
#endif
        }
        break;

    case Core::uADPCM4Bit:
        {
			m_uDecoderSize = AUDIO_CORE_ALIGN_16(sizeof(Core::StereoADPCM4bitDecoder));
			uOffset += pInitData->uInstances * m_uDecoderSize;
#ifdef PLATFORM_PS3
			m_pObject->SetDmaBufferSize(Core::uFrameLength * 2);	// 1 8ve pitch shift up
#endif
			m_pObject->IncrementNumberOfAdpcmWaves(pInitData->uInstances);
        }
        break;


    case Core::uInvalidFormat:
		COMPONENT_ERROR("could not get wave format for wave '%s' from bank '%s'. %s", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank), m_hWave==INVALID_HANDLE?"Wave handle does not exist.":"");
		m_uDecoder = ixDecoderNone;
		break;

	default:
		COMPONENT_ERROR("unknown/incompatible wave data type %d for wave '%s' from bank '%s'", sFormat, m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
		m_uDecoder = ixDecoderNone;
        break;
    }

	m_uMixer = uOffset;

	//  create mixer
	// dont reserve mem for mixer if we have no decoder or only one instance
	if (pInitData->uInstances == 1 || m_uDecoder == ixDecoderNone)
		m_uMixer = ixMixerNone;
	else
		uOffset += Core::Mixer::MixerSize( pInitData->uInstances, 2, 2, GetNumDSPSlots() );

	m_uWave = uOffset;

	if (pInitData->uInstances == 1)
		m_uWaveSize = Core::Wave::WaveSize( 2, pInitData->uEffects );
	else
		m_uWaveSize = Core::Wave::WaveSize( 2, 0 );

	uOffset += pInitData->uInstances * m_uWaveSize;

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoOneShotSound::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);
	Core::Decoder* pDecoder = DecoderBase(pInstance);
	Core::Mixer* pMixer = Mixer(pInstance);
	Core::Wave* pWave = WaveBase(pInstance);

    //  get format of wave
    S32 sFormat = m_pObject->GetWaveManager()->GetFormat(m_hWave);

	if (pDecoder)
	{
		//  and create decoder
		switch (sFormat)
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
				#endif

				//  for each instance
				for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)
				{
					Core::Decoder* pDecoderInst = DecoderInstance(pDecoder, uInst);
					if (!pDecoderInst)
						continue;

					//  16 bit signed PCM decoder
					Core::StereoPCMS16Decoder* pPCMDecoder = NULL;
					if ((waveData.uSampleRate == Core::uSampleRate) &&
						(pInitData->optFrequency.ctrl.sCom == -1) &&
						(pInitData->optFrequency.fVal == 1.f) &&
						(pInitData->optPitch.ctrl.sCom == -1) &&
						(pInitData->optPitch.fVal == 0.f))
					{
						//	assume no SRC
						pPCMDecoder = new(pDecoderInst) Core::StereoPCMS16Decoder(false, false);
					}
					else
						pPCMDecoder = new(pDecoderInst) Core::StereoPCMS16Decoder(false, true);

					//  set wave data
					pPCMDecoder->SetWaveData(waveData);

					//  reset wave context
					pPCMDecoder->Reset(pInstance->GetDecoderDataInfo());

					//  set decoder callback
//					pPCMDecoder->SetCallbackId(m_uStateOffset, Component::eStereoOneShotSound);
					pPCMDecoder->SetHasCallback();
				}
			}
			break;

		case Core::uADPCM4Bit:
			{
				//  4 bit ADPCM wave data
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
					COMPONENT_ERROR("wave '%s' in bank '%s' is not stereo", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
					m_uDecoder = ixDecoderNone;
					break;
				}
				#endif

				//  for each instance
				for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)
				{
					Core::Decoder* pDecoderInst = DecoderInstance(pDecoder, uInst);
					if (!pDecoderInst)
						continue;

					//  4 bit APCM decoder
					Core::StereoADPCM4bitDecoder* pAPCMDecoder = NULL;
					if ((waveData.uSampleRate == Core::uSampleRate) &&
						(pInitData->optFrequency.ctrl.sCom == -1) &&
						(pInitData->optFrequency.fVal == 1.f) &&
						(pInitData->optPitch.ctrl.sCom == -1) &&
						(pInitData->optPitch.fVal == 0.f))
					{
						//	assume no SRC
						pAPCMDecoder = new(pDecoderInst) Core::StereoADPCM4bitDecoder(false, false);
					}
					else
						pAPCMDecoder = new(pDecoderInst) Core::StereoADPCM4bitDecoder(false, true);

					//  set wave data
					pAPCMDecoder->SetWaveData(waveData);

					pAPCMDecoder->SetAdpcmWaveIndex(pInstance->GetNumberOfAdpcmWavesCreated());
					pInstance->IncrementNumberOfAdpcmWavesCreated();

					//  reset wave context
					pAPCMDecoder->Reset(pInstance->GetDecoderDataInfo());

					//  set decoder callback
//					pAPCMDecoder->SetCallbackId(m_uStateOffset, Component::eStereoOneShotSound);
					pAPCMDecoder->SetHasCallback();
				}
			}
			break;


		case Core::uInvalidFormat:
			COMPONENT_ERROR("could not get wave format for '%s' from bank '%s'. %s", m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank), m_hWave==INVALID_HANDLE?"Wave handle does not exist.":"");
			m_uDecoder = ixDecoderNone;
			break;

		default:
			COMPONENT_ERROR("unknown/incompatible wave data type %d for '%s' from bank '%s'", sFormat, m_pObject->GetString(pInitData->strWave), m_pObject->GetString(pInitData->strBank));
			m_uDecoder = ixDecoderNone;
			break;
		}
	}

	Core::Mixer* pIMixer = NULL;

    //  create mixer
	if (pInitData->uInstances != 1 && pMixer)
		pIMixer = (Core::Mixer*)new( pMixer ) Core::Mixer( Name(), pInitData->uInstances, 2, 2, pInitData->uEffects, false );
        
    if (pInitData->uInstances == 1)
    {
        //  only one instance
		Core::Wave* pIWave = new( pWave ) Core::Wave( Name(), 2, GetNumDSPSlots(), true, false );

		pIWave->SetDecoder(0, m_uDecoder);
    }
    else
    {
        //  create waves and connect to mixer
        for (U32 uCount = 0; uCount < pInitData->uInstances; uCount++)
        {
			Core::Wave* pWaveInst = WaveInstance(pWave, uCount);
			Core::Wave* pIWave = NULL;
			
			if (pWaveInst)
				pIWave = new( pWaveInst ) Core::Wave( Name(), 2, 0, true, false );

			if (pIMixer)
			{
				pIMixer->SetInput(uCount, Core::eWave, m_uWave + uCount * m_uWaveSize);

				pIMixer->SetGain(uCount, 0, 0, 1.f);
				pIMixer->SetGain(uCount, 0, 1, 0.f);
				pIMixer->SetGain(uCount, 1, 0, 0.f);
				pIMixer->SetGain(uCount, 1, 1, 1.f);
			}

			if (pIWave)
				pIWave->SetDecoder(0, m_uDecoder + uCount * m_uDecoderSize);
		}
    }

	//  for callback
    pState->u32ComponentOffset = (U32)((U8*)this - (U8*)(m_pObject->GetComponents()));	// [Blur2PC][x64] byte offset

    pState->uInstance = (U32)-1;
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
void Audio::StereoOneShotSound::ReleaseContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Core::Mixer* pMixer = Mixer(pInstance);
	Core::Decoder* pDecoder = DecoderBase(pInstance);
	Core::Wave* pWave = WaveBase(pInstance);

    for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)
	{
		Core::Wave* pWaveInst = WaveInstance(pWave, uInst);
		Core::Decoder* pDecoderInst = DecoderInstance(pDecoder, uInst);

		if (pDecoderInst)
		{
			pDecoderInst->Shutdown(pInstance->GetDecoderDataInfo());
			pDecoderInst->~Decoder();
		}

		if (pWaveInst)
			pWaveInst->~Wave();
	}

	if (pInitData->uInstances > 1 && pMixer)
		pMixer->~Mixer();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoOneShotSound::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoOneShotSound");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pObjContext = pInstance->GetNewStateData();
    State* pState = (State*)&pObjContext[m_uStateOffset];
    Core::Wave* pWave = WaveBase(pInstance);
	Core::Decoder* pDecoder = DecoderBase(pInstance);

	if ( pWave && pDecoder )
	{
		for ( U32 uCount = 0; uCount < pInitData->uInstances; uCount++ )
		{
			Core::Decoder* pDecoderInst = DecoderInstance( pDecoder, uCount );

			if ( pDecoderInst && pDecoderInst->HasCallbackFired() )
				Finished(pInstance);
		}

		if ( pState->bActive )
		{
			//  get frequency
			F32 fFrequency = GetFrequency( pInstance, pInitData->optFrequency, pInitData->optPitch );

			//  has changed
			if ( pState->fFrequency != fFrequency )
			{
				pState->fFrequency = fFrequency;

				//  set decoder pitch
				for ( U32 uCount = 0; uCount < pInitData->uInstances; uCount++ )
				{
					Core::Decoder* pDecoderInst = DecoderInstance( pDecoder, uCount );
					if ( pDecoderInst )
						pDecoderInst->SetPitch( fFrequency );
				}
			}

			//  get level
			F32 fLevel = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

			//  has changed
			if (pState->fLevel != fLevel)
			{
				pState->fLevel = fLevel;

				//  set wave volumes
				for (U32 uCount = 0; uCount < pInitData->uInstances; uCount++)
				{
					Core::Wave* pWaveInst = WaveInstance(pWave, uCount);
					if (pWaveInst)
						pWaveInst->SetVolume(fLevel);
				}
			}

			//  set position (to current active instance)
			if (pState->uInstance != (U32)-1)
			{
     			Core::Decoder* pDecoderInst = DecoderInstance(pDecoder, pState->uInstance);
				if (pDecoderInst)
					pState->fPosition = pDecoder->Position(pInstance->GetDecoderDataInfo());

				if (pState->fPosition >= 1.f)
					pState->fPosition = 0.f;
			}

			//	set active flag
			pState->bActive = false;
			for (U32 uCount = 0; uCount < pInitData->uInstances; uCount++)
			{
				Core::Wave* pWaveInst = WaveInstance(pWave, uCount);
				if (pWaveInst && pWaveInst->IsPlaying())
				{
					pState->bActive = true;
					break;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::StereoOneShotSound::Value(Instance* pInstance, U32 uOutput)
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
void Audio::StereoOneShotSound::DoMessage(Instance* pInstance, U32 uMessage)
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
void Audio::StereoOneShotSound::ConfigureAudio(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Com* aEffects = (Com*)m_pObject->GetInitData(m_uEffectOffset);
	U8* pStateData = pInstance->GetStateData();
	Core::Wave* pWave = WaveBase(pInstance);
	Core::Mixer* pMixer = Mixer(pInstance);

    //  do effects
    for (U32 uEffect = 0, uSlot = 0; uEffect < pInitData->uEffects; uEffect++)
    {
        if (aEffects[uEffect] != sUnused)
        {
			U32 uNumUnits = m_pObject->GetComponent(aEffects[uEffect])->DSPEffectCount( );

			for (U32 uUnit = 0; uUnit < uNumUnits; uUnit++)
			{
				if (pInitData->uInstances <= 1)
				{
					pWave->SetEffect(uSlot + uUnit, m_pObject->GetComponent(aEffects[uEffect])->DSPEffectType(pStateData, uUnit), 
											m_pObject->GetComponent(aEffects[uEffect])->DSPEffectUnit(pStateData, uUnit));
				}
				else
				{
					if (pMixer)
					{
						pMixer->SetEffect(uSlot + uUnit, m_pObject->GetComponent(aEffects[uEffect])->DSPEffectType(pStateData, uUnit), 
											m_pObject->GetComponent(aEffects[uEffect])->DSPEffectUnit(pStateData, uUnit));
					}
				}

				uSlot += uNumUnits;
			}
        }
    }
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Play
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoOneShotSound::Play(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pObjContext = pInstance->GetNewStateData();
    State* pState = (State*)&pObjContext[m_uStateOffset];
	Core::Wave* pWave = WaveBase(pInstance);
	Core::Decoder* pDecoder = DecoderBase(pInstance);

	//  move to next sound instance
	if ( ++pState->uInstance >= pInitData->uInstances )
		pState->uInstance = 0;

	Core::Wave* pWaveInst = WaveInstance( pWave, pState->uInstance );
	Core::Decoder* pDecoderInst = DecoderInstance( pDecoder, pState->uInstance );

	if ( pWaveInst && pDecoderInst )
	{
		pWaveInst->Reset( pInstance->m_coreInfo, 0 );

		//  get frequency
		pState->fFrequency = GetFrequency( pInstance, pInitData->optFrequency, pInitData->optPitch );
		
		//  get level
		pState->fLevel = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

		//  set decoder pitch
		pDecoderInst->SetPitch( pState->fFrequency );

		//  set wave volume
		pWaveInst->SetVolume( pState->fLevel );

		if ( !pWaveInst->IsActive() )
			pState->uNumActive++;

		pWaveInst->Start( pInstance->m_coreInfo );

		pState->bActive = true;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Stop
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoOneShotSound::Stop(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pObjContext = pInstance->GetNewStateData();
    State* pState = (State*)&pObjContext[m_uStateOffset];
	Core::Wave* pWave = WaveBase(pInstance);

	for (U32 uCount = 0; uCount < pInitData->uInstances; uCount++)
	{
		Core::Wave* pWaveInst = WaveInstance(pWave, uCount);
		if (pWaveInst)
			pWaveInst->Stop(pInstance->m_coreInfo);
	}
			
	pState->uNumActive = 0;
}
////////////////////////////////////////////////////////////////////////////////
//	Prime
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoOneShotSound::Prime(Instance* pInstance)
{
	Core::Decoder* pDecoder = DecoderBase(pInstance);

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
void Audio::StereoOneShotSound::Flush(Instance* pInstance)
{
	Core::Decoder* pDecoder = DecoderBase(pInstance);

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
void Audio::StereoOneShotSound::Finished(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	DoEvent(pInstance, pInitData->evFinished);

	pState->uNumActive--;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::StereoOneShotSound::AudioOutType(Instance* pInstance) const
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	if (pInitData->uInstances <= 1 || m_uDecoder == ixDecoderNone || m_uMixer == ixMixerNone)
		return Core::eWave;
	else
		return Core::eMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutOffset
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::StereoOneShotSound::AudioOutOffset(Instance* pInstance) const
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	if (pInitData->uInstances <= 1 || m_uDecoder == ixDecoderNone || m_uMixer == ixMixerNone)
		return m_uWave;
	else
		return m_uMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	Release
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::StereoOneShotSound::Release()
{
	m_pObject->GetWaveManager()->Release(m_hWave);
}
#endif

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::StereoOneShotSound::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoOneShotSound::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
