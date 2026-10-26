#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "SystemHeaders.h"
#endif
//#include "core/CorePS3shared.h"
//#include "SystemCommon.cpp"
#include <System/DebugLog.h>

#include <system/Perf/PerfEvent.h>

JQ_DECLARE_SPU_MODULE(AudioObjectUpdate, JobScheduledInstanceUpdate, audio_spu_ps3, "mercury.audio.system");
//JQ_DECLARE_SPU_MODULE(SPUAudioObjectUpdate, AudioSpuProcess, audio_spu_ps3, "LoggerName");

#ifndef PLATFORM_SPU
Debug::Bool g_bDebugRunSetProcessFlagsWithFrameSkip("Audio.System.RunSetProcessFlagsWithFrameSkip", true, Debug::Persist());
#endif

// weird hack to get ps3 compiling..
#ifdef PLATFORM_SPU
extern "C" void _exit()
{
	DebugBreak();
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	InstanceUpdate
////////////////////////////////////////////////////////////////////////////////
void InstanceUpdate(Audio::Instance* pInstance, bool bUpdate, bool bGenerate)
{
	if (bUpdate)
		pInstance->GetObject()->Update( pInstance );

#ifdef PLATFORM_SPU
	Spu::Wait(0);
	CoreLoadCodeOverlay(AUDIO_GENERATOR_OVERLAY_ID);
#endif

	if (bGenerate)
#ifndef PLATFORM_SPU
		pInstance->GetObject()->Generate( pInstance, ! ((! bUpdate) && g_bDebugRunSetProcessFlagsWithFrameSkip) );
#else
		pInstance->GetObject()->Generate( pInstance, bUpdate );
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	SpuPreInstanceUpdate
////////////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_SPU
Audio::Instance* SpuPreInstanceUpdate(InstanceUpdateParams* pParams, Audio::Definition& def)
{
	PERF_EVENT("Audio.JobScheduledInstanceUpdate.PreUpdate");

	Audio::Instance* pInstance = (Audio::Instance*)Spu::FetchAllocAsync(pParams->pInstance, (sizeof(Audio::Instance) + 15) & 0xfffffff0, 0);

	Audio::Object* pObject = (Audio::Object*)Spu::FetchAllocAsync(pParams->m_SpuInfo.pPpuObject, (sizeof(Audio::Object) + 15) & 0xfffffff0, 0);

	void* pDefData = (Audio::Definition*)Spu::FetchAllocAsync(pParams->m_SpuInfo.pPpuDefinitionData, (pParams->m_SpuInfo.u32DefinitionDataSize + 15) & 0xfffffff0, 0);

	void* pComponents = Spu::FetchAllocAsync(pParams->m_SpuInfo.pPpuObjectComponents, (pParams->m_SpuInfo.u32ObjectComponentsSize + 15) & 0xfffffff0, 0);

	void* pInstanceContext = Spu::FetchAllocAsync(pParams->m_SpuInfo.pPpuInstanceContext, (pParams->m_SpuInfo.u32InstanceContextSize + 15) & 0xfffffff0, 0);

	void* pCoreData = 0;
	
	if (pParams->m_SpuInfo.u32InstanceCoreDataSize)
		pCoreData = Spu::FetchAllocAsync(pParams->m_SpuInfo.pPpuInstanceCoreData, (pParams->m_SpuInfo.u32InstanceCoreDataSize + 15) & 0xfffffff0, 0);

	void* pBufferData = 0;

	if (pParams->m_SpuInfo.u32InstanceBufferDataSize)
		pBufferData = Spu::FetchAllocAsync(pParams->m_SpuInfo.pPpuInstanceBufferData, (pParams->m_SpuInfo.u32InstanceBufferDataSize + 15) & 0xfffffff0, 0);

	void* pSystemStateTransfer = Spu::FetchAllocAsync(pParams->m_SpuInfo.pPpuSystemState, (sizeof(Audio::SystemSpuInfo) + 15) & 0xfffffff0, 0);

	Audio::System::Get()->SetDmaScratchBuffer(pParams->m_SpuInfo.u32ScratchDmaBufferSize);

	// need adpcm buffer array during update as well as generate, so transfer here
//	void* pAdpcmBitfield = Spu::FetchAllocAsync(pParams->m_SpuInfo.pPpuAdpcmBufferBitfield, (sizeof(U32) * ADPCM_BUFFER_ARRAY_SIZE + 15) & 0xfffffff0, 0);

	Spu::Wait(0);

//	Audio::Core::GetAdpcmBufferAllocator()->PrepForSpu(pParams->m_SpuInfo.pAdpcmBufferBasePointer, (U32*)pAdpcmBitfield);

	memcpy(&(Audio::System::Get()->m_SystemInfo), pSystemStateTransfer, sizeof(Audio::SystemSpuInfo));

//	Audio::CopyInfoFromSpuStruct(&Audio::System::Get()->m_SystemInfo.debugInfo);

	pInstance->SetObject(pObject);
	pObject->SetDefinition(&def);
	pObject->SetComponents((U8*)pComponents);

	pInstance->SetContext((U8*)pInstanceContext);

//	Audio::System::Get()->StreamManager()->SetArrayPointer(pInstance->GetStreamInfo());

	def.Init(0, pDefData, 0);

	//	clear current value buffer
	F32* afCurrentValues = (F32*)((U32)pInstanceContext + pInstance->GetStateSize());
	F32* afOrigCurrentValues = pInstance->GetCurrentValues();
	afCurrentValues = (F32*)(((U32)afCurrentValues + 0x0f) & ~0x0f);
	pInstance->SetCurrentValues(afCurrentValues);

	U32 uCurrentValSize = (def.NumComponents() * sizeof(float) + 0x0f) & ~0x0f;

	//spu_printf("addr=%.8x size=%d val=%x\n", afCurrentValues, uCurrentValSize, *(U32*)afCurrentValues);

	memset(afCurrentValues, 0xff, uCurrentValSize);

	if (pParams->m_SpuInfo.u32InstanceBufferDataSize)
		pInstance->SetBufferData((U8*)pBufferData);
	else
		pInstance->SetBufferData(0);

	if (pParams->m_SpuInfo.u32InstanceCoreDataSize)
		pInstance->SetCoreData((U8*)pCoreData);
	else
		pInstance->SetCoreData(0);

	if (pParams->m_SpuInfo.u32DynamicBufferSize)
	{
//		void* pDynamicBuffers = Spu::FetchAllocAsync(pParams->m_SpuInfo.pPpuDynamicBuffers, (pParams->m_SpuInfo.u32DynamicBufferSize + 15) & 0xfffffff0, 0);
		void* pDynamicBuffers = Spu::Alloc((pParams->m_SpuInfo.u32DynamicBufferSize + 15) & 0xfffffff0);

		pInstance->m_coreInfo.pDynamicBuffers = (F32*)pDynamicBuffers;
	}
	else
		pInstance->m_coreInfo.pDynamicBuffers = 0;

	pInstance->m_coreInfo.decoderDataInfo.pWaveServerData = pInstance->GetWaveServerData();
	pInstance->m_coreInfo.decoderDataInfo.pAdpcmBufferData = pInstance->GetAdpcmWaveInfoDataChunk();

	return pInstance;
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	SpuPostInstanceUpdate
////////////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_SPU
void SpuPostInstanceUpdate(InstanceUpdateParams* pParams, Audio::Instance* pInstance)
{
	PERF_EVENT("Audio.JobScheduledInstanceUpdate.PostUpdate");

	// TODO: we shouldn't need to write system and adpcm buffer info back to main ram after each instance update finishes,
	// only once a batch of updates finish. change to work like this

	Spu::StoreAsync(pParams->m_SpuInfo.pPpuInstanceContext, pInstance->GetStateData(), (pParams->m_SpuInfo.u32InstanceContextSize + 15) & 0xfffffff0, 0);

	if (pParams->m_SpuInfo.u32InstanceCoreDataSize)
		Spu::StoreAsync(pParams->m_SpuInfo.pPpuInstanceCoreData, pInstance->GetCoreData(), (pParams->m_SpuInfo.u32InstanceCoreDataSize + 15) & 0xfffffff0, 0);

	if (pParams->m_SpuInfo.u32InstanceBufferDataSize)
		Spu::StoreAsync(pParams->m_SpuInfo.pPpuInstanceBufferData, pInstance->GetBufferData(), (pParams->m_SpuInfo.u32InstanceBufferDataSize + 15) & 0xfffffff0, 0);

	// it seems pSystemStateTransfer was getting memcpy-ed into another place, but any modifications were not getting copied back again- so it will be unchanged here anyway.. so pointless transferring back?
//	Spu::StoreAsync(pParams->m_SpuInfo.pPpuSystemState, pSystemStateTransfer, (sizeof(Audio::SystemSpuInfo) + 15) & 0xfffffff0, 0);

//	Spu::StoreAsync(pParams->m_SpuInfo.pPpuAdpcmBufferBitfield, Audio::Core::GetAdpcmBufferAllocator()->GetBitfield(), (sizeof(U32) * ADPCM_BUFFER_ARRAY_SIZE + 15) & 0xfffffff0, 0);

//	Spu::StoreAsync(pParams->m_SpuInfo.pPpuInstanceBitmaskArray, pInstance->GetAdpcmBitmask(), sizeof(U32) * ADPCM_BUFFER_ARRAY_SIZE, 0);

	// the likes of system info needs to be written back to main ram before
	// the next object begins updating, so wait here
	Spu::Wait(0);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	JobScheduledInstanceUpdate
////////////////////////////////////////////////////////////////////////////////
void JobScheduledInstanceUpdate(Scheduler::Task* pTask)
{
	PERF_EVENT("Audio.JobScheduledInstanceUpdate");

//	InstanceUpdateParams* aParams = (InstanceUpdateParams*)pTask->Input;
	U32 u32NumInstancesToUpdate = (U32)(pTask->ParamData[0]);
	InstanceUpdateParams* aParams = (InstanceUpdateParams*)Spu::FetchAlloc(pTask->Input, ((sizeof(InstanceUpdateParams) * u32NumInstancesToUpdate) + 15) & 0xfffffff0);

#ifdef PLATFORM_SPU
	Audio::System::Create();
	Audio::System::Get()->SeedRand(spu_read_decrementer());
	Audio::Definition def;
#endif

	void* pMemBase = Spu::GetMemBase();
	U32 uCount = 0;
	for (U32 i = 0; i < u32NumInstancesToUpdate; i++)
	{
		InstanceUpdateParams params = aParams[i]; 

#ifdef PLATFORM_SPU
		Audio::Instance* pInstance = SpuPreInstanceUpdate(&params, def);
#else
		Audio::Instance* pInstance = params.pInstance;
#endif

		Audio::Object* pObject = pInstance->GetObject();		

		InstanceUpdate( pInstance, params.bUpdate, params.bGenerate );

#ifdef PLATFORM_SPU
		SpuPostInstanceUpdate(&params, pInstance);
#endif

		Spu::SetMemBase(pMemBase);

		uCount++;
	}
}
