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
void* Audio::StereoPeakEQ::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoPeakEQ::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16( sizeof(Core::DSP::IIRPeakEQ) );

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoPeakEQ::InitContext(Instance* pInstance)
{
	State* pState = (State*)pInstance->GetStateData(m_uStateOffset);
	U8* pCoreData = pInstance->GetCoreData();

	new( &pCoreData[ m_uDSP ] ) Core::DSP::IIRPeakEQ( Name(), 2 );

    pState->fCentre = -1.f;
    pState->fQ = -1.f;
    pState->fLevel = -1.f;
    pState->fClamp = -1.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoPeakEQ::ReleaseContext(Instance* pInstance)
{
	U8* pCoreData = pInstance->GetCoreData();
	Core::DSP::IIRPeakEQ* pDSP = (Core::DSP::IIRPeakEQ*)&pCoreData[m_uDSP];

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoPeakEQ::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoPeakEQ");

	Data* pInitData = (Data*)m_pObject->GetInitData( m_uInitOffset );
	State* pState = (State*)pInstance->GetNewStateData( m_uStateOffset );
	Core::DSP::IIRPeakEQ* pDSP = (Core::DSP::IIRPeakEQ*)pInstance->GetCoreData( m_uDSP );

    //  get centre
	F32 fCentre = GetOptionValue( pInstance, pInitData->optCentre );

    //  adjust freq
	fCentre *= GetFrequency( pInstance, pInitData->optFreqShift, pInitData->optPitchShift );

    //  has changed
    if( pState->fCentre != fCentre )
    {
        pState->fCentre = fCentre;
		pDSP->SetParam( Core::DSP::IIRPeakEQ::eCentre, fCentre );
    }

    //  get q
	F32 fQ = GetOptionValue( pInstance, pInitData->optQ );

    //  has changed
    if( pState->fQ != fQ )
    {
        pState->fQ = fQ;
		pDSP->SetParam( Core::DSP::IIRPeakEQ::eQ, fQ );
    }

    //  get level
	F32 fLevel = GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

    //  has changed
    if( pState->fLevel != fLevel )
    {
        pState->fLevel = fLevel;
		pDSP->SetParam( Core::DSP::IIRPeakEQ::eGain, fLevel );
    }

    //  get clamp
	F32 fClamp = GetOptionValue( pInstance, pInitData->optClamp );

    //  has changed
    if (pState->fClamp != fClamp)
    {
        pState->fClamp = fClamp;
		pDSP->SetParam(Core::DSP::IIRPeakEQ::eClamp, fClamp);
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoPeakEQ::DoMessage( Instance* pInstance, U32 uMessage )
{
	Core::DSP::Effect* pDSP = (Core::DSP::Effect*)pInstance->GetCoreData( m_uDSP );

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
