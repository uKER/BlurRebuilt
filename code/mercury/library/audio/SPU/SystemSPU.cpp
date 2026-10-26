#include "AudioSpuPch.h"
#include "SystemHeadersSPU.h"


using namespace Audio;

System system;
System* System::m_pSystem = NULL;

#ifdef OVERRIDE_SPU_ALLOCS
//extern uint32_t _end;
//
//U32 u32HeapStart = 0;
//U32 u32HeapEnd = 0;
//U32 u32HeapMemPos = 0;

inline U32 AlignTo16Bytes(U32 u32Address)
{
	return (u32Address + 15) & 0xfffffff0;
}


void* operator new(size_t size)
{
	return Spu::Alloc(size);

	//U32 u32Retvalue = AlignTo16Bytes(u32HeapEnd);
	//u32HeapEnd = u32Retvalue + size;

	//if (u32HeapEnd > (U32)(&u32Retvalue))	// are we running into the stack??
	//{
	//	spu_printf("running into heap- heap end is 0x%x, alloc size is %d\n", u32HeapEnd, size);
	//	DebugBreak();
	//	return 0;
	//}

	//if (u32HeapEnd > 0x3f000)
	//	spu_printf("warning- audio heap end at 0x%x\n", u32HeapEnd);

	//return (void*)u32Retvalue;
}


void operator delete(void* ptr)
{
}


void* operator new[](size_t size)
{
	return Spu::Alloc(size);

	//U32 u32Retvalue = AlignTo16Bytes(u32HeapEnd);
	//u32HeapEnd += size;

	//if (u32HeapEnd > (U32)(&u32Retvalue))	// are we running into the stack??
	//{
	//	spu_printf("running into heap- heap end is 0x%x, alloc size is %d\n", u32HeapEnd, size);
	//	DebugBreak();
	//	return 0;
	//}

	//if (u32HeapEnd > 0x3f000)
	//	spu_printf("warning- audio heap end at 0x%x\n", u32HeapEnd);

	//return (void*)u32Retvalue;
}


void operator delete[](void* ptr)
{
}


//void CheckMemory()
//{
//	//U32 u32TestVar = 0;
//
//	//if (u32HeapEnd >= (U32)(&u32TestVar))
//	//{
//	//	spu_printf("Audio Stack running into Heap!! top of stack=0x%x, top of heap = 0x%x\n", &u32TestVar, u32HeapEnd);
//	//	DebugBreak();
//	//}
//}


//void ResetHeap()
//{
//	//u32HeapStart = AlignTo16Bytes((U32)(void *)&_end + 1 + (1024 * 50));
//	//u32HeapEnd = u32HeapStart;
//}
//
//
//void StoreHeapPosition()
//{
////	u32HeapMemPos = u32HeapEnd;
//}
//
//
//void ResetHeapToStoredPosition()
//{
////	u32HeapEnd = u32HeapMemPos;
//}
#endif	//	#ifdef OVERRIDE_SPU_ALLOCS


System* Audio::System::Create()
{
    if (System::m_pSystem == NULL)
        System::m_pSystem = &(system);

//	m_pSystem->Init();

    return System::m_pSystem;
}


U32 Audio::System::LookupCodeOverlayIdForComponent(U32 u32CompType)
{
#ifdef AUDIO_SPU_MINIMAL
	switch (u32CompType)
	{
		case Component::eControlOutput:
		case Component::eEventInput:
		case Component::eMonoToSurroundMixer:
		case Component::eSurroundToSurroundMixer:
		{
			return 1;
		}
		case Component::eParameterOut:
		case Component::eDebugPrint:
		case Component::eArray:
		case Component::eGateSwitch:
		case Component::eGradient:
		case Component::eInvert:
		{
			return 2;
		}
		case Component::eCone:
		case Component::eEnvelope:
		case Component::eExponent:
		case Component::eMax:
		case Component::eMin:
		case Component::eVariable:
		case Component::eActionIn:
		case Component::eTimer:
		{
			return 3;
		}
		case Component::eMonoAudioOutput:
		case Component::eEventOutput:
		case Component::eMonoOneShotSound:
		case Component::eMonoToMonoMixer:
		{
			return 4;
		}
		case Component::eParameterIn:
		case Component::eActionOut:
		case Component::eSum:
		case Component::eSurroundAudioOutput:
		case Component::eSurroundAudioInput:
		{
			return 5;
		}
		case Component::eControlInput:
		case Component::eLowPass:
		case Component::eScript:
		{
			return 6;
		}
	}
#elif AUDIO_GENERATORS_NOT_OVERLAID
	switch (u32CompType)
	{
		case Component::eActionIn:
		case Component::eActionOut:
		case Component::eArray:
		case Component::eCone:
		case Component::eControlInput:
		case Component::eControlOutput:
		case Component::eDebugPrint:
		case Component::eDopplerShift:
		case Component::eSpring:
		{
			return 1;
		}
		case Component::eEnvelope:
		case Component::eEventInput:
		case Component::eEventOutput:
		case Component::eExponent:
		case Component::eGateSwitch:
		case Component::eGradient:
		case Component::eInvert:
		case Component::eLowPass:
		case Component::eStereoDistortion:
		{
			return 2;
		}
		case Component::eMax:
		case Component::eMin:
		case Component::eMonoAudioInput:
		case Component::eMonoAudioOutput:
		case Component::eMonoConvolution:
		case Component::eMonoDelay:
		case Component::eMonoGain:
		case Component::eMonoSend:
		case Component::eMonoReturn:
		case Component::eMonoHighPass:
		case Component::eSurroundHighPass:
		{
			return 3;
		}
		case Component::eMonoLipSync:
		case Component::eMonoSpectralReverb:
		case Component::eMonoSpectralEnvelope:
		case Component::eStereoSpectralEnvelope:
		case Component::eMonoLoopingSound:
		case Component::eMonoLowPass:
		case Component::eMonoOneShotSound:
		case Component::eMonoOneShotSoundTable:
		case Component::eMonoPeakEQ:
		case Component::eMonoPeakLevel:
		case Component::eMonoPhaser:
		case Component::eMonoFlanger:
		case Component::eSurroundFlanger:
		case Component::eMonoDistortion:
		{
			return 4;
		}
		case Component::eMonoLowShelf:
		case Component::eMonoHighShelf:
		case Component::eMonoResonantLowPass:
		case Component::eMonoRMSLevel:
		case Component::eMonoOscilloscope:
		case Component::eMonoSaturator:
		case Component::eMonoBitCrusher:
		case Component::eSurroundBitCrusher:
		case Component::eMonoRingModulator:
		case Component::eSurroundRingModulator:
		case Component::eParameterIn:
		case Component::eParameterOut:
		case Component::ePiecewiseLinearFunction:
		case Component::eProduct:
		case Component::eQuantize:
		{
			return 5;
		}
		case Component::eRandom:
		case Component::eRolloff:
		case Component::eSampleAndHold:
		case Component::eScript:
		case Component::eSelector:
		case Component::eSine:
		case Component::eSource:
		case Component::eStereoAudioInput:
		{
			return 6;
		}
		case Component::eStereoConvolution:
		case Component::eStereoHighPass:
		case Component::eSurroundHighPass:
		case Component::eStereoLoopingSound:
		case Component::eStereoLowPass:
		case Component::eStereoOneShotSound:
		case Component::eStereoSaturator:
		case Component::eStereoResonantLowPass:
		{
			return 7;
		}
		case Component::eSum:
		case Component::eSurroundAudioInput:
		case Component::eSurroundAudioOutput:
		case Component::eSurroundDelay:
		case Component::eSurroundLowPass:
		case Component::eSurroundSaturator:
		case Component::eSurroundResonantLowPass:
		{
			return 8;
		}
		case Component::eStereoAudioOutput:
		case Component::eTimer:
		case Component::eVariable:
		case Component::eVector:
		case Component::eVectorIn:
		case Component::eVectorNearestPointOnLine:
		{
			return 9;
		}
		case Component::eMonoToMonoMixer:
		case Component::eMonoToMonoSpectralMixer:
		case Component::eMonoToStereoMixer:
		case Component::eMonoToSurroundMixer:
		{
			return 10;
		}
		case Component::eStereoToMonoMixer:
		case Component::eStereoToStereoMixer:
		case Component::eStereoToSurroundMixer:
		case Component::eSurroundToSurroundMixer:
		{
			return 11;
		}
		case Component::eMonoToSurroundLFEMixer:
		case Component::eSurroundToSurroundLFEMixer:
		case Component::eSurroundLFEToSurroundLFEMixer:
		case Component::eSurroundLFEAudioInput:
		case Component::eSurroundLFEAudioOutput:
		{
			return 12;
		}
	}
#else
	return 1;
	//switch (u32CompType)
	//{
		//case Component::eActionIn:
		//case Component::eActionOut:
		//case Component::eArray:
		//case Component::eCone:
		//case Component::eControlInput:
		//case Component::eControlOutput:
		//case Component::eDebugPrint:
		//case Component::eDopplerShift:
		//case Component::eSpring:
		//case Component::eEnvelope:
		//case Component::eEventInput:
		//case Component::eEventOutput:
		//case Component::eExponent:
		//case Component::eGateSwitch:
		//case Component::eGradient:
		//case Component::eInvert:
		//case Component::eLowPass:
		//case Component::eStereoDistortion:
		//case Component::eMax:
		//case Component::eMin:
		//case Component::eMonoAudioInput:
		//case Component::eMonoAudioOutput:
		//case Component::eMonoConvolution:
		//case Component::eMonoDelay:
		//case Component::eMonoGain:
		//case Component::eMonoHighPass:
		//case Component::eSurroundHighPass:
		//case Component::eMonoLipSync:
		//case Component::eMonoLoopingSound:
		//case Component::eMonoLowPass:
		//case Component::eMonoOneShotSound:
		//case Component::eMonoOneShotSoundTable:
		//case Component::eMonoPeakEQ:
		//case Component::eMonoPeakLevel:
		//case Component::eMonoPhaser:
		//case Component::eMonoFlanger:
		//case Component::eMonoDistortion:
		//case Component::eMonoLowShelf:
		//case Component::eMonoHighShelf:
		//case Component::eMonoResonantLowPass:
		//case Component::eMonoRMSLevel:
		//case Component::eMonoSaturator:
		//case Component::eParameterIn:
		//case Component::eParameterOut:
		//case Component::ePiecewiseLinearFunction:
		//case Component::eProduct:
		//case Component::eQuantize:
		//case Component::eRandom:
		//case Component::eRolloff:
		//case Component::eSampleAndHold:
		//case Component::eScript:
		//case Component::eSelector:
		//case Component::eSine:
		//case Component::eSource:
		//case Component::eStereoAudioInput:
		//{
		//	return 1;
		//}
		//case Component::eStereoConvolution:
		//case Component::eStereoHighPass:
		//case Component::eSurroundHighPass:
		//case Component::eStereoLoopingSound:
		//case Component::eStereoLowPass:
		//case Component::eStereoOneShotSound:
		//case Component::eStereoSaturator:
		//case Component::eStereoResonantLowPass:
		//case Component::eSum:
		//case Component::eSurroundAudioInput:
		//case Component::eSurroundAudioOutput:
		//case Component::eSurroundDelay:
		//case Component::eSurroundLowPass:
		//case Component::eSurroundSaturator:
		//case Component::eSurroundResonantLowPass:
		//case Component::eStereoAudioOutput:
		//case Component::eTimer:
		//case Component::eVariable:
		//case Component::eVector:
		//case Component::eVectorIn:
		//case Component::eVectorNearestPointOnLine:
		//case Component::eMonoToMonoMixer:
		//case Component::eMonoToMonoSpectralMixer:
		//case Component::eMonoToStereoMixer:
		//case Component::eMonoToSurroundMixer:
		//case Component::eStereoToMonoMixer:
		//case Component::eStereoToStereoMixer:
		//case Component::eStereoToSurroundMixer:
		//case Component::eSurroundToSurroundMixer:
		//case Component::eMonoToSurroundLFEMixer:
		//case Component::eSurroundToSurroundLFEMixer:
		//case Component::eSurroundLFEToSurroundLFEMixer:
		//case Component::eSurroundLFEAudioInput:
		//case Component::eSurroundLFEAudioOutput:
		//{
		//	return 2;
		//}
	//}
#endif

	// TODO: error
	return -1;
}


bool Audio::System::LoadCodeOverlay(U32 u32OverlayId)
{
return true;	// temp, overlays possibly not required anymore
	CoreLoadCodeOverlay(u32OverlayId);
/*
	if (m_u32CurrentCodeOverlayId == u32OverlayId)
		return true;

	U32 u32RetValue = 0;

	m_u32CurrentCodeOverlayId = u32OverlayId;

	if (! m_pOverlayTable)
		DebugBreak();

	switch (u32OverlayId)
	{
		case 1:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay1((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay1(20);
		}
		break;

		case 2:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay2((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay2(20);
		}
		break;

		case 3:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay3((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay3(20);
		}
		break;

		case 4:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay4((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay4(20);
		}
		break;

		case 5:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay5((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay5(20);
		}
		break;

		case 6:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay6((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay6(20);
		}
		break;

#ifndef AUDIO_SPU_MINIMAL
		case 7:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay7((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay7(20);
		}
		break;

		case 8:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay8((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay8(20);
		}
		break;

		case 9:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay9((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay9(20);
		}
		break;

		case 10:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay10((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay10(20);
		}
		break;

		case 11:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay11((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay11(20);
		}
		break;

		case 12:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay12((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay12(20);
		}
		break;

		case 13:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay13((uint64_t)m_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay13(20);
		}
		break;
#endif

		default:
		{
			u32RetValue = 1;
			DebugBreak();
		}
		break;
	}

	if (u32RetValue != 0)
	{
		spu_printf("overlap error- error code is 0x%x, overlay table location is 0x%x\n", u32RetValue, m_pOverlayTable);
		DebugBreak();
		assert(0);
		return false;
	}
	else
		return true;
*/
}


void Audio::System::LoadCodeOverlayForComponent(U32 u32CompType)
{
return;	// temp, overlays possibly not required anymore
	U32 u32RequiredOverlay = LookupCodeOverlayIdForComponent(u32CompType);

	if (u32RequiredOverlay == (U32)-1 || u32RequiredOverlay == g_u32CurrentCodeOverlayId)
		return;

	LoadCodeOverlay(u32RequiredOverlay);
}


//void Audio::System::TriggerDecoderCallback(U32 u32CallbackId, U32 u32StateOffset)
//{
//	U32 u32CurrId = Get()->GetCurrentCodeOverlayId();
//
//	Get()->LoadCodeOverlayForComponent(u32CallbackId);
//
//	switch (u32CallbackId)
//	{
//		case Component::eMonoOneShotSound:
//			MonoOneShotSound::FinishedCallback(u32StateOffset);
//			break;
//
//#ifndef AUDIO_SPU_MINIMAL
//		case Component::eMonoOneShotSoundTable:
//			MonoOneShotSoundTable::FinishedCallback(u32StateOffset);
//			break;
//
//		case Component::eMonoLoopingSound:
//			MonoLoopingSound::FinishedCallback(u32StateOffset);
//			break;
//
//		case Component::eStereoOneShotSound:
//			StereoOneShotSound::FinishedCallback(u32StateOffset);
//			break;
//
		//case Component::eStereoOneShotSoundTable:
		//	StereoOneShotSoundTable::FinishedCallback(u32StateOffset);
		//	break;

//		case Component::eStereoLoopingSound:
//			StereoLoopingSound::FinishedCallback(u32StateOffset);
//			break;
//#endif
//
//		default:
//			// TODO: warning
//			break;
//	}
//
//	Get()->LoadCodeOverlay(u32CurrId);
//	
//}

/*
void* Audio::SpuFileStreamManager::GetData(FS_STREAM_HANDLE hStream, U32 u32NumBytes)
{
	if (m_aStreamInfo[hStream].pPpuBuffer)
	{
		m_DmaTransfer.Set((m_aStreamInfo[hStream].pPpuBuffer), u32NumBytes);

		//if (m_DmaTransfer.GetRequiredBufferSize() > Audio::System::Get()->GetDmaScratchBufferSize())
		//	DebugBreak();

		m_DmaTransfer.PrepareBuffer(Audio::System::Get()->GetDmaScratchBuffer());

		m_DmaTransfer.Read();

		m_aStreamInfo[hStream].u16BytesReadOnSpu = u32NumBytes;

		return m_DmaTransfer.GetSPUAddress();
	}

	return 0;
}


bool Audio::SpuFileStreamManager::SetFilePosition(FS_STREAM_HANDLE hStream, U32 u32PositionInBytes)
{
	SetStateFlagToTrue(hStream, STREAM_SET_POSITION_REQUIRED);
	m_aStreamInfo[hStream].u32RequiredFilePosition = u32PositionInBytes;

	return true;
}
*/