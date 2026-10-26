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
void* Audio::StereoSaturator::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoSaturator::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::Saturator));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSaturator::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::Saturator( Name(), 2 );

	pState->fDrive = -1.f;
	pState->fClip = -1.f;
	pState->fLevel = -1.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSaturator::ReleaseContext(Instance* pInstance)
{
	Core::DSP::Saturator* pDSP = (Core::DSP::Saturator*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSaturator::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoSaturator");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::Saturator* pDSP = (Core::DSP::Saturator*)pInstance->GetCoreData( m_uDSP );

    //  get drive
	F32 fDrive = GetOptionValue( pInstance, pInitData->optDrive );

    //  set drive
	if( pState->fDrive != fDrive )
    {
        pState->fDrive = fDrive;
		pDSP->SetParam( Core::DSP::Saturator::eDrive, Maths::dBToFloat( fDrive ) );
    }

    //  get clip
	F32 fClip = GetOptionValue( pInstance, pInitData->optClip );

    //  set clip
	if( pState->fClip != fClip )
    {
        pState->fClip = fClip;
		pDSP->SetParam( Core::DSP::Saturator::eClip, fClip );
    }

    //  get level
	F32 fLevel = GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

    //  has changed
    if( pState->fLevel != fLevel )
    {
        pState->fLevel = fLevel;
		pDSP->SetParam( Core::DSP::Saturator::eLevel, fLevel );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSaturator::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::Saturator* pDSP = (Core::DSP::Saturator*)pInstance->GetCoreData( m_uDSP );

	switch( uMessage )
    {
    case EnableMsg:
		pDSP->SetActive( true );
        break;

    case BypassMsg:
		pDSP->SetActive( false );
        break;

    default:
        Assert(0);
        break;
    }
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
