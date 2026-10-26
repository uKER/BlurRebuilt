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
//	Init
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::StereoRMSLevel::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoRMSLevel::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::RMSLevel));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoRMSLevel::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::RMSLevel( Name(), 2 );

	pState->fLength = 0.01f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoRMSLevel::ReleaseContext(Instance* pInstance)
{
	Core::DSP::RMSLevel* pDSP = (Core::DSP::RMSLevel*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoRMSLevel::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoRMSLevel");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::RMSLevel* pDSP = (Core::DSP::RMSLevel*)pInstance->GetCoreData( m_uDSP );

	//pState->fValue = pDSP->GetParam(Core::DSP::RMSLevel::eLevel);

	//  get Length
		F32 fLength = GetOptionValue( pInstance, pInitData->optLength );

		//  set Length
		if (pState->fLength != fLength)
		{
			pState->fLength = fLength;
			pDSP->SetParam(Core::DSP::RMSLevel::eLength, fLength);
		}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::StereoRMSLevel::Value(Instance* pInstance, U32 uOutput)
{
   // State* pState = (State*)pInstance->GetStateData(m_uStateOffset);
	Core::DSP::RMSLevel* pDSP = (Core::DSP::RMSLevel*)pInstance->GetCoreData( m_uDSP );

    switch(uOutput)
    {
    case ValueCtrl:
		return pDSP->GetParam(Core::DSP::RMSLevel::eLevel);
        break;

    case LeftValueCtrl:
		return pDSP->GetParam(Core::DSP::RMSLevel::eLeftLevel);
        break;

    case RightValueCtrl:
		return pDSP->GetParam(Core::DSP::RMSLevel::eRightLevel);
        break;

    default:
        Assert(0);
        break;
    }

    return 0.f;
}
////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoRMSLevel::DoMessage(Instance* pInstance, U32 uMessage)
{
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);

	switch (uMessage)
    {
    case EnableMsg:
        pState->bActive = true;
        break;

    case BypassMsg:
        pState->bActive = false;
        break;

    default:
        Assert(0);
        break;
    }
}
#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
