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
//	(constructor)
////////////////////////////////////////////////////////////////////////////////
Audio::CoordinateInput::CoordinateInput(Object* pObject) : Component(pObject)
{
}

////////////////////////////////////////////////////////////////////////////////
//	(destructor)
////////////////////////////////////////////////////////////////////////////////
Audio::CoordinateInput::~CoordinateInput()
{
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::CoordinateInput::Init( void* pData )
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::CoordinateInput::InitContext( Instance* pInstance )
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

    pState->pInstanceOut = NULL;
    pState->uCoordinateOutput = -1;
	pState->vCoords = Maths::MakeVector3( 0.f, 0.f, 0.f );
	pState->fMagnitude = 0.f;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::CoordinateInput::Value( Instance* pInstance, U32 uOutput )
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
void Audio::CoordinateInput::Coords(Instance* pInstance, Maths::Vector* pVector)
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	*pVector = pState->vCoords;
}

////////////////////////////////////////////////////////////////////////////////
//	ProcessConnection
////////////////////////////////////////////////////////////////////////////////
void Audio::CoordinateInput::ProcessConnection( Instance* pInstance )
{
	U8* pStateData = pInstance->GetStateData();
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	if (pState->uCoordinateOutput != -1)
	{
		pState->pInstanceOut->GetObject()->GetComponentCoords( pState->uCoordinateOutput, pState->pInstanceOut, &pState->vCoords );
		pState->fMagnitude = _float( !pState->vCoords );
	}
	//else
	//    pState->f32Value = MakeVector3( 0.f, 0.f, 0.f );
}

////////////////////////////////////////////////////////////////////////////////
//	SetOutput
////////////////////////////////////////////////////////////////////////////////
void Audio::CoordinateInput::SetOutput( Instance* pThisInstance, Instance* pThatInstance, Audio::Com uCoordinateOutput )
{
    State* pState = (State*)pThisInstance->GetStateData( m_uStateOffset );

    pState->pInstanceOut = pThatInstance;
    pState->uCoordinateOutput = uCoordinateOutput;
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::CoordinateInput::GetOtherInstance( Instance* pThisInstance )
{
    State* pState = (State*)pThisInstance->GetStateData( m_uStateOffset );

	return pState->pInstanceOut;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::CoordinateInput::GetOtherComponent( Instance* pThisInstance )
{
    State* pState = (State*)pThisInstance->GetStateData( m_uStateOffset );

	return pState->uCoordinateOutput;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
