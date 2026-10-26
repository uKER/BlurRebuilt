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

//using namespace Audio;

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::SurroundAudioInput::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::SurroundAudioInput::CoreMemory( U32 uOffset )
{
	m_uInput = uOffset;

	return uOffset + Core::Input::InputSize( 5, 0 );
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundAudioInput::InitContext( Instance* pInstance )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	Core::Input* pInput = new( pInstance->GetCoreData( m_uInput ) ) Core::Input( Name(), 5, 0 );

	pState->pFrameInput = (F32*)System::Get()->Alloc(5 * Core::uFrameLength * sizeof(F32), 128);
	pInput->SetAltBuffer(pState->pFrameInput);

	for (U32 uCh = 0; uCh < 5; uCh++)
		Frame::Zero(&pState->pFrameInput[uCh * Core::uFrameLength]);

	pState->bActive = true;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundAudioInput::ReleaseContext(Instance* pInstance)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];
	U8* pCoreData = pInstance->GetCoreData();

	System::Get()->Free(pState->pFrameInput);

	Core::Input* pInput = (Core::Input*)&pCoreData[m_uInput];
	pInput->~Input();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundAudioInput::Update( Instance* pInstance )
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.SurroundAudioInput");

	State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
	Core::Input* pInput = (Core::Input*)pInstance->GetCoreData( m_uInput );

	pState->bActive = pInput->IsActive();

	pInput->SetMuted( pState->bMuted );
}

////////////////////////////////////////////////////////////////////////////////
//	AudioInput
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Input* Audio::SurroundAudioInput::AudioInput(Instance* pInstance)
{
    return (Core::Input*)pInstance->GetCoreData( m_uInput );
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::SurroundAudioInput::GetOtherInstance( Instance* pThisInstance )
{
    State* pState = (State*)pThisInstance->GetStateData( m_uStateOffset );

	return NULL;//pState->pInstanceOut;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::SurroundAudioInput::GetOtherComponent( Instance* pThisInstance )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::SurroundAudioInput::IsActive( Instance* pInstance )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundAudioInput::SetMute( Instance* pInstance, bool bMuted )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	pState->bMuted = bMuted;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
