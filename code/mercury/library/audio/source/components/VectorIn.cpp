#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

//using namespace Audio;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::VectorIn::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::VectorIn::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    pState->fX = pInitData->fX;
    pState->fY = pInitData->fY;
    pState->fZ = pInitData->fZ;
    pState->fMagnitude = Maths::Sqrt(pState->fX * pState->fX + pState->fY * pState->fY + pState->fZ * pState->fZ);
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::VectorIn::Value(Instance* pInstance, U32 uOutput)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];

    switch(uOutput)
    {
    case MagnitudeCtrl:
        return pState->fMagnitude;
        break;

    case XCtrl:
        return pState->fX;
        break;

    case YCtrl:
        return pState->fY;
        break;

    case ZCtrl:
        return pState->fZ;
        break;

    default:
        Assert(0);
        break;
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	Coords
////////////////////////////////////////////////////////////////////////////////
void Audio::VectorIn::Coords(Instance* pInstance, Audio::Maths::Vector* pVector)
{
    Assert(pVector != NULL);

    //State* pState = (State*)&pStateData[m_uStateOffset];
	State* pState = (State*)(pInstance->GetStateData(m_uStateOffset));

    if (pVector)
    {
        //pVector->x = pState->fX;
        //pVector->y = pState->fY;
        //pVector->z = pState->fZ;
        *pVector = Maths::MakeVector3( pState->fX, pState->fY, pState->fZ );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	SetDimensions
////////////////////////////////////////////////////////////////////////////////
bool Audio::VectorIn::SetDimensions(Instance* pInstance, F32* afDimensions)
{
	State* pState = (State*)(pInstance->GetStateData(m_uStateOffset));

	pState->fX = afDimensions[0];
	pState->fY = afDimensions[1];
	pState->fZ = afDimensions[2];

    pState->fMagnitude = Maths::Sqrt(afDimensions[0] * afDimensions[0] + afDimensions[1] * afDimensions[1] + afDimensions[2] * afDimensions[2]);

	return true;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

