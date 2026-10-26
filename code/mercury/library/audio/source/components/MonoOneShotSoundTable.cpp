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

#if !defined PLATFORM_PS3 && !defined PLATFORM_SPU
Debug::Bool gbLogSoundTablesWhenPlayed("Audio.System.Logging.LogSoundTableWavesWhenPlayed", false);
#endif

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::MonoOneShotSoundTable::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

	#ifndef CFG_GOLD
	if (pInitData->uWaves < 2)
		COMPONENT_FATAL("MonoOneshotSoundtable(): '%s' uWaves is %d! There must be at least 2 waves.", m_pchName ? m_pchName : "", pInitData->uWaves);
	#endif

	m_uEffectOffset = m_uInitOffset + sizeof(Data);

	m_uWaveOffset = m_uEffectOffset + pInitData->uEffects * sizeof(U32);
	InputWave* aWaves = (InputWave*)m_pObject->GetInitData(m_uWaveOffset);

	U32 uNext = m_uWaveOffset + pInitData->uWaves * sizeof(InputWave);

	m_uNumEffects = pInitData->uEffects;

	// the array of wave handles occupy the memory directly after the object
	WM_WAVE_HANDLE* phWave= WaveHandleArrayPtr();

	WaveExtendedInfo exInfo;

    //  prime waves
    for (U32 uWave = 0; uWave < pInitData->uWaves; uWave++)
    {
        //	load bank
		if (Success != m_pObject->GetWaveManager()->Prime((char*)m_pObject->GetString(pInitData->strBank), (char*)m_pObject->GetString(aWaves[uWave].strWave), phWave[uWave], &exInfo))
			COMPONENT_ERROR("cannot prime wave '%s' from bank '%s'", (char*)m_pObject->GetString(aWaves[uWave].strWave), (char*)m_pObject->GetString(pInitData->strBank));

		if (exInfo.bCreatedStream)
			m_pObject->IncrementNumberOfStreams();
	}

	return (void*)((U8*)m_pObject->GetInitData() + uNext);
} 

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoOneShotSoundTable::CoreMemory(U32 uOffset)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	InputWave* aWaves = (InputWave*)m_pObject->GetInitData(m_uWaveOffset);

	// the array of wave handles occupy the memory directly after the object
	WM_WAVE_HANDLE* phWave = WaveHandleArrayPtr();

	m_uDecoderTable = uOffset;

	uOffset += pInitData->uWaves * sizeof(DecoderTableEntry);

    for (U32 uWave = 0; uWave < pInitData->uWaves; uWave++)
    {
        //  get format of waves
		S32 sFormat = m_pObject->GetWaveManager()->GetFormat(phWave[uWave]);
		
        switch (sFormat)
        {
		default:
			// no break. intentional dropthrough

		case Core::uInvalidFormat:
			if (sFormat == Core::uInvalidFormat)
				COMPONENT_ERROR("could not get wave format for wave '%s' from bank '%s'. %s", m_pObject->GetString(aWaves[uWave].strWave), m_pObject->GetString(pInitData->strBank), phWave[uWave]==INVALID_HANDLE?"Wave handle does not exist.":"");
			else
				COMPONENT_ERROR("unknown/incompatible wave data type %d for wave '%s' from bank '%s'", sFormat, m_pObject->GetString(aWaves[uWave].strWave), m_pObject->GetString(pInitData->strBank));

			// no break. intentional drop through. we reserve memory for a (dummy) pcm decoder. otherwise offsets will all be messed up

	    case Core::uMP3:
        case Core::uPCM16BitSignedInt:
			uOffset += pInitData->uInstances * AUDIO_CORE_ALIGN_16(sizeof(Core::MonoPCMS16Decoder));
#ifdef PLATFORM_PS3
			m_pObject->SetDmaBufferSize(sizeof(S16) * Core::uFrameLength * 2);
#endif
            break;

        case Core::uADPCM4Bit:
            {
				uOffset += pInitData->uInstances * AUDIO_CORE_ALIGN_16(sizeof(Core::MonoADPCM4bitDecoder));
#ifdef PLATFORM_PS3
				m_pObject->SetDmaBufferSize(Core::uFrameLength);	// 1 8ve pitch shift up
#endif
				m_pObject->IncrementNumberOfAdpcmWaves(pInitData->uInstances);
            }
            break;

        }
    }

	m_uMixer = uOffset;

	//AUDIO_SYS_INFO("&&& %s mixer offset 0x%x. size 0x%x", Name(), uOffset, Core::Mixer::MixerSize( pInitData->uWaves * pInitData->uInstances, 1, 1, GetNumDSPSlots()) );


	uOffset += Core::Mixer::MixerSize( pInitData->uWaves * pInitData->uInstances, 1, 1, GetNumDSPSlots() );

	//AUDIO_SYS_INFO("&&& %s wavetable offset 0x%x. size 0x%x ", Name(), uOffset, pInitData->uWaves * sizeof(WaveTableEntry) );

	m_uWaveTable = uOffset;
	uOffset += pInitData->uWaves * sizeof(WaveTableEntry);

	for (U32 uWave = 0; uWave < pInitData->uWaves; uWave++)
	{
		//AUDIO_SYS_INFO("&&& %s wavet offset 0x%x. size 0x%x ", Name(), uOffset, pInitData->uInstances * Core::Wave::WaveSize( 1, pInitData->uEffects ) );
		uOffset += pInitData->uInstances * Core::Wave::WaveSize( 1, pInitData->uEffects );
	}

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pCoreData = pInstance->GetCoreData();
	InputWave* aWaves = (InputWave*)m_pObject->GetInitData(m_uWaveOffset);
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];
    DecoderTableEntry* aDecoderTable = DecoderTable(pInstance);
    WaveTableEntry* aWaveTable = WaveTable(pInstance);
	Core::Mixer* pMixer = Mixer(pInstance);

	// the array of wave handles occupy the memory directly after the object
	WM_WAVE_HANDLE* phWave= WaveHandleArrayPtr();

	pState->uLastFrame = (U32)-1;
    pState->uInstance = 0;

	ixCore uDecoder = m_uDecoderTable + (ixCore)pInitData->uWaves * sizeof(DecoderTableEntry);

	//  prime and get waves
	for (U32 uWave = 0; uWave < pInitData->uWaves; uWave++)
	{
		//  get format of wave
		S32 sFormat = m_pObject->GetWaveManager()->GetFormat(phWave[uWave]);
		
		//  create mono non-looping...
		switch (sFormat)
		{
		default:
			// no break. intentional drop through 

		case Core::uInvalidFormat:
			if (sFormat == Core::uInvalidFormat)
				COMPONENT_ERROR("could not get wave format for '%s' from bank '%s'. %s", m_pObject->GetString(aWaves[uWave].strWave), m_pObject->GetString(pInitData->strBank), phWave[uWave]==INVALID_HANDLE?"Wave handle does not exist.":"");
			else
			{
				COMPONENT_ERROR("unknown/incompatible wave data type %d for '%s' from bank '%s'", sFormat, m_pObject->GetString(aWaves[uWave].strWave), m_pObject->GetString(pInitData->strBank));
				sFormat = Core::uInvalidFormat;	
			}
			// no break. intentional drop through to create dummy pcm decoder

		case Core::uPCM16BitSignedInt:
	    case Core::uMP3:
			{
				aDecoderTable[uWave].uDecoder = uDecoder;
				aDecoderTable[uWave].uDecoderSize = AUDIO_CORE_ALIGN_16(sizeof(Core::MonoPCMS16Decoder));

				//  16 bit signed PCM wave data
				Core::PCMS16Decoder::WaveData waveData;

				if (sFormat != Core::uInvalidFormat)
				{
					//  get wave
					//#ifdef PLATFORM_PS3
					if (Success != m_pObject->GetWaveManager()->GetWave(phWave[uWave], &waveData, pInstance->GetNumStreamsCreatedPointer(), pInstance->GetNextStreamInfoSlot()))
					//#else
//					if (Success != m_pObject->GetWaveManager()->GetWave(phWave[uWave], &waveData))
//					#endif
					{
						COMPONENT_ERROR("cannot get wave '%s' from bank '%s'", m_pObject->GetString(aWaves[uWave].strWave), m_pObject->GetString(pInitData->strBank));
						aDecoderTable[uWave].uDecoder = ixDecoderNone;
						break;
					}
				}
				else
				{
					aDecoderTable[uWave].uDecoder = ixDecoderNone;
				}

				#ifndef CFG_GOLD
				//  check data
				if (waveData.uChannels != 1)
				{
					COMPONENT_ERROR("wave '%s' in bank '%s' is not mono", m_pObject->GetString(aWaves[uWave].strWave), m_pObject->GetString(pInitData->strBank));
					aDecoderTable[uWave].uDecoder = ixDecoderNone;
					break;
				}
				#endif	

				for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)	
				{
					//if (bThis)
					//	AUDIO_SYS_DEBUG("RFS(PCM): aDecoderTable[%d].uDecoder = %d", uWave, uDecoder);

					U8* pDecoder = &pCoreData[uDecoder];

					//  16 bit signed PCM decoder
					Core::MonoPCMS16Decoder* pPCMDecoder = NULL;
					if ((waveData.uSampleRate == Core::uSampleRate) &&
						(pInitData->optFrequency.ctrl.sCom == -1) &&
						(pInitData->optFrequency.fVal == 1.f) &&
						(pInitData->optPitch.ctrl.sCom == -1) &&
						(pInitData->optPitch.fVal == 0.f))
					{
						//	assume no SRC
						pPCMDecoder = new(pDecoder) Core::MonoPCMS16Decoder(false, false);
					}
					else
						pPCMDecoder = new(pDecoder) Core::MonoPCMS16Decoder(false, true);

					pPCMDecoder->SetWaveData(waveData);

					//  reset decoder
					pPCMDecoder->Reset(pInstance->GetDecoderDataInfo());

					//  set decoder callback
//						pPCMDecoder->SetCallbackId(m_uStateOffset, Component::eMonoOneShotSoundTable);
					pPCMDecoder->SetHasCallback();

					uDecoder += aDecoderTable[uWave].uDecoderSize;
				}
			}
			break;

		case Core::uADPCM4Bit:
			{
				aDecoderTable[uWave].uDecoder = uDecoder;
				aDecoderTable[uWave].uDecoderSize = AUDIO_CORE_ALIGN_16(sizeof(Core::MonoADPCM4bitDecoder));

				//  4 bit APCM wave data
				Core::ADPCM4bitDecoder::WaveData waveData;

				//  get wave
				//#ifdef PLATFORM_PS3
				if (Success != m_pObject->GetWaveManager()->GetWave(phWave[uWave], &waveData, pInstance->GetNumStreamsCreatedPointer(), pInstance->GetNextStreamInfoSlot()))
				//#else
				//if (Success != m_pObject->GetWaveManager()->GetWave(phWave[uWave], &waveData))
//				#endif
				{
					COMPONENT_ERROR("cannot get wave '%s' from bank '%s'", m_pObject->GetString(aWaves[uWave].strWave), m_pObject->GetString(pInitData->strBank));
					aDecoderTable[uWave].uDecoder = ixDecoderNone;
					break;
				}

				#ifndef CFG_GOLD
				//  check data
				if (waveData.uChannels != 1)
				{
					COMPONENT_ERROR("wave '%s' in bank '%s' is not mono", m_pObject->GetString(aWaves[uWave].strWave), m_pObject->GetString(pInitData->strBank));
					aDecoderTable[uWave].uDecoder = ixDecoderNone;
					break;
				}
				#endif	

				for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)
				{
					//if (bThis)
					//	AUDIO_SYS_DEBUG("RFS(PCM): aDecoderTable[%d].uDecoder = %d", uWave, uDecoder);

					U8* pDecoder = &pCoreData[uDecoder];

					//  16 bit signed PCM decoder
					Core::MonoADPCM4bitDecoder* pADPCMDecoder = NULL;
					if ((waveData.uSampleRate == Core::uSampleRate) &&
						(pInitData->optFrequency.ctrl.sCom == -1) &&
						(pInitData->optFrequency.fVal == 1.f) &&
						(pInitData->optPitch.ctrl.sCom == -1) &&
						(pInitData->optPitch.fVal == 0.f))
					{
						//	assume no SRC
						pADPCMDecoder = new(pDecoder) Core::MonoADPCM4bitDecoder(false, false);
					}
					else
						pADPCMDecoder = new(pDecoder) Core::MonoADPCM4bitDecoder(false, true);

					pADPCMDecoder->SetWaveData(waveData);

					pADPCMDecoder->SetAdpcmWaveIndex(pInstance->GetNumberOfAdpcmWavesCreated());
					pInstance->IncrementNumberOfAdpcmWavesCreated();

					//  reset decoder
					pADPCMDecoder->Reset(pInstance->GetDecoderDataInfo());

					//  set decoder callback
//						pADPCMDecoder->SetCallbackId(m_uStateOffset, Component::eMonoOneShotSoundTable);
					pADPCMDecoder->SetHasCallback();

					uDecoder += aDecoderTable[uWave].uDecoderSize;
				}
			}
			break;

		}
	}

	Core::Mixer* pIMixer = NULL;

    //  create mixer
	if (pMixer)
		pIMixer = (Core::Mixer*)new( pMixer ) Core::Mixer( Name(), pInitData->uWaves * pInitData->uInstances, 1, 1, GetNumDSPSlots(), false );

	ixCore uWave = m_uWaveTable + (ixCore)pInitData->uWaves * sizeof(WaveTableEntry);

    //  create waves and connect to mixer
	for (U32 uWaveCount = 0; uWaveCount < pInitData->uWaves; uWaveCount++)
    {
		aWaveTable[uWaveCount].uWave = uWave;

		aWaveTable[uWaveCount].uWaveSize = Core::Wave::WaveSize( 1, 0 );

		for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)
		{
			U8* pWave = &pCoreData[uWave];

			Core::Wave* pIWave = new( pWave ) Core::Wave( Name(), 1, 0, true, false );

			if (pIMixer)
			{
				pIMixer->SetInput(uWaveCount * pInitData->uInstances + uInst, Core::eWave, uWave);

				pIMixer->SetGain(uWaveCount * pInitData->uInstances + uInst, 0, 0, 1.f);
			}

			DecoderTableEntry* aDecoderTable = DecoderTable(pInstance);

			// make wrapper function for looking this up?
			ixCore uDecoder = aDecoderTable[uWaveCount].uDecoder + (ixCore)uInst * aDecoderTable[uWaveCount].uDecoderSize;
			
			if (uDecoder != ixDecoderNone)
				pIWave->SetDecoder(0, uDecoder);

			uWave += aWaveTable[uWaveCount].uWaveSize;
		}
	}

	//  for callback
    pState->u32ComponentOffset = (U32)((U8*)this - (U8*)(m_pObject->GetComponents()));	// [Blur2PC][x64] byte offset

    pState->uInstance = 0;
    pState->fIndex = -1.f;
    pState->fPitch = -1.f;
    pState->fFrequency = -1.f;
    pState->fGain = -1.f;
    pState->fLevel = -1.f;
    pState->fPosition = 0.f;
    pState->uIndex = -1;
	pState->uNumActive = 0;
	pState->bPlay = false;
//	pState->bStop = false;
    pState->bActive = false;
    pState->bMuted = false;
    pState->bWasMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::ReleaseContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pCoreData = pInstance->GetCoreData();
    DecoderTableEntry* aDecoderTable = DecoderTable(pInstance);
    WaveTableEntry* aWaveTable = WaveTable(pInstance);
    Core::Mixer* pMixer = Mixer(pInstance);

    for (U32 uWave = 0; uWave < pInitData->uWaves; uWave++)
	{
		ixCore uDecoder = aDecoderTable[uWave].uDecoder;
		if (uDecoder != ixDecoderNone)
		{
			U8* pDecoder = &pCoreData[uDecoder];
			U8* pWave = &pCoreData[aWaveTable[uWave].uWave];

			for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)
			{
				((Core::Decoder*)pDecoder)->Shutdown(pInstance->GetDecoderDataInfo());
				((Core::Decoder*)pDecoder)->~Decoder();
				((Core::Wave*)pWave)->~Wave();

				pDecoder += aDecoderTable[uWave].uDecoderSize;

				pWave += aWaveTable[uWave].uWaveSize;
			}
		}
	}

	if (pMixer)
		pMixer->~Mixer();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoOneShotSoundTable");

	bool bFrequencyChanged = false;
	bool bVolumeChanged = false;

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pCoreData = pInstance->GetCoreData();
	State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
    WaveTableEntry* pWaveTable = WaveTable(pInstance);
    DecoderTableEntry* pDecoderTable = DecoderTable(pInstance);

	for (U32 uWave = 0; uWave < pInitData->uWaves; uWave++)
	{
		ixCore uDecoder = pDecoderTable[uWave].uDecoder;
		if (uDecoder != ixDecoderNone)
		{
			U8* pDecoder = &pCoreData[uDecoder];
			U8* pWave = &pCoreData[pWaveTable[uWave].uWave];

			for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)
			{
				if (((Core::Decoder*)pDecoder)->HasCallbackFired())
					Finished(pInstance);

				pDecoder += pDecoderTable[uWave].uDecoderSize;
			}
		}
	}


	//  only recalc if 1st request this frame
	if (pState->uLastFrame == System::Get()->GetFrameNum())
		return;

	if (pInitData->uWaves < 2)
		return;

    //  get index
	F32 fIndex = GetOptionValue( pInstance, pInitData->optIndex );

    //  wraparound wave indices

	// original version.. wasn't working right on the spu
    //if (fIndex < 0.f)
    //    fIndex = fIndex + (S32)(1.f - fIndex / (F32)pInitData->uWaves) * pInitData->uWaves;
    //else if (fIndex >= (F32)pInitData->uWaves)
    //    fIndex = fIndex - (S32)(fIndex / (F32)pInitData->uWaves) * pInitData->uWaves;

	// fix for bug #6231. negative indices were being set and this code didnt deal with it.
	// not sure the exact behaviour we require here so resetting to 0 to be on the safe side
	if (fIndex < 0.f)
		fIndex = 0.f;

	fIndex = F32((S32)fIndex % pInitData->uWaves);

    //	set index
    if (pState->fIndex != fIndex)
    {
        pState->fIndex = fIndex;

        pState->uIndex = (U32)fIndex;

/*
        //  create x-fade blend
        pState->uIndexA = (U32)fIndex;
        pState->uIndexB = (pState->uIndexA == pInitData->uWaves - 1) ? 0 : pState->uIndexA + 1;
        F32 fBlendA = fIndex - (F32)pState->uIndexA;
        F32 fBlendB = 1.f - fBlendA;
        pState->fLevelA = sqrtf(fBlendB);
        pState->fLevelB = sqrtf(fBlendA);*/
    }

    //  get frequency
	F32 fFrequency = GetOptionValue( pInstance, pInitData->optFrequency );

    //  has changed
    //if (pState->fFrequency != fFrequency)
    {
        pState->fFrequency = fFrequency;
		bFrequencyChanged = true;
    }

    //  get pitch
	F32 fPitch = GetOptionValue( pInstance, pInitData->optPitch );

    //  has changed
    //if (pState->fPitch != fPitch)
    {
        pState->fPitch = fPitch;
        if (fPitch != 0.f)
            fFrequency *= powf(2.f, fPitch);
		bFrequencyChanged = true;
    }

	if (bFrequencyChanged)
	{
		for (U32 uWave = 0; uWave < pInitData->uWaves; uWave++)
		{
			ixCore uDecoder = pDecoderTable[uWave].uDecoder;
			if (uDecoder != ixDecoderNone)
			{
				U8* pDecoder = &pCoreData[uDecoder];
				U8* pWave = &pCoreData[pWaveTable[uWave].uWave];

				for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)
				{
					((Core::Decoder*)pDecoder)->SetPitch(fFrequency);
					pDecoder += pDecoderTable[uWave].uDecoderSize;
				}
			}
		}
	}


    //  get level
	F32 fLevel = GetOptionValue( pInstance, pInitData->optLevel );

    //  has changed
    //if (pState->fLevel != fLevel)
    {
        pState->fLevel = fLevel;
		bVolumeChanged = true;
    }

    //  get gain
	F32 fGain = GetOptionValue( pInstance, pInitData->optGain );

    //  has changed
    //if( pState->fGain != fGain )
    {
        pState->fGain = fGain;
        if (fGain != 0.f)
            fLevel *= Maths::dBToFloat(fGain);

		bVolumeChanged = true;
    }

#if defined (AUDIO_ENABLE_PROFILING)
	if (Audio::EnableProfiling)
	{
		if (pState->bMuted)
		{
			fLevel = 0.f;

			if (!pState->bWasMuted)
				bVolumeChanged = true;
		}
		else // not muted
		{
			if (pState->bWasMuted)
				bVolumeChanged = true;
		}
	}
#endif	//	AUDIO_ENABLE_PROFILING

	//  set volume
    //if (bVolumeChanged)
    {
        //  set wave volumes
		if (pWaveTable)
		{
			for (U32 uCount = 0; uCount < pInitData->uWaves; uCount++)
			{
				ixCore uWave = pWaveTable[uCount].uWave;

				for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)
				{
					Core::Wave* pWave = (Core::Wave*)&pCoreData[uWave];

					if (pWave)
						pWave->SetVolume(fLevel);

					uWave += pWaveTable[uCount].uWaveSize;
				}
			}
		}
    }

	ixCore uDecoder = pDecoderTable ? pDecoderTable[ pState->uIndex ].uDecoder + (ixCore)pState->uInstance * pDecoderTable[ pState->uIndex ].uDecoderSize : ixDecoderNone;

	if( uDecoder != ixDecoderNone )
	{
		Core::Decoder* pDecoder = (Core::Decoder*)pInstance->GetCoreData( uDecoder );

		if (pDecoder)
			pState->fPosition = pDecoder->Position(pInstance->GetDecoderDataInfo());
	}

#ifdef AUDIO_DEFER_SOUNDS
	if (pState->bPlay)
	{
		pState->bPlay = false;
		Play(pInstance);
	}
//	else if (pState->bStop)
//	{
//		pState->bStop = false;
//		Stop(pStateData);
//	}
#endif // AUDIO_DEFER_SOUNDS


#if defined (AUDIO_ENABLE_PROFILING)
	if (Audio::EnableProfiling)
	{
		pState->bActive = false;
		if (pWaveTable)
		{
			for (U32 uWave = 0; uWave < pInitData->uWaves; uWave++)
			{
				ixCore uWaveOffset = pWaveTable[uWave].uWave;

				for (U32 uCount = 0; uCount < pInitData->uInstances; uCount++)
				{
					Core::Wave* pWaveInst = (Core::Wave*)&pCoreData[uWaveOffset];

					if (pWaveInst && pWaveInst->IsActive())
					{
						pState->bActive = true;
						break;
					}
				}

				if (pState->bActive)
					break;
			}
		}

		pState->bWasMuted = pState->bMuted;
	}
#endif	//	AUDIO_ENABLE_PROFILING

	pState->uLastFrame = System::Get()->GetFrameNum();
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::MonoOneShotSoundTable::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case EntriesCtrl:
        return (F32)pInitData->uWaves;
        break;

    case ActiveCtrl:
		return (F32)pState->uNumActive;
        break;

    case PositionCtrl:
		return pState->fPosition;
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
void Audio::MonoOneShotSoundTable::DoMessage(Instance* pInstance, U32 uMessage)
{
	State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	switch (uMessage)
    {
    case PlayMsg:
		{
#ifdef AUDIO_DEFER_SOUNDS
//			if (pState->bStop)
//				pState->bStop = false;

			pState->bPlay = true;
#else
			//MonoOneShotSoundTable::Update(pStateData);
			MonoOneShotSoundTable::Play(pInstance);
#endif // AUDIO_DEFER_SOUNDS
		}
        break;

    case StopMsg:
		{
//#ifdef AUDIO_DEFER_SOUNDS
//			if (pState->bPlay)
//				pState->bPlay = false;

//			pState->bStop = true;
//#else
//			Update(pStateData);
			MonoOneShotSoundTable::Stop(pInstance);
//#endif // AUDIO_DEFER_SOUNDS
		}
        break;

    case PrimeMsg:
		{
			MonoOneShotSoundTable::Prime(pInstance);
		}
        break;

    case FlushMsg:
		{
			MonoOneShotSoundTable::Flush(pInstance);
		}
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
void Audio::MonoOneShotSoundTable::ConfigureAudio(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U32* aEffects = (U32*)m_pObject->GetInitData(m_uEffectOffset);
	U8* pCoreData = pInstance->GetCoreData();
	U8* pStateData = pInstance->GetStateData();
	Core::Mixer* pMixer = Mixer(pInstance);

    //  do effects
	if (pMixer)
	{
		//  do effects
		for (U32 uEffect = 0, uSlot = 0; uEffect < pInitData->uEffects; uEffect++)
		{
			if (aEffects[uEffect] != sUnused)
			{
				U32 uNumUnits = m_pObject->GetComponent(aEffects[uEffect])->DSPEffectCount( );

				for (U32 uUnit = 0; uUnit < uNumUnits; uUnit++)
				{
					pMixer->SetEffect(uSlot + uUnit, m_pObject->GetComponent(aEffects[uEffect])->DSPEffectType(pStateData, uUnit), 
										m_pObject->GetComponent(aEffects[uEffect])->DSPEffectUnit(pStateData, uUnit));

					uSlot += uNumUnits;
				}
			}
		}
	}
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::MonoOneShotSoundTable::AudioOutType(Instance* pInstance) const
{
	Data* pData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	return Core::eMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	Play
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::Play(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
    DecoderTableEntry* aDecoderTable = DecoderTable( pInstance );
    WaveTableEntry* aWaveTable = (WaveTableEntry*)pInstance->GetCoreData( m_uWaveTable );

	//force update to ensure member variables have default values
	Update(pInstance);

	//  increment current instance
	if( ++pState->uInstance >= pInitData->uInstances )
		pState->uInstance = 0;

	//	get next wave and decoder
	ixCore uWave = aWaveTable[ pState->uIndex ].uWave + (ixCore)pState->uInstance * aWaveTable[ pState->uIndex ].uWaveSize;
	ixCore uDecoder = aDecoderTable ? aDecoderTable[ pState->uIndex ].uDecoder + (ixCore)pState->uInstance * aDecoderTable[ pState->uIndex ].uDecoderSize : ixDecoderNone;

	if( uDecoder != ixDecoderNone )
	{
		Core::Wave* pWave = (Core::Wave*)pInstance->GetCoreData( uWave );
		Core::Decoder* pDecoder = (Core::Decoder*)pInstance->GetCoreData( uDecoder );
		
		if( !pWave->IsActive() )
			pState->uNumActive++;

		//	reset it
		pWave->Reset( pInstance->m_coreInfo, 0 );

		//  get frequency
		pState->fFrequency = GetFrequency( pInstance, pInitData->optFrequency, pInitData->optPitch );
		
		//  get level
		pState->fLevel = pState->bMuted ? 0.f : GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

		//  set decoder pitch
		pDecoder->SetPitch( pState->fFrequency );

		//  set wave volume
		pWave->SetVolume( pState->fLevel );
		
		#if !defined CFG_GOLD && !defined PLATFORM_PS3 && !defined PLATFORM_SPU
		if (gbLogSoundTablesWhenPlayed)
		{
			InputWave* aWaves = (InputWave*)m_pObject->GetInitData(m_uWaveOffset);
			AUDIO_SYS_DEBUG("Playing sound table wave '%s'", m_pObject->GetString(aWaves[pState->uIndex].strWave));
		}
		#endif

		

		//	play it
		pWave->Start( pInstance->m_coreInfo );
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Stop
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::Stop(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pCoreData = pInstance->GetCoreData();
	State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
    DecoderTableEntry* aDecoderTable = DecoderTable(pInstance);
    WaveTableEntry* aWaveTable = (WaveTableEntry*)&pCoreData[m_uWaveTable];

    //  stop all
    for (U32 uCount = 0; uCount < pInitData->uWaves; uCount++)
    {
		ixCore uWave = aWaveTable[uCount].uWave;

		ixCore uDecoder = aDecoderTable[uCount].uDecoder;

		if (uDecoder != ixDecoderNone)
		{
			U8* pDecoder = &pCoreData[uDecoder];

			for (U32 uInst = 0; uInst < pInitData->uInstances; uInst++)
			{
				//	stop waves
				Core::Wave* pWave = (Core::Wave*)&pCoreData[uWave];

				pWave->Stop(pInstance->m_coreInfo);

				uWave += aWaveTable[uCount].uWaveSize;

				//	shutdown decoders
				((Core::Decoder*)pDecoder)->Shutdown(pInstance->GetDecoderDataInfo());
				uDecoder += aDecoderTable[uCount].uDecoderSize;
			}
		}
    }

	pState->uNumActive = 0;
}

////////////////////////////////////////////////////////////////////////////////
//	Prime
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::Prime(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pCoreData = pInstance->GetCoreData();
    DecoderTableEntry* aDecoderTable = DecoderTable(pInstance);
    WaveTableEntry* aWaveTable = (WaveTableEntry*)&pCoreData[m_uWaveTable];

    for (U32 uCount = 0; uCount < pInitData->uWaves; uCount++)
    {
		ixCore uWave = aWaveTable[uCount].uWave;

		ixCore uDecoder = aDecoderTable[uCount].uDecoder;

		if (uDecoder != ixDecoderNone)
		{
			U8* pDecoder = &pCoreData[uDecoder];

			if (pDecoder)
				((Core::Decoder*)pDecoder)->Prime(pInstance->GetDecoderDataInfo());
		}
	}

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
void Audio::MonoOneShotSoundTable::Flush(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pCoreData = pInstance->GetCoreData();
    DecoderTableEntry* aDecoderTable = DecoderTable(pInstance);
    WaveTableEntry* aWaveTable = (WaveTableEntry*)&pCoreData[m_uWaveTable];

    for (U32 uCount = 0; uCount < pInitData->uWaves; uCount++)
    {
		ixCore uWave = aWaveTable[uCount].uWave;

		ixCore uDecoder = aDecoderTable[uCount].uDecoder;

		if (uDecoder != ixDecoderNone)
		{
			U8* pDecoder = &pCoreData[uDecoder];

			if (pDecoder)
				((Core::Decoder*)pDecoder)->Flush(pInstance->GetDecoderDataInfo());
		}
	}

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
void Audio::MonoOneShotSoundTable::Finished(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	DoEvent(pInstance, pInitData->evFinished);

	pState->uNumActive--;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutOffset
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::MonoOneShotSoundTable::AudioOutOffset(Instance* pInstance) const
{
	if (m_uDecoderTable == ixDecoderNone || m_uMixer == ixMixerNone)
	{
		U8* pCoreData = pInstance->GetCoreData();
		WaveTableEntry* aWaveTable = (WaveTableEntry*)&pCoreData[m_uWaveTable];
		return aWaveTable[0].uWave;		// ?
	}
	else
		return m_uMixer;
}

////////////////////////////////////////////////////////////////////////////////
//	Release
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::Release()
{
#ifndef PLATFORM_SPU
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	// the array of wave handles occupy the memory directly after the object
	WM_WAVE_HANDLE* phWave= WaveHandleArrayPtr();

	//  TODO: need to split Prime() into 2 funcs so we don't need to call Release() for each wave
    for (U32 uWave = 0; uWave < pInitData->uWaves; uWave++)
    {
		m_pObject->GetWaveManager()->Release(phWave[uWave]);
	}
#endif	//	#ifndef PLATFORM_SPU
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::MonoOneShotSoundTable::IsActive(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::SetMute(Instance* pInstance, bool bMuted)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
