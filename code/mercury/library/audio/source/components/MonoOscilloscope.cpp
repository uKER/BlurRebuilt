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
void* Audio::MonoOscilloscope::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoOscilloscope::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::Oscilloscope));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOscilloscope::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::Oscilloscope( Name(), 1 );

	pState->fLength = 0.001f;
	pState->fLevel = 1.f;

	memset( pState->afHighs, 0, 24 * sizeof(F32) );
	memset( pState->afLows, 0, 24 * sizeof(F32) );
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOscilloscope::ReleaseContext( Instance* pInstance )
{
	Core::DSP::Oscilloscope* pDSP = (Core::DSP::Oscilloscope*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOscilloscope::Update( Instance* pInstance )
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoOscilloscope");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::Oscilloscope* pDSP = (Core::DSP::Oscilloscope*)pInstance->GetCoreData( m_uDSP );

	//pState->fValue = pDSP->GetParam(Core::DSP::Oscilloscope::eLevel);

	//  get Length
	F32 fLength = GetOptionValue( pInstance, pInitData->optLength );

	//  set Length
	if (pState->fLength != fLength)
	{
		pState->fLength = fLength;
		pDSP->SetParam(Core::DSP::Oscilloscope::eLength, fLength);
	}

	//  get Level
	F32 fLevel = GetOptionValue( pInstance, pInitData->optLevel );

	//  set Level
	if (pState->fLevel != fLevel)
	{
		pState->fLevel = fLevel;
		pDSP->SetParam( Core::DSP::Oscilloscope::eLevel, fLevel );
	}

	pDSP->GetHighs( pState->afHighs, 24 );
	pDSP->GetLows( pState->afLows, 24 );
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::MonoOscilloscope::Value( Instance* pInstance, U32 uOutput )
{
	State* pState = (State*)pInstance->GetStateData( m_uStateOffset );

	if( uOutput >= High00Ctrl && uOutput <= High23Ctrl )
		return pState->afHighs[ uOutput ];
	else if( uOutput >= Low00Ctrl && uOutput <= Low23Ctrl )
		return pState->afLows[ uOutput - Low00Ctrl ];
	else
		return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoOscilloscope::DoMessage( Instance* pInstance, U32 uMessage )
{
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::Oscilloscope* pDSP = (Core::DSP::Oscilloscope*)pInstance->GetCoreData( m_uDSP );

	switch (uMessage)
    {
    case EnableMsg:
        pState->bActive = true;
        break;

    case BypassMsg:
        pState->bActive = false;
        break;

    case ResetMsg:
		pDSP->Reset();
        break;

    default:
        Assert(0);
        break;
    }
}
