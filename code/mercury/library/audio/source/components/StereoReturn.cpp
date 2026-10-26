#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::StereoReturn::Init( void* pData )
{
	Data* pInitData = PreInit<Data>( pData );

	m_uEffectOffset = m_uInitOffset + sizeof( Data );

	U32 uNext = m_uEffectOffset + pInitData->uEffects * sizeof( U32 );

	m_uNumEffects = pInitData->uEffects;

	return ( void* )( ( U8* )m_pObject->GetInitData() + uNext );
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoReturn::CoreMemory( U32 uOffset )
{
	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
	Com* aEffects = ( Com* )m_pObject->GetInitData( m_uEffectOffset );

	m_uReturn = uOffset;

	uOffset += Core::Return::ReturnSize( 1, 0 );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoReturn::InitContext( Instance* pInstance )
{
	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
    State* pState = ( State* )pInstance->GetStateData( m_uStateOffset );
	Core::Return* pReturn = ( Core::Return* )pInstance->GetCoreData( m_uReturn );

	new( pReturn ) Core::Return( Name(), 1, GetNumDSPSlots() );

    pState->fLevel = -1.f;

    pState->bActive = false;
    pState->bMuted = false;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoReturn::ReleaseContext( Instance* pInstance )
{
	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
	Core::Return* pReturn = ( Core::Return* )pInstance->GetCoreData( m_uReturn );

	pReturn->~Return();
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoReturn::Update( Instance* pInstance )
{
	AUDIO_PERF_MICRO_EVENT( "Audio.Update.StereoReturn" );

	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
	State* pState = ( State* )pInstance->GetNewStateData( m_uStateOffset );
	Core::Return* pReturn = ( Core::Return* )pInstance->GetCoreData( m_uReturn );
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::StereoReturn::Value(Instance* pInstance, U32 uOutput)
{
    State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

    switch(uOutput)
    {
    case ActiveCtrl:
		return pState->bActive ? 1.f : 0.f;
        break;

    default:
        Assert(0);
        break;
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ConfigureAudio
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::StereoReturn::ConfigureAudio( Instance* pInstance )
{
	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
	Com* aEffects = ( Com* )m_pObject->GetInitData( m_uEffectOffset );
	U8* pStateData = pInstance->GetStateData();
	Core::Return* pReturn = ( Core::Return* )pInstance->GetCoreData( m_uReturn );
    
    //  do effects
    for( U32 uEffect = 0, uSlot = 0; uEffect < pInitData->uEffects; uEffect++ )
    {
        if( aEffects[ uEffect ] != sUnused )
        {
			U32 uNumUnits = m_pObject->GetComponent( aEffects[ uEffect ] )->DSPEffectCount( );

			for( U32 uUnit = 0; uUnit < uNumUnits; uUnit++ )
			{
				pReturn->SetEffect( uSlot + uUnit, m_pObject->GetComponent( aEffects[ uEffect ] )->DSPEffectType( pStateData, uUnit ), 
										m_pObject->GetComponent( aEffects[ uEffect ] )->DSPEffectUnit( pStateData, uUnit ) );

				uSlot += uNumUnits;
			}
        }
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	AudioOutType
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::StereoReturn::AudioOutType( Instance* pInstance ) const
{
	return Core::eReturn;
}

////////////////////////////////////////////////////////////////////////////////
//	AudioOutOffset
////////////////////////////////////////////////////////////////////////////////
const U32 Audio::StereoReturn::AudioOutOffset( Instance* pInstance ) const
{
	return m_uReturn;
}

////////////////////////////////////////////////////////////////////////////////
//	Release
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::StereoReturn::Release( )
{
}
#endif	//	#ifndef PLATFORM_SPU

#if defined (AUDIO_ENABLE_PROFILING)
////////////////////////////////////////////////////////////////////////////////
//	IsActive
////////////////////////////////////////////////////////////////////////////////
bool Audio::StereoReturn::IsActive( Instance* pInstance )
{
	State* pState = ( State* )pInstance->GetStateData( m_uStateOffset );

	return pState->bActive;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMute
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoReturn::SetMute( Instance* pInstance, bool bMuted )
{
	State* pState = ( State* )pInstance->GetStateData( m_uStateOffset );

	pState->bMuted = bMuted;
}
#endif	//	AUDIO_ENABLE_PROFILING

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
