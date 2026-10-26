#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#include "ComponentHelper.h"
#endif

//using namespace Audio;

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::SurroundDelay::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::SurroundDelay::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::Delay));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundDelay::InitContext(Instance* pInstance)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];
	U8* pCoreData = pInstance->GetCoreData();

	new( &pCoreData[ m_uDSP ] ) Core::DSP::Delay( Name(), 5, 96, 0 );

    pState->fLength = 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundDelay::ReleaseContext(Instance* pInstance)
{
	U8* pCoreData = pInstance->GetCoreData();
	Core::DSP::Delay* pDSP = (Core::DSP::Delay*)&pCoreData[m_uDSP];

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundDelay::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.SurroundDelay");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetNewStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];
	U8* pCoreData = pInstance->GetCoreData();
	Core::DSP::Delay* pDSP = (Core::DSP::Delay*)&pCoreData[m_uDSP];

    //  get length
	F32 fLength = GetOptionValue( pInstance, pInitData->optLength );

    //  set gain
    if (pState->fLength != fLength)
    {
        pState->fLength = fLength;
		pDSP->SetParam(Core::DSP::Delay::eLength, fLength);
    }
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::SurroundDelay::DoMessage( Instance* pInstance, U32 uMessage )
{
	U8* pCoreData = pInstance->GetCoreData( );
	Core::DSP::Effect* pDSP = (Core::DSP::Effect*)&pCoreData[ m_uDSP ];

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

