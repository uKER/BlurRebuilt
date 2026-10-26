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
void* Audio::StateMachine::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);

	Assert(pInitData->ctrlNextState.sCom != -1);
    Assert(pInitData->uNumStates >= 2);

	m_uStateItemOffset = m_uInitOffset + sizeof(Data);

	U32 uNext = m_uStateItemOffset + pInitData->uNumStates * sizeof(StateItem);

    return (void*)((U8*)m_pObject->GetInitData() + uNext);
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StateMachine::InitContext(Instance* pInstance)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    //  get initial state
	pState->sCurrState = pInitData->sInitialState;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::StateMachine::Value(Instance* pInstance, U32 uOutput)
{
	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	StateItem* aStateItems = (StateItem*)m_pObject->GetInitData(m_uStateItemOffset);
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

    switch(uOutput)
    {
    case ValueCtrl:
	    return (F32)pState->sCurrState;
        break;

    default:
        Assert( 0 );
        break;
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::StateMachine::DoMessage(Instance* pInstance, U32 uMessage)
{
	switch (uMessage)
    {
    case IncrementMsg:
        Increment( pInstance );
        break;

    case DecrementMsg:
        Decrement( pInstance );
        break;

    case ChangeMsg:
        Change( pInstance );
        break;

    case ResetMsg:
        Reset( pInstance );
        break;

    default:
        Assert( 0 );
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Increment
////////////////////////////////////////////////////////////////////////////////
void Audio::StateMachine::Increment( Instance* pInstance )
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

	//	do if can increment
	if( pState->sCurrState + 1 < pInitData->uNumStates )
		ChangeState( pInstance, pState->sCurrState + 1 );
}

////////////////////////////////////////////////////////////////////////////////
//	Decrement
////////////////////////////////////////////////////////////////////////////////
void Audio::StateMachine::Decrement( Instance* pInstance )
{
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

	//	do if can decrement
	if( pState->sCurrState > 0 )
		ChangeState( pInstance, pState->sCurrState - 1 );
}

////////////////////////////////////////////////////////////////////////////////
//	Change
////////////////////////////////////////////////////////////////////////////////
void Audio::StateMachine::Change( Instance* pInstance )
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

    //  get next state
	S32 sNextState = (S32)Maths::Round( GetControlValue( pInstance, pInitData->ctrlNextState ) );

	//	clamp
	sNextState = Maths::Clamp( sNextState, 0, pInitData->uNumStates - 1 );

	//	do if different to current state
	if( sNextState != pState->sCurrState )
		ChangeState( pInstance, sNextState );
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::StateMachine::Reset( Instance* pInstance )
{
	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

	//	if different to current state
	if( pInitData->sInitialState != pState->sCurrState )
		ChangeState( pInstance, pInitData->sInitialState );
}

////////////////////////////////////////////////////////////////////////////////
//	Reset
////////////////////////////////////////////////////////////////////////////////
void Audio::StateMachine::ChangeState( Instance* pInstance, S32 sNextState )
{
	StateItem* aStateItems = (StateItem*)m_pObject->GetInitData( m_uStateItemOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

	//	fire exit current state event
	if( aStateItems[ pState->sCurrState ].evExit.sCom != Component::eUndefined )
		DoEvent( pInstance, aStateItems[ pState->sCurrState ].evExit );

	//	update state
	pState->sCurrState = sNextState;

	//	fire enter next state event
	if( aStateItems[ pState->sCurrState ].evEnter.sCom != Component::eUndefined )
		DoEvent( pInstance, aStateItems[ pState->sCurrState ].evEnter );
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

