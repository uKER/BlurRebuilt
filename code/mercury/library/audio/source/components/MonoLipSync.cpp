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
void* Audio::MonoLipSync::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoLipSync::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::LipSync));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoLipSync::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::LipSync( Name(), 1 );

	pState->fBlendRMS =  0.75f;
	pState->fBlendPitch= 0.90f;
    pState->fSilenceThreshold= 0.023f;
	pState->fClosedMouthWidth = 0.355f;
	pState->fSibPostScale = 40.f;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoLipSync::ReleaseContext(Instance* pInstance)
{
	Core::DSP::LipSync* pDSP = (Core::DSP::LipSync*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoLipSync::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoLipSync");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::LipSync* pDSP = (Core::DSP::LipSync*)pInstance->GetCoreData( m_uDSP );

	State* pOldState = (State*)pInstance->GetStateData(m_uStateOffset);

	//pDSP->SetActive( pState->bActive );

	//	if active
	if (pOldState->bActive)
	{
		//  get BlendRMS
		F32 fBlendRMS = GetOptionValue( pInstance, pInitData->optBlendRMS );

		//  set blendRMS
		if (pState->fBlendRMS != fBlendRMS)
		{
			pState->fBlendRMS = fBlendRMS;
			pDSP->SetParam(Core::DSP::LipSync::eBlendRMS, fBlendRMS);
		}

		//  get BlendPitch
		F32 fBlendPitch = GetOptionValue( pInstance, pInitData->optBlendPitch );

		//  set blendPitch
		if (pState->fBlendPitch != fBlendPitch)
		{
			pState->fBlendPitch = fBlendPitch;
			pDSP->SetParam(Core::DSP::LipSync::eBlendPitch, fBlendPitch);
		}

		//  get SilenceThreshold
		F32 fSilenceThreshold = GetOptionValue( pInstance, pInitData->optSilenceThreshold );

		//  set SilenceThreshold
		if (pState->fSilenceThreshold != fSilenceThreshold)
		{
			pState->fSilenceThreshold = fSilenceThreshold;
			pDSP->SetParam(Core::DSP::LipSync::eSilenceThreshold, fSilenceThreshold);
		}

		 //  get ClosedMouthWidth
		F32 fClosedMouthWidth = GetOptionValue( pInstance, pInitData->optClosedMouthWidth );

		//  set ClosedMouthWidth
		if (pState->fClosedMouthWidth != fClosedMouthWidth)
		{
			pState->fClosedMouthWidth = fClosedMouthWidth;
			pDSP->SetParam(Core::DSP::LipSync::eClosedMouthWidth, fClosedMouthWidth);
		}

		 //  get SibPostScale
		F32 fSibPostScale = GetOptionValue( pInstance, pInitData->optSibPostScale );

		//  set SibPostScale
		if (pState->fSibPostScale != fSibPostScale)
		{
			pState->fSibPostScale = fSibPostScale;
			pDSP->SetParam(Core::DSP::LipSync::eSibPostScale, fSibPostScale);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::MonoLipSync::Value(Instance* pInstance, U32 uOutput)
{
	Core::DSP::LipSync* pDSP = (Core::DSP::LipSync*)pInstance->GetCoreData( m_uDSP );

    switch(uOutput)
    {
    case RMSCtrl:
		return pDSP->GetParam(Core::DSP::LipSync::eRMSOutput);
        break;

    case PitchCtrl:
		return pDSP->GetParam(Core::DSP::LipSync::ePitchOutput);
        break;

    case SibDifferenceCtrl:
		return pDSP->GetParam(Core::DSP::LipSync::eSibDifference);
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
void Audio::MonoLipSync::DoMessage(Instance* pInstance, U32 uMessage)
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

//////////////////////////////////////////////////////////////////////////////
//	Enable
//////////////////////////////////////////////////////////////////////////////
void Audio::MonoLipSync::Enable(U8* pStateData)
{
    State* pState = (State*)&pStateData[m_uStateOffset];

	pState->bActive = true;
}

//////////////////////////////////////////////////////////////////////////////
// Bypass
//////////////////////////////////////////////////////////////////////////////
void Audio::MonoLipSync::Bypass(U8* pStateData)
{
    State* pState = (State*)&pStateData[m_uStateOffset];

	pState->bActive = false;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

