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
void* Audio::StereoSpectralEnvelope::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoSpectralEnvelope::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::SpectralEnvelope));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSpectralEnvelope::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::SpectralEnvelope( Name(), 2 );

	pState->bActive = true;

}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSpectralEnvelope::ReleaseContext(Instance* pInstance)
{
	Core::DSP::SpectralEnvelope* pDSP = (Core::DSP::SpectralEnvelope*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoSpectralEnvelope::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoSpectralEnvelope");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::SpectralEnvelope* pDSP = (Core::DSP::SpectralEnvelope*)pInstance->GetCoreData( m_uDSP );

	State* pOldState = (State*)pInstance->GetStateData(m_uStateOffset);

	//pDSP->SetActive( pState->bActive );

	//	if active
	//if (pOldState->bActive)
	{
		//  get BlendRMS
		//F32 fBlendRMS = GetOptionValue( pInstance, pInitData->optBlendRMS );

		////  set blendRMS
		//if (pState->fBlendRMS != fBlendRMS)
		//{
		//	pState->fBlendRMS = fBlendRMS;
		//	pDSP->SetParam(Core::DSP::SpectralEnvelope::eBlendRMS, fBlendRMS);
		//}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::StereoSpectralEnvelope::Value(Instance* pInstance, U32 uOutput)
{
	Core::DSP::SpectralEnvelope* pDSP = (Core::DSP::SpectralEnvelope*)pInstance->GetCoreData( m_uDSP );

    switch(uOutput)
    {

	 case Bin1_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin1);
        break;

	 case Bin2_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin2);
        break;

	 case Bin3_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin3);
        break;

	 case Bin4_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin4);
        break;

	 case Bin5_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin5);
        break;

	 case Bin6_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin6);
        break;

	 case Bin7_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin7);
        break;

	 case Bin8_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin8);
        break;

	 case Bin9_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin9);
        break;

	 case Bin10_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin10);
        break;

	 case Bin11_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin11);
        break;

	 case Bin12_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin12);
        break;

	 case Bin13_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin13);
        break;

	 case Bin14_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin14);
        break;

	 case Bin15_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin15);
        break;

	 case Bin16_MagnitudeCtrl:
		return pDSP->GetParam(Core::DSP::SpectralEnvelope::eBin16);
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
void Audio::StereoSpectralEnvelope::DoMessage(Instance* pInstance, U32 uMessage)
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
void Audio::StereoSpectralEnvelope::Enable(U8* pStateData)
{
    State* pState = (State*)&pStateData[m_uStateOffset];

	pState->bActive = true;
}

//////////////////////////////////////////////////////////////////////////////
// Bypass
//////////////////////////////////////////////////////////////////////////////
void Audio::StereoSpectralEnvelope::Bypass(U8* pStateData)
{
    State* pState = (State*)&pStateData[m_uStateOffset];

	pState->bActive = false;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

