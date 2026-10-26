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
void* Audio::MonoDistortion::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoDistortion::CoreMemory(U32 uOffset)
{
	m_uDSP[0] = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::IIRLowShelf));

	m_uDSP[1] = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::Saturator));

	m_uDSP[2] = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::IIRLowShelf));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoDistortion::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP[ 0 ] ) ) Core::DSP::IIRLowShelf( Name(), 1 );
	Core::DSP::Saturator* pDSP1 = new( pInstance->GetCoreData( m_uDSP[ 1 ] ) ) Core::DSP::Saturator( Name(), 1 );
	new( pInstance->GetCoreData( m_uDSP[ 2 ] ) ) Core::DSP::IIRLowShelf( Name(), 1 );

	pDSP1->SetParam( Core::DSP::Saturator::eClip, 1.f );

    pState->fDrive = -1.f;
    pState->fCutLevel = -1.f;
    pState->fCutoff = -1.f;
    pState->fSlope = -1.f;
    pState->fBoostLevel = -1.f;
    pState->fLevel = -1.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoDistortion::ReleaseContext(Instance* pInstance)
{
	U8* pCoreData = pInstance->GetCoreData();

	Core::DSP::IIRLowShelf* pDSP0 = (Core::DSP::IIRLowShelf*)pInstance->GetCoreData( m_uDSP[0] );
	Core::DSP::Saturator* pDSP1 = (Core::DSP::Saturator*)pInstance->GetCoreData( m_uDSP[1] );
	Core::DSP::IIRLowShelf* pDSP2 = (Core::DSP::IIRLowShelf*)pInstance->GetCoreData( m_uDSP[2] );

	delete pDSP0;
	delete pDSP1;
	delete pDSP2;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoDistortion::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoDistortion");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
    State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );

	Core::DSP::IIRLowShelf* pDSP0 = (Core::DSP::IIRLowShelf*)pInstance->GetCoreData( m_uDSP[ 0 ] );
	Core::DSP::Saturator* pDSP1 = (Core::DSP::Saturator*)pInstance->GetCoreData( m_uDSP[ 1 ] );
	Core::DSP::IIRLowShelf* pDSP2 = (Core::DSP::IIRLowShelf*)pInstance->GetCoreData( m_uDSP[ 2 ] );

	//  get cut level
	F32 fCutLevel = Maths::Max( GetLevel( pInstance, pInitData->optCutLevel, pInitData->optCutGain ), 0.f );

    //  has changed
    if( pState->fCutLevel != fCutLevel )
    {
        pState->fCutLevel = fCutLevel;
		pDSP0->SetParam( Core::DSP::IIRLowShelf::eGain, fCutLevel );
    }

    //  get cutoff
	F32 fCutoff = GetOptionValue( pInstance, pInitData->optCutoff );

	//  has changed
    if( pState->fCutoff != fCutoff )
    {
		pState->fCutoff = fCutoff;
		pDSP0->SetParam( Core::DSP::IIRLowShelf::eCutoff, fCutoff );
		pDSP2->SetParam( Core::DSP::IIRLowShelf::eCutoff, fCutoff );
	}

    //  get slope
	F32 fSlope = GetOptionValue( pInstance, pInitData->optSlope );

	//  has changed
    if( pState->fSlope != fSlope )
    {
		pState->fSlope = fSlope;
		pDSP0->SetParam( Core::DSP::IIRLowShelf::eSlope, fSlope );
		pDSP2->SetParam( Core::DSP::IIRLowShelf::eSlope, fSlope );
	}

	//  get boost level
	F32 fBoostLevel = Maths::Max( GetLevel( pInstance, pInitData->optBoostLevel, pInitData->optBoostGain ), 0.f );

    //  has changed
    if( pState->fBoostLevel != fBoostLevel )
    {
        pState->fBoostLevel = fBoostLevel;
		pDSP2->SetParam( Core::DSP::IIRLowShelf::eGain, fBoostLevel );
    }

    //  get drive
	F32 fDrive = GetOptionValue( pInstance, pInitData->optDrive );

    //  set drive
	if( pState->fDrive != fDrive )
    {
        pState->fDrive = fDrive;
		//pDSP1->SetParam( Core::DSP::Saturator::eDrive, Maths::dBToFloat( fDrive ) );
		pDSP1->SetDriveParam( Maths::dBToFloat( fDrive ) );
    }

    //  get level
	F32 fLevel = GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

    //  has changed
    if( pState->fLevel != fLevel )
    {
        pState->fLevel = fLevel;
		pDSP1->SetParam( Core::DSP::Saturator::eLevel, fLevel );
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoDistortion::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::Effect* pDSP0 = (Core::DSP::Effect*)pInstance->GetCoreData( m_uDSP[ 0 ] );
	Core::DSP::Effect* pDSP1 = (Core::DSP::Effect*)pInstance->GetCoreData( m_uDSP[ 1 ] );
	Core::DSP::Effect* pDSP2 = (Core::DSP::Effect*)pInstance->GetCoreData( m_uDSP[ 2 ] );

	switch( uMessage )
    {
    case EnableMsg:
		pDSP0->SetActive( true );
		pDSP1->SetActive( true );
		pDSP2->SetActive( true );
        break;

    case BypassMsg:
		pDSP0->SetActive( false );
		pDSP1->SetActive( false );
		pDSP2->SetActive( false );
        break;

    default:
        Assert( 0 );
        break;
    }
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


