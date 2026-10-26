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
//	destructor
////////////////////////////////////////////////////////////////////////////////
Audio::MonoAudioInput::~MonoAudioInput()
{
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::MonoAudioInput::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoAudioInput::CoreMemory( U32 uOffset )
{
	m_uInput = uOffset;

	return uOffset + Core::Input::InputSize( 1, 0 );
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoAudioInput::InitContext( Instance* pInstance )
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );
	U8* pCoreData = pInstance->GetCoreData();

	Core::Input* pInput = new( &pCoreData[ m_uInput ] ) Core::Input( Name(), 1, 0 );

	pState->pFrameInput = (F32*)System::Get()->Alloc( Core::uFrameLength * sizeof(F32), 128 );
	pInput->SetAltBuffer( pState->pFrameInput );

	Frame::Zero( pState->pFrameInput );

	pState->bActive = true;
	pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoAudioInput::ReleaseContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);
	U8* pCoreData = pInstance->GetCoreData();

	System::Get()->Free(pState->pFrameInput);

	Core::Input* pInput = (Core::Input*)&pCoreData[m_uInput];
	pInput->~Input();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoAudioInput::Update( Instance* pInstance )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoAudioInput");

	State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
	Core::Input* pInput = (Core::Input*)pInstance->GetCoreData( m_uInput );

	pState->bActive = pInput->IsActive();

	pInput->SetMuted( pState->bMuted );
}

////////////////////////////////////////////////////////////////////////////////
//	AudioInput
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Input* Audio::MonoAudioInput::AudioInput(Instance* pInstance)
{
    return (Core::Input*)pInstance->GetCoreData( m_uInput );
}

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	GetOtherInstance
////////////////////////////////////////////////////////////////////////////////
Audio::Instance* Audio::MonoAudioInput::GetOtherInstance( Instance* pThisInstance )
{
    State* pState = (State*)pThisInstance->GetStateData( m_uStateOffset );

	return NULL;//pState->pInstanceOut;
}

////////////////////////////////////////////////////////////////////////////////
//	GetOtherComponent
////////////////////////////////////////////////////////////////////////////////
Audio::Com Audio::MonoAudioInput::GetOtherComponent( Instance* pThisInstance )
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::MonoAudioInput::IsActive( Instance* pInstance )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoAudioInput::SetMute( Instance* pInstance, bool bMuted )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	pState->bMuted = bMuted;
}
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
