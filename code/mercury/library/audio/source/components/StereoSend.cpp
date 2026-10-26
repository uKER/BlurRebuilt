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
void* Audio::StereoSend::Init( void* pData )
{
	Data* pInitData = PreInit< Data >( pData );
    return ( void* )( pInitData + 1 );
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoSend::CoreMemory( U32 uOffset )
{
	m_uDSP = ( ixCore )uOffset;

	uOffset += AUDIO_CORE_ALIGN_16( sizeof( Core::DSP::Send ) );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSend::InitContext( Instance* pInstance )
{
    State* pState = ( State* )pInstance->GetStateData( m_uStateOffset );

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::Send( Name(), 1 );

    pState->fLevel = -1.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSend::ReleaseContext( Instance* pInstance )
{
	Core::DSP::Send* pDSP = (Core::DSP::Send*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSend::Update( Instance* pInstance )
{
	AUDIO_PERF_MICRO_EVENT( "Audio.Update.StereoSend" );

	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
    State* pState = ( State* )pInstance->GetNewStateData( m_uStateOffset );
	Core::DSP::Send* pDSP = ( Core::DSP::Send* )pInstance->GetCoreData( m_uDSP );

	//  get level
	F32 fLevel = GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

    //  set level
    if( pState->fLevel != fLevel )
    {
        pState->fLevel = fLevel;
		pDSP->SetParam( Core::DSP::Send::eLevel, fLevel );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSend::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::Effect* pDSP = ( Core::DSP::Effect* )pInstance->GetCoreData( m_uDSP );

	switch( uMessage )
    {
    case EnableMsg:
		pDSP->SetActive( true );
        break;

    case BypassMsg:
		pDSP->SetActive( false );
        break;

    default:
        Assert( 0 );
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	ConfigureAudio
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void Audio::StereoSend::ConfigureAudio( Instance* pInstance )
{
	Data* pInitData = ( Data* )m_pObject->GetInitData( m_uInitOffset );
    State* pState = ( State* )pInstance->GetStateData( m_uStateOffset );
	Core::DSP::Send* pSend = ( Core::DSP::Send* )pInstance->GetCoreData( m_uDSP );

	Component* pCom = m_pObject->GetComponent( pInitData->comReturn );

	if( pCom->TypeId() == eStereoReturn )
		pSend->SetReturn( ( ( StereoReturn* )pCom )->Return( ) );
}
#endif	//	#ifndef PLATFORM_SPU


