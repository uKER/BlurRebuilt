#include "AudioPch.h"
#ifdef SN_TARGET_PS3_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	(destructor)
////////////////////////////////////////////////////////////////////////////////
Audio::VectorOut::~VectorOut()
{
}

#ifndef PLATFORM_SPU
////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void* Audio::VectorOut::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

    //Assert(pInitData->sInputCom != -1);
    //Assert((m_pObject->GetComponent(pInitData->sInputCom)->Flags() & Component::flagMono) != 0);

    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
#ifndef SN_TARGET_PS3_SPU
void Audio::VectorOut::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

	pState->fValueX = pInitData->fInputXVal;
	pState->fValueY = pInitData->fInputYVal;
	pState->fValueZ = pInitData->fInputZVal;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::VectorOut::ReleaseContext(Instance* pInstance)
{
}
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::VectorOut::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.VectorOut");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    //  get input
	F32 fValueX;
    if (pInitData->sInputXCom != -1)
        fValueX = m_pObject->GetComponentValue(pInitData->sInputXCom, pInstance, pInitData->uInputXOut);
    else
        fValueX = pInitData->fInputXVal;

	    //  get input
	F32 fValueY;
    if (pInitData->sInputYCom != -1)
        fValueY = m_pObject->GetComponentValue(pInitData->sInputYCom, pInstance, pInitData->uInputYOut);
    else
        fValueY = pInitData->fInputYVal;

	    //  get input
	F32 fValueZ;
    if (pInitData->sInputZCom != -1)
        fValueZ = m_pObject->GetComponentValue(pInitData->sInputZCom, pInstance, pInitData->uInputZOut);
    else
        fValueZ = pInitData->fInputZVal;

	pState->fValueX = fValueX; 
	pState->fValueY = fValueY; 
	pState->fValueZ = fValueZ; 
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
void Audio::VectorOut::GetDimensions(Instance* pInstance, F32* valueX,F32* valueY,F32* valueZ)
{
	F32 fValueX;
	F32 fValueY;
	F32 fValueZ;

	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	fValueX = pState->fValueX;
	fValueY = pState->fValueY;
	fValueZ = pState->fValueZ;

	*valueX = fValueX;
	*valueY = fValueY;
	*valueZ = fValueZ;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
