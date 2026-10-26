#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif
#include <stdio.h>

//using namespace Audio;

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::MonoConvolution::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

//    U32 uInpulseLengthInFrames = ((pInitData->uImpulseLength - 1) / Core::uFrameLength) + 1;

    //m_pDSP = CreateMonoDSP48kHzConvolution(uInpulseLengthInFrames);

    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoConvolution::CoreMemory(U32 uOffset)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoConvolution::InitContext(Instance* pInstance)
{
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoConvolution::Update(Instance* pInstance)
{
    //if (m_pImpulse == NULL)
    {
        //pInitData->pWaveServer->Get(pInitData->strBank, pInitData->pchWave, &m_pImpulse);
        //const Core::WaveData* pImpulse = m_pObject->GetComponentValue(pInitData->uImpulseCom)->GetWaveData();
//        if (pImpulse)
//            m_pDSP->SetImpulse(0, pImpulse);
    }
}

