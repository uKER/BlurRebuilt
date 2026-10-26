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
void* Audio::MonoSpectralReverb::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::MonoSpectralReverb::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::SpectralReverb));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoSpectralReverb::InitContext(Instance* pInstance)
{
    State* pState = (State*)pInstance->GetStateData(m_uStateOffset);

	new( pInstance->GetCoreData( m_uDSP ) ) Core::DSP::SpectralReverb( Name(), 1 );

	//pState->fBand_To_175Hz_DecayTime = 0;
	//pState->fBand_To_312Hz_DecayTime = 0;
	//pState->fBand_To_750Hz_DecayTime = 0;
	//pState->fBand_To_1500Hz_DecayTime = 0;
	//pState->fBand_To_3kHz_DecayTime = 0;
	//pState->fBand_To_22kHz_DecayTime = 0;

	//pState->fMaster_DecayTime = 0;
	

	//pState->fBlendRMS =  0.75f;
	//pState->fBlendPitch= 0.90f;
 //   pState->fSilenceThreshold= 0.023f;
	//pState->fClosedMouthWidth = 0.355f;
	pState->bActive = true;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoSpectralReverb::ReleaseContext(Instance* pInstance)
{
	Core::DSP::SpectralReverb* pDSP = (Core::DSP::SpectralReverb*)pInstance->GetCoreData( m_uDSP );

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoSpectralReverb::Update(Instance* pInstance)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Update.MonoSpectralReverb");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
    State* pState = (State*)pInstance->GetNewStateData(m_uStateOffset);
	Core::DSP::SpectralReverb* pDSP = (Core::DSP::SpectralReverb*)pInstance->GetCoreData( m_uDSP );

	State* pOldState = (State*)pInstance->GetStateData(m_uStateOffset);

	//pDSP->SetActive( pState->bActive );

	//	if active
	//if (pOldState->bActive)
	{
		
		//  get Band1_DecayTime
		F32 fBand1_DecayTime = GetOptionValue( pInstance, pInitData->optBand1_DecayTime );

		//  set band6_DecayTime
		if (pState->fBand1_DecayTime != fBand1_DecayTime)
		{
			pState->fBand1_DecayTime = fBand1_DecayTime;
			pDSP->SetParam(Core::DSP::SpectralReverb::eBand1DecayTime, fBand1_DecayTime);
		}

		//  get Band2_DecayTime
		F32 fBand2_DecayTime = GetOptionValue( pInstance, pInitData->optBand2_DecayTime );

		//  set band6_DecayTime
		if (pState->fBand2_DecayTime != fBand2_DecayTime)
		{
			pState->fBand2_DecayTime = fBand2_DecayTime;
			pDSP->SetParam(Core::DSP::SpectralReverb::eBand2DecayTime, fBand2_DecayTime);
		}

		//  get Band3_DecayTime
		F32 fBand3_DecayTime = GetOptionValue( pInstance, pInitData->optBand3_DecayTime );

	//  set band6_DecayTime
		if (pState->fBand3_DecayTime != fBand3_DecayTime)
		{
			pState->fBand3_DecayTime = fBand3_DecayTime;
			pDSP->SetParam(Core::DSP::SpectralReverb::eBand3DecayTime, fBand3_DecayTime);
		}

		//  get Band4_DecayTime
		F32 fBand4_DecayTime = GetOptionValue( pInstance, pInitData->optBand4_DecayTime );

		//  set band6_DecayTime
		if (pState->fBand4_DecayTime != fBand4_DecayTime)
		{
			pState->fBand4_DecayTime = fBand4_DecayTime;
			pDSP->SetParam(Core::DSP::SpectralReverb::eBand4DecayTime, fBand4_DecayTime);
		}

		//  get Band5_DecayTime
		F32 fBand5_DecayTime = GetOptionValue( pInstance, pInitData->optBand5_DecayTime );

		//  set band6_DecayTime
		if (pState->fBand5_DecayTime != fBand5_DecayTime)
		{
			pState->fBand5_DecayTime = fBand5_DecayTime;
			pDSP->SetParam(Core::DSP::SpectralReverb::eBand5DecayTime, fBand5_DecayTime);
		}

		//  get Band6_DecayTime
		F32 fBand6_DecayTime = GetOptionValue( pInstance, pInitData->optBand6_DecayTime );

		//  set band6_DecayTime
		if (pState->fBand6_DecayTime != fBand6_DecayTime)
		{
			pState->fBand6_DecayTime = fBand6_DecayTime;
			pDSP->SetParam(Core::DSP::SpectralReverb::eBand6DecayTime, fBand6_DecayTime);
		}

		//  get Master_DecayTime
		F32 fMaster_DecayTime = GetOptionValue( pInstance, pInitData->optMaster_DecayTime );

		//  set Master_DecayTime
		if (pState->fMaster_DecayTime != fMaster_DecayTime)
		{
			pState->fMaster_DecayTime = fMaster_DecayTime;
			pDSP->SetParam(Core::DSP::SpectralReverb::eMasterDecayTime, fMaster_DecayTime);
		}

		//  get gain
		
		F32 fGain = GetLevel( pInstance, pInitData->optLevel, pInitData->optGain );

		//  set Master_DecayTime
		if (pState->fGain != fGain)
		{
			pState->fGain = fGain;
			pDSP->SetParam(Core::DSP::SpectralReverb::eReverbGain, fGain);
		}

		F32 fSmoothFilter = GetOptionValue( pInstance, pInitData->optSmoothFilter );

		//  set Master_DecayTime
		if (pState->fSmoothFilter != fSmoothFilter)
		{
			pState->fSmoothFilter = fSmoothFilter;
			pDSP->SetParam(Core::DSP::SpectralReverb::eSmoothFilter, fSmoothFilter);
		}

#ifdef NEW_REVERB_PARAMS
		F32 sPreset = GetOptionValue( pInstance, pInitData->optPreset );

		if (pState->sPreset != sPreset)
		{
			pState->sPreset = (S32)sPreset;
			pDSP->SetParam(Core::DSP::SpectralReverb::ePreset, sPreset);
		}

		F32 sHighPassCutoffFreq = GetOptionValue( pInstance, pInitData->optHighPassCutoffFreq );

		if (pState->sHighPassCutoffFreq!= sHighPassCutoffFreq)
		{
			pState->sHighPassCutoffFreq = (S32)sHighPassCutoffFreq;
			pDSP->SetParam(Core::DSP::SpectralReverb::eHighPassCutoffFreq, sHighPassCutoffFreq);
		}

		F32 fHighPassAttenuation = GetOptionValue( pInstance, pInitData->optHighPassAttenuation );

		if (pState->fHighPassAttenuation != fHighPassAttenuation)
		{
			pState->fHighPassAttenuation = fHighPassAttenuation;
			pDSP->SetParam(Core::DSP::SpectralReverb::eHighPassAttenuation, fHighPassAttenuation);
		}

		F32 sLowPassCutoffFreq = GetOptionValue( pInstance, pInitData->optLowPassCutoffFreq );

		if (pState->sLowPassCutoffFreq != sLowPassCutoffFreq)
		{
			pState->sLowPassCutoffFreq = (S32)sLowPassCutoffFreq;
			pDSP->SetParam(Core::DSP::SpectralReverb::eLowPassCutoffFreq, sLowPassCutoffFreq);
		}

		F32 fLowPassAttenuation = GetOptionValue( pInstance, pInitData->optLowPassAttenuation );


		if (pState->fLowPassAttenuation != fLowPassAttenuation)
		{
			pState->fLowPassAttenuation = fLowPassAttenuation;
			pDSP->SetParam(Core::DSP::SpectralReverb::eLowPassAttenuation, fLowPassAttenuation);
		}

		F32 fSmoothingRatio = GetOptionValue( pInstance, pInitData->optSmoothingRatio );


		if (pState->fSmoothingRatio != fSmoothingRatio)
		{
			pState->fSmoothingRatio = fSmoothingRatio;
			pDSP->SetParam(Core::DSP::SpectralReverb::eSmoothingRatio, fSmoothingRatio);
		}
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
F32 Audio::MonoSpectralReverb::Value(Instance* pInstance, U32 uOutput)
{
	Core::DSP::SpectralReverb* pDSP = (Core::DSP::SpectralReverb*)pInstance->GetCoreData( m_uDSP );

    switch(uOutput)
    {
  /*  case RMSCtrl:
		return pDSP->GetParam(Core::DSP::SpectralReverb::eRMSOutput);
        break;

    case PitchCtrl:
		return pDSP->GetParam(Core::DSP::SpectralReverb::ePitchOutput);
        break;

    case SibDifferenceCtrl:
		return pDSP->GetParam(Core::DSP::SpectralReverb::eSibDifference);
        break;*/

	case 0: // fix compiler warning
    default:
        Assert(0);
        break;
    }

    return 0.f;
}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::MonoSpectralReverb::DoMessage(Instance* pInstance, U32 uMessage)
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
void Audio::MonoSpectralReverb::Enable(U8* pStateData)
{
    State* pState = (State*)&pStateData[m_uStateOffset];

	pState->bActive = true;
}

//////////////////////////////////////////////////////////////////////////////
// Bypass
//////////////////////////////////////////////////////////////////////////////
void Audio::MonoSpectralReverb::Bypass(U8* pStateData)
{
    State* pState = (State*)&pStateData[m_uStateOffset];

	pState->bActive = false;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

