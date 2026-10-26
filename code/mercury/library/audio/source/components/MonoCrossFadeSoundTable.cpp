#include "AudioPch.h"
#ifdef SN_TARGET_PS3_SPU
#include "SystemHeadersSPU.h"
#else
#include "SystemHeaders.h"
#endif
#include "core/core.h"
#include "system/memory.h"

//using namespace Audio;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF

#define AUDIO_DEFER_SOUNDS

////////////////////////////////////////////////////////////////////////////////
//	Callback (static)
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::FinishedCallback(void* pContext)
{
    Audio::MonoOneShotSoundTable::Context* pThisContext = (Audio::MonoOneShotSoundTable::Context*)pContext;

    //  get mono looping sound
    Audio::MonoOneShotSoundTable* pSound = (Audio::MonoOneShotSoundTable*)pThisContext->pThis;
    
    //  signal completion
    pSound->Finished(pThisContext->pObjContext);
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void* Audio::MonoOneShotSoundTable::Init(void* pData)
{ 
	m_pchName = m_pObject->GetString(m_pData->strName);

	// TODO: setup string hash or use PreInit template

    Assert(m_pData->uWaves >= 2);

    U32* aEffects = (U32*)(m_pData + 1);

    //  setup effects table
    if (m_pData->uEffects)
    {
        m_aEffects = aEffects;
        aEffects += m_pData->uEffects;
    }
    else
        m_aEffects = NULL;

    InputWave* aWaves = (InputWave*)aEffects;

    //  setup wave table
    if (m_pData->uWaves)
    {
        m_aWaves = aWaves;
        aWaves += m_pData->uWaves;
    }
    else
        m_aWaves = NULL;

    //  prime waves
    for (U32 uWave = 0; uWave < m_pData->uWaves; uWave++)
    {
        //	load bank
        if (Success != m_pObject->GetWaveManager()->Prime((char*)m_pObject->GetString(m_pData->strBank), (char*)m_pObject->GetString(m_aWaves[uWave].strWave)))
            COMPONENT_ERROR("cannot prime wave '%s' from bank '%s'", (char*)m_pObject->GetString(m_aWaves[uWave].strWave), (char*)m_pObject->GetString(m_pData->strBank));
	}

    return (void*)aWaves;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::InitContext(U8* pObjContext)
{
    MonoOneShotSoundTable::Context* pContext = (MonoOneShotSoundTable::Context*)&pObjContext[m_uOffset];

	pContext->uLastFrame = (U32)-1;
    pContext->uInstance = 0;

    //  create arrays
    pContext->apWaveA = new Core::IWave*[m_pData->uInstances];
    pContext->apWaveB = new Core::IWave*[m_pData->uInstances];
    //pContext->apDecoder = new Core::Decoder*[m_pData->uWaves];//remove
    //pContext->apDecoderContext = new U8*[m_pData->uInstances * m_pData->uWaves];//remove

    pContext->apDecoder = new Core::Decoder*[2 * m_pData->uInstances * m_pData->uWaves];

    //  prime and get waves
    for (U32 uWave = 0; uWave < m_pData->uWaves; uWave++)
    {
        //  get format of wave
        S32 sFormat = m_pObject->GetWaveManager()->GetFormat(m_pObject->GetString(m_pData->strBank), m_pObject->GetString(m_aWaves[uWave].strWave));

        //  create mono non-looping...
        switch (sFormat)
        {
        case Core::uPCM16BitSignedInt:
            {
				//  16 bit signed PCM wave data
				Core::PCMS16Decoder::WaveData waveData;

				//  get wave
				if (Success != m_pObject->GetWaveManager()->GetWave(m_pObject->GetString(m_pData->strBank), m_pObject->GetString(m_aWaves[uWave].strWave), &waveData))
					COMPONENT_ERROR("cannot get wave '%s' from bank '%s'", m_pObject->GetString(m_aWaves[uWave].strWave), m_pObject->GetString(m_pData->strBank));

				//  check data
				Assert(waveData.uChannels == 1);

				//  for each instance
				//for (U32 uWave = 0; uWave < m_pData->uWaves; uWave++)
				{
					for (U32 uInst = 0; uInst < m_pData->uInstances; uInst++)
					{
						//  create decoders
						Core::PCMS16Decoder* pDecoderA = new Core::PCMS16Decoder;
						pContext->apDecoder[2 * (uWave * m_pData->uInstances + uInst)] = pDecoderA;

						Core::PCMS16Decoder* pDecoderB = new Core::PCMS16Decoder;
						pContext->apDecoder[2 * (uWave * m_pData->uInstances + uInst) + 1] = pDecoderB;

						//  set wave data
						pDecoderA->SetWaveData(waveData);
						pDecoderB->SetWaveData(waveData);

						//  reset decoders
						pDecoderA->Reset(NULL);
						pDecoderB->Reset(NULL);
					}
				}
            }
            break;
/*
        case Core::uADPCM4Bit:
            {
                //  ADPCM decoder
                Core::ADPCM4bitDecoder* pDecoder = new Core::ADPCM4bitDecoder;
                pContext->apDecoder[uWave] = pDecoder;

                //  create wave data
                Core::ADPCM4bitDecoder::WaveData* pWaveData = new Core::ADPCM4bitDecoder::WaveData;

                //  get wave
                if (Success != m_pObject->GetWaveManager()->GetWave(m_pObject->GetString(m_pData->strBank), m_pObject->GetString(m_aWaves[uWave].strWave), pWaveData))
                    COMPONENT_ERROR("cannot get wave '%s' from bank '%s'", m_pObject->GetString(m_aWaves[uWave].strWave), m_pObject->GetString(m_pData->strBank));

                //  check data
                Assert(pWaveData->uChannels == 1);
                Assert(pWaveData->bLoop == false);

                //  for each instance
                for (U32 uInst = 0; uInst < m_pData->uInstances; uInst++)
                {
                    //  create context
                    Core::ADPCM4bitDecoder::Context* pDecoderContext = new Core::ADPCM4bitDecoder::Context;

                    //  set context
                    pContext->apDecoderContext[uWave * m_pData->uInstances + uInst] = (U8*)pDecoderContext;

                    //  set wave data
                    //pDecoderContext->pWaveData = pWaveData;

                    //  reset wave context
                    pDecoder->Reset(pDecoderContext);
                }
            }
            break;
*/

        default:
            {
                //pContext->apDecoder[uWave] = NULL;
                //pContext->pDecoderContext = NULL;

                COMPONENT_ERROR("unknown wave data type");
            }
            break;
        }
    }

    //  create mixer
    if (m_pData->uInstances <= 2)
    {
        if (m_pData->uEffects == 0)
            pContext->pMixer = (Core::IMixer*)new Core::Mixer<4, 1, 1, 0>(Name());
        else if (m_pData->uEffects <= 8)
            pContext->pMixer = (Core::IMixer*)new Core::Mixer<4, 1, 1, 8>(Name());
        else if (m_pData->uEffects <= 128)
            pContext->pMixer = (Core::IMixer*)new Core::Mixer<4, 1, 1, 128>(Name());
        else
        {
            pContext->pMixer = NULL;
            COMPONENT_ERROR("could not create Core::Mixer - too many effects");
        }
    }
    else if (m_pData->uInstances <= 8)
    {
        if (m_pData->uEffects == 0)
            pContext->pMixer = (Core::IMixer*)new Core::Mixer<16, 1, 1, 0>(Name());
        else if (m_pData->uEffects <= 8)
            pContext->pMixer = (Core::IMixer*)new Core::Mixer<16, 1, 1, 8>(Name());
        else if (m_pData->uEffects <= 128)
            pContext->pMixer = (Core::IMixer*)new Core::Mixer<16, 1, 1, 128>(Name());
        else
        {
            pContext->pMixer = NULL;
            COMPONENT_ERROR("could not create Core::Mixer - too many effects");
        }
    }
    else if (m_pData->uInstances <= 32)
    {
        if (m_pData->uEffects == 0)
            pContext->pMixer = (Core::IMixer*)new Core::Mixer<64, 1, 1, 0>(Name());
        else if (m_pData->uEffects <= 8)
            pContext->pMixer = (Core::IMixer*)new Core::Mixer<64, 1, 1, 8>(Name());
        else if (m_pData->uEffects <= 128)
            pContext->pMixer = (Core::IMixer*)new Core::Mixer<64, 1, 1, 128>(Name());
        else
        {
            pContext->pMixer = NULL;
            COMPONENT_ERROR("could not create Core::Mixer - too many effects");
        }
    }
    else
    {
        pContext->pMixer = NULL;
        COMPONENT_ERROR("could not create Core::Mixer - too many instances");
    }

    //  create waves and connect to mixer
    for (U32 uInst = 0; uInst < m_pData->uInstances; uInst++)
    {
        pContext->apWaveA[uInst] = (Core::IWave*)new Core::Wave<1, 0>(Name());
        pContext->apWaveB[uInst] = (Core::IWave*)new Core::Wave<1, 0>(Name());

        Core::Decoder* pDecoderA = pContext->apDecoder[0];
        pContext->apWaveA[uInst]->SetDecoder(NULL, pDecoderA);

        Core::Decoder* pDecoderB = pContext->apDecoder[1];
        pContext->apWaveB[uInst]->SetDecoder(NULL, pDecoderB);

        pContext->pMixer->SetInput(2 * uInst, (Core::IGenerator*)((Core::Wave<1, 0>*)pContext->apWaveA[uInst]));
        pContext->pMixer->SetInput(2 * uInst + 1, (Core::IGenerator*)((Core::Wave<1, 0>*)pContext->apWaveB[uInst]));
        pContext->pMixer->SetGain(2 * uInst, 0, 0, 1.f);
        pContext->pMixer->SetGain(2 * uInst + 1, 0, 0, 1.f);
    }

    //  for callback
    pContext->pObjContext = pObjContext;
    pContext->pThis = this;

	for (U32 uWave = 0; uWave < m_pData->uWaves; uWave++)
	{
        for (U32 uInst = 0; uInst < m_pData->uInstances; uInst++)
        {
            //  set decoder callback2
			pContext->apDecoder[2 * (uWave * m_pData->uInstances + uInst)]->SetCallback(NULL, pContext, FinishedCallback);
			pContext->apDecoder[2 * (uWave * m_pData->uInstances + uInst) + 1]->SetCallback(NULL, pContext, FinishedCallback);
        }
    }

	pContext->bPlay = false;
//	pContext->bStop = false;
    pContext->uInstance = 0;
    pContext->fIndex = -1.f;
    pContext->fPitch = -1.f;
    pContext->fFrequency = -1.f;
    pContext->fGain = -1.f;
    pContext->fLevel = -1.f;
    pContext->uIndexA = -1;
    pContext->uIndexB = -1;
    pContext->fLevelA = 0.f;
    pContext->fLevelB = 0.f;
	pContext->uNumActive = 0;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::ReleaseContext(U8* pObjContext)
{
    MonoOneShotSoundTable::Context* pContext = (MonoOneShotSoundTable::Context*)&pObjContext[m_uOffset];

	if (pContext->apDecoder)
    {
        for (U32 uWave = 0; uWave < m_pData->uWaves; uWave++)
        {
            for (U32 uInst = 0; uInst < m_pData->uInstances; uInst++)
			{
                pContext->apDecoder[2 * (uWave * m_pData->uInstances + uInst)]->Shutdown(NULL);
                pContext->apDecoder[2 * (uWave * m_pData->uInstances + uInst) + 1]->Shutdown(NULL);

				delete pContext->apDecoder[2 * (uWave * m_pData->uInstances + uInst)];
				delete pContext->apDecoder[2 * (uWave * m_pData->uInstances + uInst) + 1];
			}
        }

        delete [] pContext->apDecoder;
        pContext->apDecoder = NULL;
    }

    if (pContext->apWaveA)
    {
        for (U32 uCount = 0; uCount < m_pData->uInstances; uCount++)
            delete pContext->apWaveA[uCount];

        delete [] pContext->apWaveA;
        pContext->apWaveA = NULL;
    }

    if (pContext->apWaveB)
    {
        for (U32 uCount = 0; uCount < m_pData->uInstances; uCount++)
            delete pContext->apWaveB[uCount];

        delete [] pContext->apWaveB;
        pContext->apWaveB = NULL;
    }

    if (pContext->pMixer)
    {
        delete pContext->pMixer;
        pContext->pMixer = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::Update(U8* pObjContext)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoOneShotSoundTable");

	bool bFrequencyChanged = false;
	bool bVolumeChanged = false;

    MonoOneShotSoundTable::Context* pContext = (MonoOneShotSoundTable::Context*)&pObjContext[m_uOffset];

	//  only recalc if 1st request this frame
	if (pContext->uLastFrame == System::Get()->GetFrameNum())
		return;

    //  get index
    F32 fIndex;
    if (m_pData->optIndex.ctrl.sCom != -1)
        fIndex = m_pObject->GetComponent(m_pData->optIndex.ctrl.sCom)->Value(pObjContext, m_pData->optIndex.ctrl.uOut);
    else
        fIndex = m_pData->optIndex.fVal;

    //  wraparound wave indices
    if (fIndex < 0.f)
        fIndex = fIndex + (S32)(1.f - fIndex / (F32)m_pData->uWaves) * m_pData->uWaves;
    else if (fIndex >= (F32)m_pData->uWaves)
        fIndex = fIndex - (S32)(fIndex / (F32)m_pData->uWaves) * m_pData->uWaves;

    //  set index
    if (pContext->fIndex != fIndex)
    {
        pContext->fIndex = fIndex;

        //  create x-fade blend
        pContext->uIndexA = (U32)fIndex;
        pContext->uIndexB = (pContext->uIndexA == m_pData->uWaves - 1) ? 0 : pContext->uIndexA + 1;
        F32 fBlendA = fIndex - (F32)pContext->uIndexA;
        F32 fBlendB = 1.f - fBlendA;
        pContext->fLevelA = sqrtf(fBlendB);
        pContext->fLevelB = sqrtf(fBlendA);
    }

    //  get frequency
    F32 fFrequency;
    if (m_pData->optFrequency.ctrl.sCom != -1)
        fFrequency = m_pObject->GetComponent(m_pData->optFrequency.ctrl.sCom)->Value(pObjContext, m_pData->optFrequency.ctrl.uOut);
    else
        fFrequency = m_pData->optFrequency.fVal;

    //  has changed
    if (pContext->fFrequency != fFrequency)
    {
        pContext->fFrequency = fFrequency;
		bFrequencyChanged = true;
    }

    //  get pitch
    F32 fPitch;
    if (m_pData->optPitch.ctrl.sCom != -1)
        fPitch = m_pObject->GetComponent(m_pData->optPitch.ctrl.sCom)->Value(pObjContext, m_pData->optPitch.ctrl.uOut);
    else
        fPitch = m_pData->optPitch.fVal;

    //  has changed
    if (pContext->fPitch != fPitch)
    {
        pContext->fPitch = fPitch;
        if (fPitch != 0.f)
            fFrequency *= powf(2.f, fPitch);
		bFrequencyChanged = true;
    }

    //  set pitch
    if (bFrequencyChanged)
    {
        //  set wave pitch
        for (U32 uCount = 0; uCount < m_pData->uInstances; uCount++)
        {
            pContext->apWaveA[uCount]->SetPitch(fFrequency);
            pContext->apWaveB[uCount]->SetPitch(fFrequency);
        }
    }

    //  get level
    F32 fLevel;
    if (m_pData->optLevel.ctrl.sCom != -1)
        fLevel = m_pObject->GetComponent(m_pData->optLevel.ctrl.sCom)->Value(pObjContext, m_pData->optLevel.ctrl.uOut);
    else
        fLevel = m_pData->optLevel.fVal;

    //  has changed
    if (pContext->fLevel != fLevel)
    {
        pContext->fLevel = fLevel;
		bVolumeChanged = true;
    }

    //  get gain
    F32 fGain;
    if (m_pData->optGain.ctrl.sCom != -1)
        fGain = m_pObject->GetComponent(m_pData->optGain.ctrl.sCom)->Value(pObjContext, m_pData->optGain.ctrl.uOut);
    else
        fGain = m_pData->optGain.fVal;

    //  has changed
    if (pContext->fGain != fGain)
    {
        pContext->fGain = fGain;
        if (fGain != 0.f)
            fLevel *= Maths::FloatTodB(fGain);

		bVolumeChanged = true;
    }

    //  set volume
    if (bVolumeChanged)
    {
        //  set wave volumes
        for (U32 uCount = 0; uCount < m_pData->uInstances; uCount++)
        {
            pContext->apWaveA[uCount]->SetVolume(fLevel);
            pContext->apWaveB[uCount]->SetVolume(fLevel);
        }
    }

#ifdef AUDIO_DEFER_SOUNDS
	if (pContext->bPlay)
	{
		pContext->bPlay = false;
		Play(pObjContext);
	}
//	else if (pContext->bStop)
//	{
//		pContext->bStop = false;
//		Stop(pObjContext);
//	}
#endif // AUDIO_DEFER_SOUNDS

	pContext->uLastFrame = System::Get()->GetFrameNum();
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::MonoOneShotSoundTable::Value(U8* pObjContext, U32 uOutput)
{
    MonoOneShotSoundTable::Context* pContext = (MonoOneShotSoundTable::Context*)&pObjContext[m_uOffset];

    switch(uOutput)
    {
    case EntriesCtrl:
        return (F32)m_pData->uWaves;
        break;

    case ActiveCtrl:
		return (F32)pContext->uNumActive;
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
void Audio::MonoOneShotSoundTable::DoMessage(U8* pObjContext, U32 uMessage)
{
    MonoOneShotSoundTable::Context* pContext = (MonoOneShotSoundTable::Context*)&pObjContext[m_uOffset];

    switch (uMessage)
    {
    case PlayMsg:
		{
#ifdef AUDIO_DEFER_SOUNDS
//			if (pContext->bStop)
//				pContext->bStop = false;

			pContext->bPlay = true;
#else
			Update(pObjContext);
			Play(pObjContext);
#endif // AUDIO_DEFER_SOUNDS
		}
        break;

    case StopMsg:
		{
//#ifdef AUDIO_DEFER_SOUNDS
//			if (pContext->bPlay)
//				pContext->bPlay = false;

//			pContext->bStop = true;
//#else
//			Update(pObjContext);
			Stop(pObjContext);
//#endif // AUDIO_DEFER_SOUNDS
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
void Audio::MonoOneShotSoundTable::ConfigureAudio(U8* pObjContext)
{
    MonoOneShotSoundTable::Context* pContext = (MonoOneShotSoundTable::Context*)&pObjContext[m_uOffset];

    //  do effects
    for (U32 uEffect = 0; uEffect < m_pData->uEffects; uEffect++)
    {
        if (m_aEffects[uEffect] != (U32)-1)
            pContext->pMixer->SetEffect(uEffect, (Core::DSP::Effect*)m_pObject->GetComponent(m_aEffects[uEffect])->DSPEffect(pObjContext));
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Play
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::Play(U8* pObjContext)
{
    MonoOneShotSoundTable::Context* pContext = (MonoOneShotSoundTable::Context*)&pObjContext[m_uOffset];

    Core::Decoder* pDecoderA = pContext->apDecoder[2 * (pContext->uIndexA * m_pData->uInstances)];
    pContext->apWaveA[pContext->uInstance]->SetDecoder(NULL, pDecoderA);

    Core::Decoder* pDecoderB = pContext->apDecoder[2 * (pContext->uIndexB * m_pData->uInstances) + 1];
    pContext->apWaveB[pContext->uInstance]->SetDecoder(NULL, pDecoderB);

    //  set x-fades
    pContext->pMixer->SetGain(2 * pContext->uInstance, 0, 0, pContext->fLevelA);
    pContext->pMixer->SetGain(2 * pContext->uInstance + 1, 0, 0, pContext->fLevelB);

    //  play it

	if (! pContext->apWaveA[pContext->uInstance]->IsPlaying())
		pContext->uNumActive++;

    pContext->apWaveA[pContext->uInstance]->Reset(0);
    pContext->apWaveA[pContext->uInstance]->Start();

	if (! pContext->apWaveB[pContext->uInstance]->IsPlaying())
		pContext->uNumActive++;

    pContext->apWaveB[pContext->uInstance]->Reset(0);
    pContext->apWaveB[pContext->uInstance]->Start();

    //  move to next sound instance
    pContext->uInstance++;

    //  reset
    if (pContext->uInstance >= m_pData->uInstances)
        pContext->uInstance = 0;

	//AUDIO_SYS_DEBUG(MonoOneShotSoundTable::Name());//return oneshot sound
}

////////////////////////////////////////////////////////////////////////////////
//	Stop
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::Stop(U8* pObjContext)
{
    MonoOneShotSoundTable::Context* pContext = (MonoOneShotSoundTable::Context*)&pObjContext[m_uOffset];

    //  stop all
    for (U32 uCount = 0; uCount < m_pData->uInstances; uCount++)
    {
        pContext->apWaveA[uCount]->Stop();
        pContext->apWaveB[uCount]->Stop();
    }

    for (U32 uWave = 0; uWave < m_pData->uWaves; uWave++)
    {
        for (U32 uInst = 0; uInst < m_pData->uInstances; uInst++)
		{
            pContext->apDecoder[2 * (uWave * m_pData->uInstances + uInst)]->Shutdown(NULL);
            pContext->apDecoder[2 * (uWave * m_pData->uInstances + uInst) + 1]->Shutdown(NULL);
		}
    }

	pContext->uNumActive = 0;
}

////////////////////////////////////////////////////////////////////////////////
//	Finished
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::Finished(U8* pObjContext)
{
    MonoOneShotSoundTable::Context* pContext = (MonoOneShotSoundTable::Context*)&pObjContext[m_uOffset];

    if (m_pData->sFinishedScr != -1)
    {
        if (m_pObject->GetComponent(m_pData->sFinishedScr)->Flags() & Component::flagEvent)
            m_pObject->GetComponent(m_pData->sFinishedScr)->Execute(pObjContext);
        else
            COMPONENT_WARN("MonoOneShotSoundTable::Finished not an event");
    }

	pContext->uNumActive--;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOut
////////////////////////////////////////////////////////////////////////////////
const Audio::Core::IGenerator* Audio::MonoOneShotSoundTable::AudioOut(U8* pObjContext) const
{
    MonoOneShotSoundTable::Context* pContext = (MonoOneShotSoundTable::Context*)&pObjContext[m_uOffset];

    return pContext->pMixer->GeneratorOutput();
}

////////////////////////////////////////////////////////////////////////////////
//	Release
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOneShotSoundTable::Release()
{
    //  clunkity clunk! clunkity clunk! TODO: need to split Prime() into 2 funcs so we don't need to call Release() for each wave
    for (U32 uWave = 0; uWave < m_pData->uWaves; uWave++)
    {
		m_pObject->GetWaveManager()->Release(m_pObject->GetString(m_pData->strBank));
	}
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF
