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
void* Audio::StereoPitchShift::Init(void* pData)
{
	Data* pInitData = PreInit<Data>(pData);
    return (void*)(pInitData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	CoreMemory
////////////////////////////////////////////////////////////////////////////////
U32 Audio::StereoPitchShift::CoreMemory(U32 uOffset)
{
	m_uDSP = (ixCore)uOffset;

	uOffset += AUDIO_CORE_ALIGN_16(sizeof(Core::DSP::PitchShift));

	return uOffset;
}

////////////////////////////////////////////////////////////////////////////////
//	InitContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoPitchShift::InitContext(Instance* pInstance)
{
	U8* pStateData = pInstance->GetStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];
	U8* pCoreData = pInstance->GetCoreData();

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	Core::DSP::PitchShift* pDSP = (Core::DSP::PitchShift*)&pCoreData[m_uDSP];

	new(&pCoreData[m_uDSP]) Core::DSP::PitchShift(Name(), 2);

    pState->fPitchShift = 0.f;
	pState->fSequenceLength = 0.f;
	pState->fSeekWindowLength = 0.f;
	pState->fOverlapLength = 0.f;

}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseContext
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoPitchShift::ReleaseContext(Instance* pInstance)
{
	U8* pCoreData = pInstance->GetCoreData();
	Core::DSP::PitchShift* pDSP = (Core::DSP::PitchShift*)&pCoreData[m_uDSP];

	delete pDSP;
}
#endif	//	#ifndef PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoPitchShift::Update(Instance* pInstance)
{
    AUDIO_PERF_MICRO_EVENT("Audio.Update.StereoPitchShift");

	Data* pInitData = (Data*)m_pObject->GetInitData(m_uInitOffset);
	U8* pStateData = pInstance->GetNewStateData();
    State* pState = (State*)&pStateData[m_uStateOffset];
	U8* pCoreData = pInstance->GetCoreData();
	Core::DSP::PitchShift* pDSP = (Core::DSP::PitchShift*)&pCoreData[m_uDSP];

	//////    //  get pitch
	bool bPitchChanged = false;
	F32 fPitch = GetOptionValue( pInstance, pInitData->optPitchShift );

    //  has changed
    if (pState->fPitchShift != fPitch)
    {
        pState->fPitchShift = fPitch;
        //fLevel *= Maths::dBToFloat(fPitch);
		bPitchChanged = true;
    }

	if(bPitchChanged)
		pDSP->SetParam(Core::DSP::PitchShift::ePitch, fPitch);

		//////    //  get SequenceLength
	bool bSequenceLengthChanged = false;
	F32 fSequenceLength = GetOptionValue( pInstance, pInitData->optSequenceLength );

    //  has changed
    if (pState->fSequenceLength!= fSequenceLength)
    {
        pState->fSequenceLength = fSequenceLength;
        //fLevel *= Maths::dBToFloat(fSequenceLength);
		bSequenceLengthChanged = true;
    }

	if(bSequenceLengthChanged)
		pDSP->SetParam(Core::DSP::PitchShift::eSequenceLength, fSequenceLength);

			//////    //  get SeekWindowLength
	bool bSeekWindowLengthChanged = false;
	F32 fSeekWindowLength = GetOptionValue( pInstance, pInitData->optSeekWindowLength );

    //  has changed
    if (pState->fSeekWindowLength != fSeekWindowLength)
    {
        pState->fSeekWindowLength = fSeekWindowLength;
        //fLevel *= Maths::dBToFloat(fSeekWindowLength);
		bSeekWindowLengthChanged = true;
    }

	if(bSeekWindowLengthChanged)
		pDSP->SetParam(Core::DSP::PitchShift::eSeekWindowLength, fSeekWindowLength);

				//////    //  get OverlapLength
	bool bOverlapLengthChanged = false;
	F32 fOverlapLength = GetOptionValue( pInstance, pInitData->optOverlapLength );

    //  has changed
    if (pState->fOverlapLength != fOverlapLength)
    {
        pState->fOverlapLength = fOverlapLength;
        //fLevel *= Maths::dBToFloat(fOverlapLength);
		bOverlapLengthChanged = true;
    }

	if(bOverlapLengthChanged)
		pDSP->SetParam(Core::DSP::PitchShift::eOverlapLength, fOverlapLength);





	//	//set sequence length
	//if(pInitData->optSequenceLength.ctrl.sCom != -1)
	//	pDSP->SetParam(Core::DSP::PitchShift::eSequenceLength, m_pObject->GetComponentValue(pInitData->optSequenceLength.ctrl.sCom, pInstance, pInitData->optSequenceLength.ctrl.uOut));
 //   else
 //       pDSP->SetParam(Core::DSP::PitchShift::eSequenceLength,pInitData->optSequenceLength.fVal);

	//if(pInitData->optSeekWindowLength.ctrl.sCom != -1)
	//	pDSP->SetParam(Core::DSP::PitchShift::eSeekWindowLength, m_pObject->GetComponentValue(pInitData->optSeekWindowLength.ctrl.sCom, pInstance, pInitData->optSeekWindowLength.ctrl.uOut));
 //   else
 //       pDSP->SetParam(Core::DSP::PitchShift::eSeekWindowLength,pInitData->optSeekWindowLength.fVal);

	//if(pInitData->optOverlapLength.ctrl.sCom != -1)
	//	pDSP->SetParam(Core::DSP::PitchShift::eOverlapLength, m_pObject->GetComponentValue(pInitData->optOverlapLength.ctrl.sCom, pInstance, pInitData->optOverlapLength.ctrl.uOut));
 //   else
 //       pDSP->SetParam(Core::DSP::PitchShift::eOverlapLength,pInitData->optOverlapLength.fVal);

}

////////////////////////////////////////////////////////////////////////////////
//	DoMessage
////////////////////////////////////////////////////////////////////////////////
void Audio::StereoPitchShift::DoMessage( Instance* pInstance, U32 uMessage )
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

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

