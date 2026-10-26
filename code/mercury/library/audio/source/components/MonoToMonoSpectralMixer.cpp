#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#endif

//using namespace Audio;
/*
////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* MonoToMonoSpectralMixer::Init(void* pData)
{
	m_uInitOffset = (U8*)pData - (U8*)m_pObject->GetInitData();
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);

	m_pchName = m_pObject->GetString(pInitData->strName);

	m_uEffectOffset = m_uInitOffset + sizeof(Data);

	m_uInputOffset = m_uEffectOffset + pInitData->uEffects * sizeof(Com);

	U32 uNext = m_uInputOffset + pInitData->uInputs * sizeof(InputGain);

	return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void MonoToMonoSpectralMixer::InitContext(Instance* pInstance)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    //  create mixer
    //m_pMixer = CreateMonoToMonoSpectralMixer(pInitData->uInputs, pInitData->uEffects + 1);
    if (pState->pMixer == NULL)
        COMPONENT_ERROR("could not create Core::Mixer - too many inputs/effects requested?");
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void MonoToMonoSpectralMixer::ReleaseContext(Instance* pInstance)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    if (pState->pMixer)
    {
        delete pState->pMixer;
        pState->pMixer = NULL;
    }

    pState = NULL;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void MonoToMonoSpectralMixer::Update(Instance* pInstance)
{
    //for (U32 uInput = 0; uInput < pInitData->uInputs; uInput++)
    //{
    //    //  get gain
    //    F32 fGain;
    //    if (aInputs[uInput].sGainCom != -1)
    //        fGain = m_pObject->GetComponentValue(aInputs[uInput].sGainCom, pStateData, aInputs[uInput].uGainOut);
    //    else
    //        fGain = aInputs[uInput].fGainVal;

    //    //  set gain
    //    if( pState->fGain != fGain )
    //    {
    //        pState->fGain = fGain;
    //        m_pMixer->SetGain(uInput, 0, 0, fGain);
    //    }
    //}
}

*/






