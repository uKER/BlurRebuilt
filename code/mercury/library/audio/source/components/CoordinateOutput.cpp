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
void* Audio::CoordinateOutput::Init( void* pData )
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::CoordinateOutput::InitContext( Instance* pInstance )
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

    pState->pInstanceIn = NULL;
    pState->uCoordinateInput = -1;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::CoordinateOutput::Value( Instance* pInstance, U32 uOutput )
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

    switch( uOutput )
    {
    case MagnitudeCtrl:
		return pState->fMagnitude;
        break;
    case XCtrl:
		return _float( X( pState->vCoords ) );
        break;
	case YCtrl:
		return _float( Y( pState->vCoords ) );
        break;
	case ZCtrl:
		return _float( Z( pState->vCoords ) );
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
void Audio::CoordinateOutput::Coords(Instance* pInstance, Maths::Vector* pVector)
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );

	return m_pObject->GetComponentCoords( pInitData->sInputCom, pInstance, pVector );
}

////////////////////////////////////////////////////////////////////////////////
//	SetInput
////////////////////////////////////////////////////////////////////////////////
void Audio::CoordinateOutput::SetInput( Instance* pThisInstance, Instance* pThatInstance, Audio::Com uCoordinateInput )
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

    pState->pInstanceIn = pThatInstance;
    pState->uCoordinateInput = uCoordinateInput;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::CoordinateOutput::GetOtherInstance(Instance* pThisInstance)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

	return pState->pInstanceIn;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::CoordinateOutput::GetOtherComponent(Instance* pThisInstance)
{
    State* pState = (State*)pThisInstance->GetStateData(m_uStateOffset);

	return pState->uCoordinateInput;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
