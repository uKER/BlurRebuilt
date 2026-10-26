#include "AudioPch.h"
#include "..\include\Audio.h"
#include "SystemHeaders.h"
#include <system/Perf/Profiler.h>
#include "CpuCounters.h"
#include "StringHash.h"

#include <core/containers.h>

#include <System/memory/MemoryTracker.h>

#include "BatchFrameAdd.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

U32 Audio::ObjectManager::m_uTempFrameBufferSize = 0;
F32* Audio::ObjectManager::m_pfMixerFrames = NULL;
U32 Audio::ObjectManager::m_uPeakTempFrameBufferSize;
U32 Audio::ObjectManager::m_uPeakInstanceStateDataSize;
#ifndef CFG_GOLD
char Audio::ObjectManager::m_pchPeakFrameBufferObjectName[32];
char Audio::ObjectManager::m_pchPeakInstanceStateDataObjectName[32];
#endif
#ifndef CFG_GOLD
Debug::Bool g_DebugLogSuspendAndResumingOfInstances("Audio.System.LogSuspendingAndResumingOfInstances", false);
Debug::Bool g_DebugLogMutingAndUnmutingOfInstances("Audio.System.LogMutingAndUnmutingOfInstances", false);
Debug::Bool g_DebugUpdateAndGenerate("Audio.System.UpdateAndGenerate", false);
Debug::Bool g_DistributeUpdate("Audio.System.DistributeUpdate", true);
Debug::Bool g_DumpProcessingFlag("Audio.System.Performance.DumpProcessingFlag", false);
#endif
Debug::Bool g_bUseBufferAllocator("Audio.System.UseBufferAllocatorForTasks", true);

#ifdef PLATFORM_PS3
Debug::Bool g_DebugJoinBetweenSpuUpdates("Audio.System.JoinOnEveryObject", false);
Debug::Bool g_DebugEnableFrameJoin("Audio.System.OldJoin", false);
#endif

#ifdef PLATFORM_PS3
Debug::Int g_DebugUpdateBatchSize("Audio.System.UpdateBatchSize", 5, Debug::Range(1, 200));
#else
Debug::Int g_DebugUpdateBatchSize("Audio.System.UpdateBatchSize", 20, Debug::Range(1, 200));
#endif

extern Scheduler::jqModule AudioObjectUpdate;

//Debug::Bool g_bDumpXmas("Audio.System.InstanceStats.Dumpxmas", false);
 
////////////////////////////////////////////////////////////////////////////////
//	ObjectManager
////////////////////////////////////////////////////////////////////////////////
Audio::ObjectManager::ObjectManager()
{
    m_pFirst = NULL;
	m_pFirstActive = NULL;
    m_pLast = NULL;
	m_pLastActive = NULL;
	m_u32NumMixersActive = 0;
	m_uPeakTempFrameBufferSize = 0;
	m_uPeakInstanceStateDataSize = 0;
	m_u32NumTotalInstances = 0;
	m_u32NumInstancesMuted = 0;
	#ifndef CFG_GOLD
	memset(m_pchPeakFrameBufferObjectName, 0, lengthof(m_pchPeakFrameBufferObjectName));
	memset(m_pchPeakInstanceStateDataObjectName, 0, lengthof(m_pchPeakInstanceStateDataObjectName));
	#endif

	m_ObjectHandles.InitHandleCollection();
	m_InstanceHandles.InitHandleCollection();
	m_ObjectComponentHandles.InitHandleCollection();

#ifdef PLATFORM_PS3
	m_bDistributeInstanceUpdateTasks = true;
#else
	m_bDistributeInstanceUpdateTasks = false;
#endif

}

////////////////////////////////////////////////////////////////////////////////
//	~ObjectManager
////////////////////////////////////////////////////////////////////////////////
Audio::ObjectManager::~ObjectManager()
{
	BatchFrame::gMoveAudioBatch.Shutdown();
	if (m_pfMixerFrames != NULL)
		Audio::System::Free(m_pfMixerFrames, Audio::System::eVirtual);
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::Init(U32 uTempFrameBufferSize, ISystem::eDistributedUpdateSetting eDistributedUpdate, Scheduler::PolicyHandle hPolicy, Scheduler::GroupHandle hGroup, Scheduler::PolicyHandle hPolicyB/*, U32 uStateDataBufferSize*/)
{
	Assert(uTempFrameBufferSize != 0);
	Assert(uTempFrameBufferSize % (Audio::Core::uFrameLength*sizeof(F32)) == 0);

	MEMORY_TRACKER_PUSH("Audio.DynamicBuffers");

	if (m_pfMixerFrames == NULL)
		m_pfMixerFrames = (F32*)Audio::System::Alloc(uTempFrameBufferSize * 7, 128, Audio::System::eVirtual);

	MEMORY_TRACKER_POP("Audio.DynamicBuffers");

	m_uTempFrameBufferSize = uTempFrameBufferSize;

	m_hAudioPolicy = hPolicy;
	m_hAudioSchedulerGroup = hGroup;
#ifndef CFG_GOLD
	if (eDistributedUpdate != ISystem::eDistribUpdate_UseDefaultSetting)
		g_DistributeUpdate = eDistributedUpdate == ISystem::eDistribUpdate_Enabled ? true : false;
#endif

	m_aBatchListMemory.Init(sizeof(InstanceUpdateParams) * (g_DebugUpdateBatchSize + 1), 50);

	Audio::BatchFrame::gMoveAudioBatch.InitTask( hPolicyB );
	Audio::BatchFrame::gMoveAudioBatch.Init( 2048 );

}

////////////////////////////////////////////////////////////////////////////////
//	Create
////////////////////////////////////////////////////////////////////////////////
Audio::ObjectHandle Audio::ObjectManager::Create(const char* pchFilename, IWaveManager* pWaveManager)
{
	using namespace Audio;

	U8 au8PacketBuffer[512] = {0};

	ObjectHandle hObj = m_ObjectHandles.RequestUnusedHandle();

	if (! m_ObjectHandles.IsHandleValid(hObj))
		return hObj;

	U32 uFnameLen = strlen(pchFilename) + 1;

	U8* pBuffer = au8PacketBuffer;

	((ObjectCreationPacket*)pBuffer)->packet.uMessage = eCreateObject;
	((ObjectCreationPacket*)pBuffer)->hObject = hObj;
	((ObjectCreationPacket*)pBuffer)->pWaveManager = pWaveManager;
	((ObjectCreationPacket*)pBuffer)->uFilenameLen = uFnameLen;

	pBuffer += sizeof(ObjectCreationPacket);

	strcpy_s((char*)pBuffer, lengthof(au8PacketBuffer) - (pBuffer - au8PacketBuffer), pchFilename);

	pBuffer += uFnameLen;

	BIZ_ASSERT((pBuffer - au8PacketBuffer) <= lengthof(au8PacketBuffer));

	AUDIO_SYS_DEBUG("ObjectManager::Create() '%s' [hObj 0x%x]", pchFilename, hObj.Value());

	if (! System::Get()->GetQueue()->SendItem(Audio::Core::SendQueue::eObjectManager, au8PacketBuffer, sizeof(ObjectManager::ObjectCreationPacket) + uFnameLen/* + uEntityNameLen*/))
		hObj.SetAsInvalid();

	return hObj;
}

////////////////////////////////////////////////////////////////////////////////
//	CreateImmediate
////////////////////////////////////////////////////////////////////////////////
Audio::ObjectHandle Audio::ObjectManager::CreateImmediate(const char* pchFilename, IWaveManager* pWaveManager)
{
	ObjectHandle hObject = m_ObjectHandles.RequestUnusedHandle();

	//	load definition or return previously loaded
	Util::C_String strFullPath = GetPath() + pchFilename;

	Definition* pDef = System::Get()->GetDefinitionManager()->Create(strFullPath.Chars());

	if( pDef )
	{
		if( pDef->GetObject() )
		{
			//	already created one, just map handle
			m_ObjectHandles.MapResourceToHandle(hObject, (HandleResource)pDef->GetObject());

			//*****************************************************
			// NOTE: see below
			pDef->GetObject()->SetHandle(hObject);
			//*****************************************************
		}
		else
		{
			Object* pObject = new Object( pDef->FrameSkip() );

			#if defined (AUDIO_ENABLE_PROFILING)
			{
				GetLock()->Acquire();
				m_Objects.PushBack( pObject );
				GetLock()->Release();
			}
			#endif

			pObject->SetObjectManager( this );

			if( pObject )
			{
				AUDIO_SYS_DEBUG("Initializing Object '%s'. [hObj 0x%x]", pDef->Name(), hObject.Value());

				if( Audio::Success == pObject->Init(pDef, pWaveManager) )
				{
					m_ObjectHandles.MapResourceToHandle(hObject, (HandleResource)pObject);

					//*****************************************************
					// NOTE: this code is only needed for the profiler tool
					// but has a problem. it means the handle stored in the
					// object will be the handle of the last object created
					// of this type. this is because we can have multiple
					// handles mapping to a single object.
					pObject->SetHandle(hObject);
					//*****************************************************
				}
				else
				{
					AUDIO_SYS_ERROR("Create object '%s' failed (init failed) [hObj 0x%x]", pchFilename, hObject.Value());
					
					System::Get()->GetDefinitionManager()->Release(pDef);

					m_ObjectHandles.SetResourceAllocationFailed(hObject);
					delete pObject;
					pObject = NULL;
				}
			}
			else
			{
				AUDIO_SYS_ERROR("Create object '%s' failed (internal create) [hObj 0x%x]", pchFilename, hObject.Value());
				
				System::Get()->GetDefinitionManager()->Release(pDef);

				m_ObjectHandles.SetResourceAllocationFailed(hObject);
			}
		}
	}
	else
	{
		m_ObjectHandles.SetResourceAllocationFailed(hObject);
		AUDIO_SYS_ERROR("Create object '%s' failed (definition failed to load) [hObj 0x%x]", strFullPath.Chars(), hObject.Value());
	}

	return hObject;
}

////////////////////////////////////////////////////////////////////////////////
//	CreateUsingHandle
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::CreateUsingHandle(const char* pchFilename, IWaveManager* pWaveManager, ObjectHandle hObject/*, const char* pchEntityName*/)
{
	using namespace Audio;

	U8 au8PacketBuffer[512] = {0};

	U32 uFnameLen = strlen(pchFilename) + 1;

	U8* pBuffer = au8PacketBuffer;

	((ObjectCreationPacket*)pBuffer)->packet.uMessage = eCreateObject;
	((ObjectCreationPacket*)pBuffer)->hObject = hObject;
	((ObjectCreationPacket*)pBuffer)->pWaveManager = pWaveManager;
	((ObjectCreationPacket*)pBuffer)->uFilenameLen = uFnameLen;

	pBuffer += sizeof(ObjectCreationPacket);

	strcpy_s((char*)pBuffer, lengthof(au8PacketBuffer) - (pBuffer - au8PacketBuffer), pchFilename);

	pBuffer += uFnameLen;

	BIZ_ASSERT((pBuffer - au8PacketBuffer) <= lengthof(au8PacketBuffer));

	AUDIO_SYS_DEBUG("ObjectManager::Create() '%s' [hObj 0x%x]", pchFilename, hObject.Value());

	if (! System::Get()->GetQueue()->SendItem(Audio::Core::SendQueue::eObjectManager, au8PacketBuffer, sizeof(ObjectManager::ObjectCreationPacket) + uFnameLen/* + uEntityNameLen*/))
		hObject.SetAsInvalid();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	InternalCreate
//	- not used by CreateImmediate()
////////////////////////////////////////////////////////////////////////////////
Audio::Object* Audio::ObjectManager::InternalCreate(U32 uFrameSkip)
{
	Object* pNewObject = new Object(uFrameSkip);

	if (! pNewObject)
		return 0;

	#if defined (AUDIO_ENABLE_PROFILING)
	{
		GetLock()->Acquire();
		m_Objects.PushBack(pNewObject);
		GetLock()->Release();
	}
	#endif

    pNewObject->SetObjectManager(this);

	return pNewObject;
}

////////////////////////////////////////////////////////////////////////////////
//	Destroy
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::ObjectManager::Destroy(ObjectHandle hObject, bool bReserveHandle)
{
	if (! m_ObjectHandles.IsHandleValid(hObject))
		return Failed;

	if (! m_ObjectHandles.FlagHandleForRelease(hObject))
	{
		AUDIO_SYS_ERROR("couldn't release handle %d", hObject.Value());
		return Failed;
	}

	ObjectDestructionPacket thePacket;

	thePacket.packet.uMessage = eDestroyObject;
	thePacket.hObject = hObject;
	thePacket.bReserveHandle = bReserveHandle;

	AUDIO_SYS_DEBUG("ObjectManager::Destroy() [hObj 0x%x]", hObject.Value());

	return (System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(ObjectDestructionPacket))) ? Success : Failed;
}

////////////////////////////////////////////////////////////////////////////////
//	InternalDestroy
////////////////////////////////////////////////////////////////////////////////
#if 0
Audio::Result Audio::ObjectManager::InternalDestroy(ObjectHandle hObject, bool bReserveHandle)
{
	if (!m_ObjectHandles.IsHandleValid(hObject))
		return InvalidHandle;

	Object* pObject = GetDoomedObject(hObject);

	if ((!pObject) && (! IsObjectReservedForUse(hObject)))
	{
		AUDIO_SYS_ERROR("ObjectManager::InternalDestroy() GetDoomedObject returned NULL! [hObj 0x%x]", hObject.Value());
		return Failed;
	}

	if (pObject)
	{
		bool bReleaseObject = false;

		if (pObject->RefCount() == 1)
		{
			bReleaseObject = true;
		}

		AUDIO_SYS_DEBUG("Received destroy on object %s, ref count is %d\n", pObject->Name(), pObject->RefCount());

		if (bReleaseObject)
		{
			// should this be fatal..?
			if (pObject->NumInstances())
			{
				AUDIO_SYS_ERROR("Destroying object %s with active instances! [hObj 0x%x]", pObject->Name(), hObject.Value());
				return Failed;
			}

			GetLock()->Acquire();
			for (int i=0; i<m_Objects.Size(); i++)
			{
				if (m_Objects[i] == pObject)
				{
					m_Objects.EraseByIndex(i);
					break;
				}
			}
			GetLock()->Release();

			pObject->DecRefCount();

			Definition* pDef = pObject->GetDefinition();

	#ifndef CFG_SPEED
			// final sanity check
			for (Instance* pInst = m_pFirst; pInst; pInst = pInst->Next())
			{
				if (pInst->GetObject() == pObject || pInst->GetObject()->GetDefinition() == pDef)
				{
					AUDIO_SYS_FATAL("About to destroy object and definition that an instance is still referring to!");
					DebugBreak();
				}
			}
	#endif

			pObject->Release();
			delete pObject;

			if (pDef)
			{
				System::Get()->GetDefinitionManager()->RemoveFromList(pDef);
				System::Get()->GetDefinitionManager()->Destroy(pDef);
			}
		}		
		else
			pObject->DecRefCount();
	}

	if (m_ObjectHandles.IsReleasePending(hObject))
	{
		m_ObjectHandles.ReleaseHandle(hObject);

		if (bReserveHandle)
			m_ObjectHandles.ReserveHandle(hObject);
	}
	else
		m_ObjectHandles.SetResourceAllocationFailed(hObject);

    return Success;
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	CreateObjectInstance
////////////////////////////////////////////////////////////////////////////////
Audio::InstanceHandle Audio::ObjectManager::CreateObjectInstance(ObjectHandle hObject, const char* pchGroupName, const char* pchEntityName)
{
	bool bFail = false;
	InstanceHandle hInstance;

	if (! m_ObjectHandles.IsHandleValid(hObject))
		bFail = true;
	else
	{
		hInstance = m_InstanceHandles.RequestUnusedHandle();

		if (! m_InstanceHandles.IsHandleValid(hInstance))
		{
			bFail = true;
		}
	}

	if (bFail == true)
	{
		AUDIO_SYS_ERROR("Create Object Instance failed ['%s' '%s' hObj 0x%x]", pchEntityName?pchEntityName:"", pchGroupName?pchGroupName:"", hObject.Value());
		return hInstance;
	}

	InstanceCreationPacket thePacket;

	thePacket.packet.uMessage = eCreateInstance;
	thePacket.hObject = hObject;
	thePacket.hInstance = hInstance;

	#if !defined (PLATFORM_PS3) && !defined (PLATFORM_SPU) && !defined CFG_GOLD
	if (pchGroupName)
	{
		strncpy(thePacket.pchGroupName, pchGroupName, 15);
		thePacket.pchGroupName[15] = 0;
	}
	else
		thePacket.pchGroupName[0] = 0;

	if (pchEntityName)
	{
		strncpy(thePacket.pchEntityName, pchEntityName, 15);
		thePacket.pchEntityName[15] = 0;
	}
	else
		thePacket.pchEntityName[0] = 0;
	#endif

	AUDIO_SYS_DEBUG("ObjectManager::CreateObjectInstance() Entity '%s' Group '%s' hInst 0x%x hObj 0x%x]", pchEntityName?pchEntityName:"", pchGroupName?pchGroupName:"", hInstance.Value(), hObject.Value());

	System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(InstanceCreationPacket));

	return hInstance;
}

////////////////////////////////////////////////////////////////////////////////
//	CreateObjectInstanceUsingHandle
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::CreateObjectInstanceUsingHandle(ObjectHandle hObject, InstanceHandle hInstance, const char* pchGroupName, const char* pchEntityName)
{
	bool bFail = false;

	if (! m_ObjectHandles.IsHandleValid(hObject))
	{
		AUDIO_SYS_ERROR("Create Object Instance Using Handle failed. [hInst 0x%x, hObj 0x%x]", hInstance.Value(), hObject.Value());
		return false;
	}

	InstanceCreationPacket thePacket;

	thePacket.packet.uMessage = eCreateInstance;
	thePacket.hObject = hObject;
	thePacket.hInstance = hInstance;

	#if !defined (PLATFORM_PS3) && !defined (PLATFORM_SPU) && !defined CFG_GOLD
	if (pchGroupName)
	{
		strncpy(thePacket.pchGroupName, pchGroupName, 15);
		thePacket.pchGroupName[15] = 0;
	}
	else
		memset(thePacket.pchGroupName, 0 , 16);
	if (pchEntityName)
	{
		strncpy(thePacket.pchEntityName, pchEntityName, 15);
		thePacket.pchEntityName[15] = 0;
	}
	else
		memset(thePacket.pchEntityName, 0 , 16);
	#endif

	AUDIO_SYS_DEBUG("ObjectManager::CreateObjectInstanceUsingHandle() [hInst 0x%x hObj 0x%x]", hInstance.Value(), hObject.Value());

	System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(InstanceCreationPacket));

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	DestroyObjectInstance
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::DestroyObjectInstance(Audio::InstanceHandle hInstance, bool bReserveHandle)
{
	if (! m_InstanceHandles.FlagHandleForRelease(hInstance))
	{
		AUDIO_SYS_ERROR("couldn't release handle %d", hInstance.Value());
		return;
	}

	InstanceDestructionPacket thePacket;

	thePacket.packet.uMessage = eDestroyInstance;
	thePacket.hInstance = hInstance;
	thePacket.bReserveHandle = bReserveHandle;

	AUDIO_SYS_DEBUG("ObjectManager::DestroyObjectInstance() [hInst 0x%x]", hInstance.Value());

	System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(InstanceDestructionPacket));
}

////////////////////////////////////////////////////////////////////////////////
//	LogActiveInstanceStates
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::LogActiveInstanceStates()
{
	#ifndef CFG_GOLD
	AUDIO_SYS_DEBUG("================ ACTIVE Instance state dump ================");
    Instance* pInstance = m_pFirstActive;
	while (pInstance)
    {
		AUDIO_SYS_DEBUG("%s state flags 0x%x [%20s, %20s, %20s, %20s, %20s, %20s, %20s, %20s]", pInstance->GetObject()->Name(), pInstance->GetStateFlags(),
				pInstance->StateFlagIsSet(InstanceNotActivated) ? "NotActivated" : "", 
				pInstance->StateFlagIsSet(InstanceActivationPending) ? "ActivationPending" : "", 
				pInstance->StateFlagIsSet(InstanceActivated) ? "Activated" : "", 
				pInstance->StateFlagIsSet(InstanceDeactivationPending) ? "DeactivationPending" : "", 
				pInstance->StateFlagIsSet(InstanceReleasePending) ? "ReleasePending" : "", 
				pInstance->StateFlagIsSet(InstanceSuspendPending) ? "SuspendPending" : "", 
				pInstance->StateFlagIsSet(InstanceSuspended) ? "Suspended" : "", 
				pInstance->StateFlagIsSet(InstanceResumePending) ? "ResumePending" : "");

        pInstance = pInstance->NextActive();
	}
	AUDIO_SYS_DEBUG("============== ACTIVE Instance state dump END ==============");
	#endif
}

////////////////////////////////////////////////////////////////////////////////
//	LogAllInstanceStates
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::LogAllInstanceStates()
{
	#ifndef CFG_GOLD
	AUDIO_SYS_DEBUG("================ ALL Instance state dump ================");
    Instance* pInstance = m_pFirst;
	while (pInstance)
    {
		AUDIO_SYS_DEBUG("%s state flags 0x%x [%20s, %20s, %20s, %20s, %20s, %20s, %20s, %20s]", pInstance->GetObject()->Name(), pInstance->GetStateFlags(),
				pInstance->StateFlagIsSet(InstanceNotActivated) ? "NotActivated" : "", 
				pInstance->StateFlagIsSet(InstanceActivationPending) ? "ActivationPending" : "", 
				pInstance->StateFlagIsSet(InstanceActivated) ? "Activated" : "", 
				pInstance->StateFlagIsSet(InstanceDeactivationPending) ? "DeactivationPending" : "", 
				pInstance->StateFlagIsSet(InstanceReleasePending) ? "ReleasePending" : "", 
				pInstance->StateFlagIsSet(InstanceSuspendPending) ? "SuspendPending" : "", 
				pInstance->StateFlagIsSet(InstanceSuspended) ? "Suspended" : "", 
				pInstance->StateFlagIsSet(InstanceResumePending) ? "ResumePending" : "");

        pInstance = pInstance->Next();
	}
	AUDIO_SYS_DEBUG("============== ALL Instance state dump END ==============");
	#endif
}

////////////////////////////////////////////////////////////////////////////////
//    Update
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::Update()
{
	AUDIO_PERF_EVENT("Audio.ObjectManager.Update");
	
	bool bUseBufferAllocator = g_bUseBufferAllocator;
	U32 uBatchSize = g_DebugUpdateBatchSize;

	Btl::Vector<InstanceUpdateParams*> vBatches;

	InstanceUpdateParams* aParams = NULL;
	U32 uCount = 0;

#ifndef CFG_GOLD
	bool bDistributeTasks = g_DistributeUpdate;
#else
	bool bDistributeTasks = true;
#endif

	//bool bDistributeTasks = m_bDistributeInstanceUpdateTasks;
	U32 uFrameNum = System::Get()->GetFrameNum();

#ifdef PLATFORM_PS3
	System::Get()->PopulateSpuInfoStruct();
#endif

	U32 u32NumInstancesActive = 0;

	Instance* pInstance = m_pFirstActive;
	bool bUpdate = false;
	bool bGenerate = false;
	bool bHaveSentTask = false;

	while (pInstance)
    {
		OBJECT_CPU_COUNTER_BEGIN(pInstance);

		u32NumInstancesActive++;

		bool bWasMuted = pInstance->WasMuted();
		bool bIsMuted = pInstance->IsMuted();
		bool bWasSuspended = pInstance->WasSuspended();
		bool bIsSuspended = pInstance->IsSuspended();
		Core::ProcessInfo& aInfo = pInstance->m_coreInfo;

		aInfo.bMutePrev = bWasMuted & (!bIsMuted);
		aInfo.bMuteNext = (!bWasMuted) & bIsMuted;

		aInfo.bSuspendPrev = bWasSuspended & (!bIsSuspended);
		aInfo.bSuspendNext = (!bWasSuspended) & bIsSuspended;

		//if(bWasMuted && !bIsMuted)
		//{
		//	aInfo.bMutePrev = true;
		//	aInfo.bMuteNext = false;
		//}
		//else if(!bWasMuted && bIsMuted)
		//{
		//	aInfo.bMutePrev = false;
		//	aInfo.bMuteNext = true;
		//}
		//else
		//{
		//	aInfo.bMutePrev = false;
		//	aInfo.bMuteNext = false;
		//}

		//if(bWasSuspended && !bIsSuspended)
		//{
		//	aInfo.bSuspendPrev = true;
		//	aInfo.bSuspendNext = false;
		//}
		//else if(!bWasSuspended && bIsSuspended)
		//{
		//	aInfo.bSuspendPrev = false;
		//	aInfo.bSuspendNext = true;
		//}
		//else
		//{
		//	aInfo.bSuspendPrev = false;
		//	aInfo.bSuspendNext = false;
		//}

		pInstance->PrevMute(bIsMuted);
		pInstance->PrevSuspend(bIsSuspended);

		//	setup core info
		Object* pObj = pInstance->GetObject();
		U32 uFrameInt = (U16) pObj->FrameSkip();
		F32 fFrameSkip = pObj->FrameSkipF();
		F32 fFrameInt = aInfo.fFrameInt;
		U32 uFrameMod = aInfo.uFrameMod + 1;
		F32 fFrameMod = aInfo.fFrameMod + 1.0f;

		if( uFrameMod >= uFrameInt )
		{
			uFrameMod = 0;
			fFrameMod = 0.0f;
		}

		F32 fRcpFrame = Maths::RcpEst(fFrameInt);
		F32 fFraction = fFrameMod * fRcpFrame;

		aInfo.uFrameNum = uFrameNum;
		aInfo.uFrameInt = uFrameInt;
		aInfo.fFrameInt = fFrameSkip;
		aInfo.uFrameMod = uFrameMod;
		aInfo.fFrameMod = fFrameMod;
		aInfo.fFraction = fFraction;
		aInfo.fWidth = fRcpFrame;

		aInfo.pCoreData = pInstance->GetCoreData();

		aInfo.pDynamicBuffers = m_pfMixerFrames;
		aInfo.pStaticBuffers = (F32*)pInstance->GetBufferData();

		bUpdate = ! (bIsSuspended || uFrameMod);

		//	need to update?
		//if (!bIsSuspended)
		//{
		//	if (uFrameMod == 0)
		//		bUpdate = true;
		//	else
		//		bUpdate = false;
		//}
		//else
		//	bUpdate = false;

		bGenerate = !(bWasSuspended && bIsSuspended) && !(bWasMuted && bIsMuted);

		//	need to generate?
		//if (!(bWasSuspended && bIsSuspended) &&
		//	!(bWasMuted && bIsMuted))
		//{
		//	bGenerate = true;
		//}
		//else
		//	bGenerate = false;

		OBJECT_CPU_COUNTER_END(pInstance);

		if (bUpdate || bGenerate)
		{
			if (bDistributeTasks)
			{
				InstanceUpdateParams* pParams = pInstance->GetDistributedTaskData();

				pParams->pInstance = pInstance;
				pParams->bUpdate = bUpdate;
				pParams->bGenerate = bGenerate;
				static bool bBreakInstanceUpdate = false;
				pParams->bBreak = bBreakInstanceUpdate;

#ifdef PLATFORM_PS3
				pParams->m_SpuInfo.pPpuObject = pInstance->GetObject();
				pParams->m_SpuInfo.pPpuDefinitionData = pParams->m_SpuInfo.pPpuObject->GetDefinition()->Data();
#ifdef NO_STRING_TABLE_ON_SPU
				pParams->m_SpuInfo.u32DefinitionDataSize = pParams->m_SpuInfo.pPpuObject->GetDefinition()->GetSpuDataSize();
#else
				pParams->m_SpuInfo.u32DefinitionDataSize = pParams->m_SpuInfo.pPpuObject->GetDefinition()->GetDataSize();
#endif
				pParams->m_SpuInfo.pPpuObjectComponents = pParams->m_SpuInfo.pPpuObject->GetComponents();
				pParams->m_SpuInfo.u32ObjectComponentsSize = pParams->m_SpuInfo.pPpuObject->GetComponentFootprint() + pParams->m_SpuInfo.pPpuObject->GetComponentIndexArraySize();
				pParams->m_SpuInfo.pPpuInstanceContext = pInstance->GetStateData();
				pParams->m_SpuInfo.u32InstanceContextSize = (pInstance->GetStateSize() + ((pParams->m_SpuInfo.pPpuObject->GetDefinition()->NumComponents() * sizeof(F32)) + 0x0f) & ~0x0f);
				pParams->m_SpuInfo.pPpuInstanceCoreData = pInstance->GetCoreData();
				pParams->m_SpuInfo.u32InstanceCoreDataSize = pInstance->GetCoreDataSize();
				pParams->m_SpuInfo.pPpuInstanceBufferData = pInstance->GetBufferData();
				pParams->m_SpuInfo.u32InstanceBufferDataSize = pInstance->GetBufferDataSize();
				pParams->m_SpuInfo.pPpuSystemState = System::Get()->GetSystemSpuInfoAddress();
				pParams->m_SpuInfo.u32DynamicBufferSize = pParams->m_SpuInfo.pPpuObject->GetDynamicBufferSize() * sizeof(F32);
				pParams->m_SpuInfo.u32ScratchDmaBufferSize = pParams->m_SpuInfo.pPpuObject->GetDmaBufferSize();
#endif

				if( uCount == 0 )
				{
					//	1st of new batch

					if (bUseBufferAllocator)
						aParams = (InstanceUpdateParams*)(m_aBatchListMemory.GetBuffer(sizeof(InstanceUpdateParams) *  (uBatchSize + 1)));
					else
					{
						aParams = new InstanceUpdateParams[uBatchSize];
						vBatches.PushBack( aParams );
					}
				}

				//	add this task to batch
				aParams[ uCount++ ] = *(pParams);

				if( uCount >= (U32)uBatchSize )
				{
					//	end this batch
//					apParams[ uCount ] = NULL;

					Scheduler::AddTask( AudioObjectUpdate, aParams, NULL, m_hAudioSchedulerGroup, m_hAudioPolicy, &uCount, 4 );

					uCount = 0;
				}

				bHaveSentTask = true;
			}
			else
				InstanceUpdate(pInstance, bUpdate, bGenerate);
		}

        pInstance = pInstance->NextActive();
    }

	if (uCount != 0)
	{
		//	do final batch, if req
		Scheduler::AddTask(AudioObjectUpdate, aParams, NULL, m_hAudioSchedulerGroup, m_hAudioPolicy, &uCount, 4);
		bHaveSentTask = true;
	}

	if (bHaveSentTask)
		Scheduler::Flush(m_hAudioSchedulerGroup, false);

	//	free this update's batches
	if (bUseBufferAllocator)
		m_aBatchListMemory.FreeAllBuffers();
	else
	{
		for (S32 i = 0; i < vBatches.Size(); i++)
			delete [] vBatches[i];

		vBatches.Clear();
	}

	m_u32NumInstancesActive = u32NumInstancesActive;

	AUDIO_PERF_BEGIN_EVENT("Audio.Connections");

#ifndef CFG_GOLD
	bool bDumpProcessFlags = g_DumpProcessingFlag;
	if( g_DumpProcessingFlag )
		g_DumpProcessingFlag = false;
#endif

	// Clear the Audio Frame Butcher
	Audio::BatchFrame::gMoveAudioBatch.Clear();

	// process connections
    pInstance = m_pFirstActive;
    while (pInstance)
    {
		pInstance->m_coreInfo.pCoreData = pInstance->GetCoreData();

		pInstance->m_coreInfo.pDynamicBuffers = m_pfMixerFrames;
		pInstance->m_coreInfo.pStaticBuffers = (F32*)pInstance->GetBufferData();

		AUDIO_PERF_BEGIN_EVENT("Audio.ProcessConnections");

		pInstance->GetObject()->ProcessConnections(pInstance);

		AUDIO_PERF_END_EVENT();

#ifndef CFG_GOLD
		if( bDumpProcessFlags )
			pInstance->GetObject()->DumpProcessFlags( pInstance );
#endif

#ifdef PLATFORM_PS3
		if( pInstance->m_coreInfo.uFrameMod == 0 )
		{
			AUDIO_PERF_BEGIN_EVENT("Audio.ProcessActionOuts");

			//	only on frameskip
			Object* pObj = pInstance->GetObject();
			pObj->ProcessActionOuts(pInstance);

			AUDIO_PERF_END_EVENT();
		}
#endif

		U32 u32StreamsNeedingUpdate = pInstance->GetNumStreamsNeedingUpdate();
		if (u32StreamsNeedingUpdate > 0)
		{
			U32 u32StreamsUpdated = 0;
			for (U32 i = 0; i < pInstance->GetNumberOfStreamsCreated() && u32StreamsUpdated < u32StreamsNeedingUpdate; i++)
			{
				Core::WaveStreamData* pData = pInstance->GetWaveServerStreamData(i);

				if (pData->bRequiresUpdate)
				{
					Audio::System::Get()->GetWaveServer()->UpdateStream(*pData, (U32*)pInstance->GetWaveServerData());
					u32StreamsUpdated++;
				}
			}
		}

		pInstance = pInstance->NextActive();
	}

	AUDIO_PERF_BEGIN_EVENT("Audio.ProcessAudioBatches");

	Audio::BatchFrame::gMoveAudioBatch.Process();
	Audio::BatchFrame::gMoveAudioBatch.Wait();

	AUDIO_PERF_END_EVENT();

	AUDIO_PERF_END_EVENT();
}


bool bShowGenerationInfo = false;

////////////////////////////////////////////////////////////////////////////////
//	IsInstanceValid
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::IsInstanceValid(InstanceHandle hInstance)
{
	if (! m_InstanceHandles.IsHandleValid(hInstance))
		return false;

	return m_InstanceHandles.IsReservedOrInUse(hInstance);
}

////////////////////////////////////////////////////////////////////////////////
//	IsObjectValid
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::IsObjectValid(ObjectHandle hObject)
{
	if (! m_ObjectHandles.IsHandleValid(hObject))
		return false;

	return m_ObjectHandles.IsReservedOrInUse(hObject);
}

////////////////////////////////////////////////////////////////////////////////
//	IsObjectComponentValid
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::IsObjectComponentValid(ObjectComponentHandle hComponent)
{
	if (! m_ObjectComponentHandles.IsHandleValid(hComponent))
		return false;

	return m_ObjectComponentHandles.IsReservedOrInUse(hComponent);
}

////////////////////////////////////////////////////////////////////////////////
//	IsInstanceComponentValid
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::IsInstanceComponentValid(InstanceComponentHandle hComponent)
{
	ObjectComponentHandle hObjComp = ExtractObjectComponentHandle(hComponent);
	InstanceHandle hInst = ExtractInstanceHandle(hComponent);

	return (IsObjectComponentValid(hObjComp) && IsInstanceValid(hInst));
}

////////////////////////////////////////////////////////////////////////////////
//	Suspend
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::Suspend(InstanceHandle hInstance, bool bSuspend)
{
	if (! IsInstanceValid(hInstance))
	{
		AUDIO_SYS_WARN("Trying to suspend an invalid instance");
		return false;
	}

    InstancePacket thePacket;

    thePacket.hInstance = hInstance;
	thePacket.packet.uMessage = bSuspend ? eSuspend : eResume;

	//AUDIO_SYS_DEBUG("ObjectManager::Suspend(%s)]", bSuspend ? "true" : "false");

	return System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(InstancePacket));
}

////////////////////////////////////////////////////////////////////////////////
//	IsSuspended
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::IsSuspended(InstanceHandle hInstance)
{
	Instance* pInstance = GetInstance(hInstance);

	if (! pInstance)
	{
		AUDIO_SYS_WARN("FAIL: IsSuspended failed (no instance) [hInst 0x%x]", hInstance.Value());
		return false;
	}

	return pInstance->IsSuspended();
}

////////////////////////////////////////////////////////////////////////////////
//	Mute
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::Mute(InstanceHandle hInstance, bool bMute)
{
	if (! IsInstanceValid(hInstance))
	{
		AUDIO_SYS_WARN("Trying to mute an invalid instance");
		return false;
	}

    InstancePacket thePacket;

    thePacket.hInstance = hInstance;
	thePacket.packet.uMessage = bMute ? eMute : eUnmute;

	AUDIO_SYS_DEBUG("ObjectManager::Mute(%s)", bMute ? "true" : "false");

	return System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(InstancePacket));
}

////////////////////////////////////////////////////////////////////////////////
//	IsMuted
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::IsMuted(InstanceHandle hInstance)
{
	Instance* pInstance = GetInstance(hInstance);

	if (! pInstance)
	{
		AUDIO_SYS_WARN("FAIL: IsMuted failed (no instance) [hInst 0x%x]", hInstance.Value());
		return false;
	}

	return pInstance->IsMuted();
}

////////////////////////////////////////////////////////////////////////////////
//	ConnectGeneric
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::ConnectGeneric(InstanceComponentHandle hOut, InstanceComponentHandle hIn)
{
#ifndef CFG_SPEED
	if (! (IsInstanceComponentValid(hOut) && IsInstanceComponentValid(hIn)))
	{
		AUDIO_SYS_WARN("Trying to connect using one or more invalid instance components");
		return false;
	}
#endif

	ConnectionPacket thePacket;

	thePacket.packet.uMessage = eConnectGeneric;
	thePacket.hOut = hOut;
	thePacket.hIn = hIn;

	AUDIO_SYS_DEBUG("ObjectManager::ConnectGeneric() [hInst 0x%x hObjCom 0x%x to hInst 0x%x hObjCom 0x%x]", hOut.GetInstanceHandle().Value(), hOut.GetObjectComponentHandle().Value(), hIn.GetInstanceHandle().Value(), hIn.GetObjectComponentHandle().Value());

	return System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(ConnectionPacket));
}

////////////////////////////////////////////////////////////////////////////////
//	DisconnectGeneric
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::DisconnectGeneric(InstanceComponentHandle hOut, InstanceComponentHandle hIn)
{
#ifndef CFG_SPEED
	if (! (IsInstanceComponentValid(hOut) && IsInstanceComponentValid(hIn)))
	{
		AUDIO_SYS_WARN("Trying to disconnect using one or more invalid instance components");
		return false;
	}
#endif

	GenericDisconnectionPacket thePacket;

	thePacket.packet.uMessage = eDisconnectGeneric;
	thePacket.hOut = hOut;
	thePacket.hIn = hIn;

	AUDIO_SYS_DEBUG("ObjectManager::DisconnectGeneric() [hInst 0x%x hObjCom 0x%x to hInst 0x%x hObjCom 0x%x]", hOut.GetInstanceHandle().Value(), hOut.GetObjectComponentHandle().Value(), hIn.GetInstanceHandle().Value(), hIn.GetObjectComponentHandle().Value());

	return System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(GenericDisconnectionPacket));
}

////////////////////////////////////////////////////////////////////////////////
//	RegisterActionOutCallback
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::RegisterActionOutCallback(InstanceComponentHandle hActionOut, AudioActionOutCallback pCallback, U32 u32AppData)
{
#ifndef CFG_SPEED
	if (! IsInstanceComponentValid(hActionOut))
	{
		AUDIO_SYS_WARN("Trying to register action out callback using invalid instance component");
		return false;
	}
#endif

	RegisterActionOutCallbackPacket thePacket;

	thePacket.packet.uMessage = eRegisterActionOutCallback;
	thePacket.hActionOut = hActionOut;
	thePacket.pCallback = pCallback;
	thePacket.u32AppData = u32AppData;

	AUDIO_SYS_DEBUG("ObjectManager::RegisterActionOutCallback()");

	return System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(RegisterActionOutCallbackPacket));
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveData
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveData(void* pData, U32 uSize)
{
    Packet* pPacket = (Packet*)pData;

	switch (pPacket->uMessage)
	{
		case eSuspend:
		{
			InstancePacket* pPacket = (InstancePacket*)pData;
			Assert(uSize == sizeof(InstancePacket));

			ReceiveSuspend(pPacket->hInstance);
		}
		break;

		case eResume:
		{
			InstancePacket* pPacket = (InstancePacket*)pData;
			Assert(uSize == sizeof(InstancePacket));

			ReceiveResume(pPacket->hInstance);
		}
		break;

		case eMute:
		{
			InstancePacket* pPacket = (InstancePacket*)pData;
			Assert(uSize == sizeof(InstancePacket));

			ReceiveMute(pPacket->hInstance);
		}
		break;

		case eUnmute:
		{
			InstancePacket* pPacket = (InstancePacket*)pData;
			Assert(uSize == sizeof(InstancePacket));

			ReceiveUnmute(pPacket->hInstance);
		}
		break;

		case eConnectGeneric:
		{
			ConnectionPacket* pPacket = (ConnectionPacket*)pData;
			Assert(uSize == sizeof(ConnectionPacket));

			ReceiveConnectGeneric(pPacket->hOut, pPacket->hIn);
		}
		break;

		case eDisconnectGeneric:
		{
			GenericDisconnectionPacket* pPacket = (GenericDisconnectionPacket*)pData;
			Assert(uSize == sizeof(GenericDisconnectionPacket));

			ReceiveDisconnectGeneric(pPacket->hOut, pPacket->hIn);
		}
		break;

		case eCreateObject:
		{
			ObjectCreationPacket* pPacket = (ObjectCreationPacket*)pData;
			ReceiveCreate(pPacket->hObject, (char*)pData + sizeof(ObjectManager::ObjectCreationPacket), pPacket->pWaveManager);
		}
		break;

		case eDestroyObject:
		{
			ObjectDestructionPacket* pPacket = (ObjectDestructionPacket*)pData;
			ReceiveDestroy(pPacket->hObject, pPacket->bReserveHandle);
		}
		break;

		case eCreateInstance:
		{
			InstanceCreationPacket* pPacket = (InstanceCreationPacket*)pData;
			#if !defined PLATFORM_SPU && !defined PLATFORM_PS3 && !defined CFG_GOLD
			ReceiveCreateInstance(pPacket->hObject, pPacket->hInstance, pPacket->pchGroupName, pPacket->pchEntityName);
			#else
			ReceiveCreateInstance(pPacket->hObject, pPacket->hInstance);
			#endif
		}
		break;

		case eDestroyInstance:
		{
			InstanceDestructionPacket* pPacket = (InstanceDestructionPacket*)pData;
			ReceiveDestroyInstance(pPacket->hInstance, pPacket->bReserveHandle);
		}
		break;

		case eReleaseObjectComponentHandle:
		{
			ReleaseObjectComponentHandlePacket* pPacket = (ReleaseObjectComponentHandlePacket*)pData;
			ReceiveReleaseObjectComponentHandle(pPacket->hObjComp, pPacket->bReserveHandle);
		}
		break;

		case eGetObjectActionIn:
		case eGetObjectActionOut:
		case eGetObjectParameterIn:
		case eGetObjectParameterOut:
		case eGetObjectVectorIn:
		case eGetObjectControlInput:
		case eGetObjectEventInput:
		case eGetObjectVectorInput:
		case eGetObjectGraphInput:
		case eGetObjectMonoAudioInput:
		case eGetObjectStereoAudioInput:
		case eGetObjectSurroundAudioInput:
		case eGetObjectSurroundLFEAudioInput:
		case eGetObjectControlOutput:
		case eGetObjectEventOutput:
		case eGetObjectVectorOutput:
		case eGetObjectGraphOutput:
		case eGetObjectMonoAudioOutput:
		case eGetObjectStereoAudioOutput:
		case eGetObjectSurroundAudioOutput:
		case eGetObjectSurroundLFEAudioOutput:
		case eGetComponentGeneric:
		{
			GetComponentPacket* pPacket = (GetComponentPacket*)pData;
			ReceiveGetComponent(pPacket->packet.uMessage, (char*)pData + sizeof(ObjectManager::GetComponentPacket), pPacket->hObject, pPacket->hComponent);
		}
		break;

		case eGetObjectActionInFromHash:
		case eGetObjectActionOutFromHash:
		case eGetObjectParameterInFromHash:
		case eGetObjectParameterOutFromHash:
		case eGetObjectVectorInFromHash:
		case eGetObjectControlInputFromHash:
		case eGetObjectEventInputFromHash:
		case eGetObjectVectorInputFromHash:
		case eGetObjectGraphInputFromHash:
		case eGetObjectMonoAudioInputFromHash:
		case eGetObjectStereoAudioInputFromHash:
		case eGetObjectSurroundAudioInputFromHash:
		case eGetObjectSurroundLFEAudioInputFromHash:
		case eGetObjectControlOutputFromHash:
		case eGetObjectEventOutputFromHash:
		case eGetObjectVectorOutputFromHash:
		case eGetObjectGraphOutputFromHash:
		case eGetObjectMonoAudioOutputFromHash:
		case eGetObjectStereoAudioOutputFromHash:
		case eGetObjectSurroundAudioOutputFromHash:
		case eGetObjectSurroundLFEAudioOutputFromHash:
		case eGetComponentGenericFromHash:
		{
			GetComponentFromHashPacket* pPacket = (GetComponentFromHashPacket*)pData;
			ReceiveGetComponentFromHash(pPacket->packet.uMessage, pPacket->nameHash, pPacket->hObject, pPacket->hComponent);
		}
		break;

		case eSetValue:
		{
			SetValuePacket* pPacket = (SetValuePacket*)pData;
			ReceiveSetValue(pPacket->hParamIn, pPacket->fValue);
		}
		break;

		case eValue:
		{
			GetValuePacket* pPacket = (GetValuePacket*)pData;
			ReceiveValue(pPacket->hParamOut, pPacket->hDataTracker, pPacket->pCallback);
		}
		break;

		case eExecute:
		{
			ExecutePacket* pPacket = (ExecutePacket*)pData;
			ReceiveExecute(pPacket->hActionIn);
		}
		break;

		case eSetDimensions:
		{
			SetDimensionsPacket* pPacket = (SetDimensionsPacket*)pData;
			ReceiveSetDimensions(pPacket->hVectorIn, pPacket->afDimensions);
		}
		break;

		case eRegisterActionOutCallback:
		{
			RegisterActionOutCallbackPacket* pPacket = (RegisterActionOutCallbackPacket*)pData;
			ReceiveRegisterActionOutCallback(pPacket->hActionOut, pPacket->pCallback, pPacket->u32AppData);
		}
		break;

		default:
		{
			AUDIO_SYS_ERROR("ObjectManager::ReceiveData() unknown message");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveActivate
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::Activate(Instance* pInstance)
{
	if (! pInstance)
	{
		AUDIO_SYS_ERROR("Instance activate failed");
		return;
	}

#ifndef CFG_GOLD
	Instance* pCheckForActivated = m_pFirst;
	while (pCheckForActivated != NULL)
	{
		if (pCheckForActivated == pInstance)
			AUDIO_SYS_FATAL("ObjectManager::ReceiveActivate() : instance '%s' already activated", pInstance->GetObject()->Name());

		pCheckForActivated = pCheckForActivated->Next();
	}
#endif

	pInstance->GetObject()->IncrementInstanceCounter();

	if ((m_pFirst == NULL) && (m_pLast == NULL))
        //  list empty
        m_pFirst = m_pLast = pInstance;
    else
    {
        //  add to end of list
        m_pLast->SetNext(pInstance);
        pInstance->SetPrev(m_pLast);
        m_pLast = pInstance;
    }

	LinkActive(pInstance);

	AUDIO_SYS_DEBUG("ReceiveActivate() : instance '%s'", pInstance->GetObject()->Name());

    pInstance->SetStateFlag(InstanceActivated);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveDeactivate
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::Deactivate(Instance* pInstance)
{
	if (! pInstance)
	{
		AUDIO_SYS_ERROR("Instance deactivate failed");
		return;
	}

#ifndef CFG_GOLD
	Instance* pCheckForActivated = m_pFirst;
	while (pCheckForActivated != NULL)
	{
		if (pCheckForActivated == pInstance)
			break;

		pCheckForActivated = pCheckForActivated->Next();
	}
	if (pCheckForActivated == NULL)
	{
		Instance* pCurr = m_pFirst;
		while (pCurr != NULL)
		{
			printf("[0x%p] - [flags 0x%x] - '%s'\n", (void*)pCurr, pCurr->GetStateFlags(), pCurr->GetObject()->Name());
			pCurr = pCurr->Next();
		}
		AUDIO_SYS_FATAL("ObjectManager::ReceiveDectivate() [0x%x] [flags 0x%x]: instance '%s' is not in linked list", (U32)(INTPTR)pInstance, pInstance->GetStateFlags(), pInstance->GetObject()->Name());
	}
#endif

    if ((m_pFirst == pInstance) && (m_pLast == pInstance))
    {
        //  unique
        m_pFirst = NULL;
        m_pLast = NULL;
    }
    else if (m_pFirst == pInstance)
    {
        //  first
        m_pFirst = m_pFirst->Next();

        m_pFirst->SetPrev(NULL);
        pInstance->SetNext(NULL);
    }
    else if (m_pLast == pInstance)
    {
        //  last
        m_pLast = m_pLast->Prev();

        m_pLast->SetNext(NULL);
        pInstance->SetPrev(NULL);
    }
    else
    {
        //  rest
        pInstance->Prev()->SetNext(pInstance->Next());
        pInstance->Next()->SetPrev(pInstance->Prev());

        pInstance->SetPrev(NULL);
        pInstance->SetNext(NULL);
    }

	UnlinkActive(pInstance);

	pInstance->ResetStateFlag(InstanceActivated | InstanceDeactivationPending);
    pInstance->SetStateFlag(InstanceReleasePending);
	
	pInstance->GetObject()->DecrementInstanceCounter();

	#ifdef ENABLE_DEBUG_EXTENSIONS
	if (pInstance->IsSuspended())
		pInstance->GetObject()->DecrementSuspendedInstanceCounter();
	#endif
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveGetComponent
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveGetComponent(U32 u32Message, char* pchName, ObjectHandle hObject, ObjectComponentHandle hComponent)
{
	bool bFail = false;

	Object* pObject = GetObject(hObject);

	if (! pObject)
	{
		m_ObjectComponentHandles.SetResourceAllocationFailed(hComponent);
		bFail = true;
	}

	// extra debugging for bug #5568
	#ifndef CFG_GOLD
	if (!hComponent.IsValid())
	{
		AUDIO_SYS_WARN(DBSTR " Component '%s', pObject 0x%x, hComponent is not valid. [hObj 0x%x, hObjCom 0x%x]", pchName, (U32)(INTPTR)pObject, hObject.Value(), hComponent.Value());
		return;
	}	
	#endif

	U16 u16ComponentIndex = 0xffff;

	if (! bFail)
	{
		switch (u32Message)
		{
			case eGetObjectActionIn:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagActionIn);
			}
			break;

			case eGetObjectActionOut:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagActionOut);
			}
			break;

			case eGetObjectParameterIn:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagParameterIn);
			}
			break;

			case eGetObjectParameterOut:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagParameterOut);
			}
			break;

			case eGetObjectVectorIn:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagVectorIn);
			}
			break;

			case eGetObjectControlInput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagControl | Component::flagInputConnector | Component::flagControlConnector);
			}
			break;

			case eGetObjectEventInput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagEvent | Component::flagInputConnector | Component::flagEventConnector);
			}
			break;

			case eGetObjectMonoAudioInput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagMono | Component::flagInputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectStereoAudioInput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagStereo | Component::flagInputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectSurroundAudioInput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagSurround | Component::flagInputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectSurroundLFEAudioInput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagSurroundLFE | Component::flagInputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectControlOutput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagOutputConnector | Component::flagControlConnector);
			}
			break;

			case eGetObjectEventOutput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagOutputConnector | Component::flagEventConnector);
			}
			break;

			case eGetObjectMonoAudioOutput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagMono | Component::flagOutputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectStereoAudioOutput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagStereo | Component::flagOutputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectSurroundAudioOutput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagSurround | Component::flagOutputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectSurroundLFEAudioOutput:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, Component::flagSurroundLFE | Component::flagOutputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetComponentGeneric:
			{
				u16ComponentIndex = pObject->GetComponentIndex(pchName, 0);
			}
			break;			

			case eGetObjectVectorInput:
			case eGetObjectGraphInput:
			case eGetObjectVectorOutput:
			case eGetObjectGraphOutput:
			default:
			{
				m_ObjectComponentHandles.SetResourceAllocationFailed(hComponent);
				bFail = true;
			}
			break;
		}
	}

	if (u16ComponentIndex == 0xffff && (! bFail))
	{
		m_ObjectComponentHandles.SetResourceAllocationFailed(hComponent);
		bFail = true;
	}

	if (bFail)
	{
		AUDIO_SYS_WARN("FAIL: Get Component '%s' from object '%s' failed [hObj 0x%x, hObjCom 0x%x]", pchName, pObject ? pObject->Name() : "NULL", hObject.Value(), hComponent.Value());
		return;
	}

	U32 u32Resource = ObjectResource(hObject, u16ComponentIndex);

	m_ObjectComponentHandles.MapResourceToHandle(hComponent, u32Resource);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveGetComponent stringHash version
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveGetComponentFromHash(U32 u32Message, StringHash nameHash, ObjectHandle hObject, ObjectComponentHandle hComponent)
{
	bool bFail = false;

	Object* pObject = GetObject(hObject);

	if (! pObject)
	{
		m_ObjectComponentHandles.SetResourceAllocationFailed(hComponent);
		bFail = true;
	}

	#ifndef CFG_GOLD
	if (!hComponent.IsValid())
	{
		AUDIO_SYS_WARN(DBSTR " Component with hash 0x%x, pObject 0x%x, hComponent is not valid. [hObj 0x%x, hObjCom 0x%x]", nameHash.GetValue(), (U32)(INTPTR)pObject, hObject.Value(), hComponent.Value());
		return;
	}	
	#endif

	U16 u16ComponentIndex = 0xffff;

	if (! bFail)
	{
		switch (u32Message)
		{
			case eGetObjectActionInFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagActionIn);
			}
			break;

			case eGetObjectActionOutFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagActionOut);
			}
			break;

			case eGetObjectParameterInFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagParameterIn);
			}
			break;

			case eGetObjectParameterOutFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagParameterOut);
			}
			break;

			case eGetObjectVectorInFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagVectorIn);
			}
			break;

			case eGetObjectControlInputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagControl | Component::flagInputConnector | Component::flagControlConnector);
			}
			break;

			case eGetObjectEventInputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagEvent | Component::flagInputConnector | Component::flagEventConnector);
			}
			break;

			case eGetObjectMonoAudioInputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagMono | Component::flagInputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectStereoAudioInputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagStereo | Component::flagInputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectSurroundAudioInputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagSurround | Component::flagInputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectSurroundLFEAudioInputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagSurroundLFE | Component::flagInputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectControlOutputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagOutputConnector | Component::flagControlConnector);
			}
			break;

			case eGetObjectEventOutputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagOutputConnector | Component::flagEventConnector);
			}
			break;

			case eGetObjectMonoAudioOutputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagMono | Component::flagOutputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectStereoAudioOutputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagStereo | Component::flagOutputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectSurroundAudioOutputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagSurround | Component::flagOutputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetObjectSurroundLFEAudioOutputFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, Component::flagSurroundLFE | Component::flagOutputConnector | Component::flagAudioConnector);
			}
			break;

			case eGetComponentGenericFromHash:
			{
				u16ComponentIndex = pObject->GetComponentIndexFromHash(nameHash, 0);
			}
			break;			

			case eGetObjectVectorInputFromHash:
			case eGetObjectGraphInputFromHash:
			case eGetObjectVectorOutputFromHash:
			case eGetObjectGraphOutputFromHash:
			default:
			{
				m_ObjectComponentHandles.SetResourceAllocationFailed(hComponent);
				bFail = true;
			}
			break;
		}
	}

	if (u16ComponentIndex == 0xffff && (! bFail))
	{
		m_ObjectComponentHandles.SetResourceAllocationFailed(hComponent);
		bFail = true;
	}

	if (bFail)
	{
		AUDIO_SYS_WARN("FAIL: Get Component with hash 0x%x '%s' from object '%s' failed [hObj 0x%x, hObjCom 0x%x]", 
			nameHash.GetValue(), 
			Dictionary::Instance().GetString(nameHash.GetValue()).Chars(), 
			pObject ? pObject->Name() : "NULL", 
			hObject.Value(), 
			hComponent.Value());
		return;
	}

	U32 u32Resource = ObjectResource(hObject, u16ComponentIndex);

	m_ObjectComponentHandles.MapResourceToHandle(hComponent, u32Resource);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveReleaseObjectComponentHandle
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveReleaseObjectComponentHandle(ObjectComponentHandle hObjComp, bool bReserveHandle)
{
	m_ObjectComponentHandles.ReleaseHandle(hObjComp, bReserveHandle);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveSetValue
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveSetValue(InstanceComponentHandle hParamIn, F32 fValue)
{
	Instance* pInst = 0;
	Component* pComp = 0;

	if (! DereferenceInstanceComponentHandle(hParamIn, Component::flagParameterIn, &pInst, &pComp))
	{
		AUDIO_SYS_WARN("Receive Set Value [%s] '%s'.'%s' failed [hInst 0x%x, hObjCom 0x%x]", pInst ? pInst->EntityName() : "null", pInst ? pInst->GetObject()->Name() : "null", pComp ? pComp->Name() : "null", hParamIn.GetInstanceHandle().Value(), hParamIn.GetObjectComponentHandle().Value());
		return;
	}

	#ifndef CFG_SPEED
	if (!Maths::IsFinite(fValue))
	{
		AUDIO_SYS_WARN("Receive Set Value: value is NAN. [%s] '%s'.'%s' [hInst 0x%x, hObjCom 0x%x]", pInst ? pInst->EntityName() : "null", pInst->GetObject()->Name(), pComp->Name(), hParamIn.GetInstanceHandle().Value(), hParamIn.GetObjectComponentHandle().Value());
		return;
	}
	#endif

	((ParameterIn*)pComp)->SetValue(pInst, fValue);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveValue
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveValue(InstanceComponentHandle hParamOut, U32 hDataTracker, AudioRequestDataCallback pCallback)
{
	Instance* pInst = 0;
	Component* pComp = 0;
	GetValueQueueMessage msg;

	msg.pCallback = pCallback;
	msg.hDataTracker = hDataTracker;

	if (! DereferenceInstanceComponentHandle(hParamOut, Component::flagParameterOut, &pInst, &pComp))
	{
		AUDIO_SYS_WARN("Receive Get Value [%s] '%s'.'%s' failed [hInst 0x%x, hObjCom 0x%x]", pInst ? pInst->EntityName() : "null", pInst ? pInst->GetObject()->Name() : "null", pComp ? pComp->Name() : "null", hParamOut.GetInstanceHandle().Value(), hParamOut.GetObjectComponentHandle().Value());
		msg.bSuccess = false;
	}
	else
	{
		msg.f32Value = ((ParameterOut*)pComp)->Value(pInst);
		msg.bSuccess = true;
	}

	System::Get()->GetReturnQueue()->SendItem(Core::ReturnQueue::eGetValue, (void*)&msg, sizeof(GetValueQueueMessage));
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveExecute
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveExecute(InstanceComponentHandle hActionIn)
{
	Instance* pInst = 0;
	Component* pComp = 0;

	if (! DereferenceInstanceComponentHandle(hActionIn, Component::flagActionIn, &pInst, &pComp))
	{
		AUDIO_SYS_WARN("Receive Execute [%s] '%s'.'%s' failed [hInst 0x%x, hObjCom 0x%x]", pInst ? pInst->EntityName() : "null", pInst ? pInst->GetObject()->Name() : "null", pComp ? pComp->Name() : "null", hActionIn.GetInstanceHandle().Value(), hActionIn.GetObjectComponentHandle().Value());
		return;
	}

	((ActionIn*)pComp)->DoExecute(pInst);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveSetDimensions
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveSetDimensions(InstanceComponentHandle hVectorIn, F32* afDimensions)
{
	Instance* pInst = 0;
	Component* pComp = 0;

	if (! DereferenceInstanceComponentHandle(hVectorIn, Component::flagVectorIn, &pInst, &pComp))
	{
		AUDIO_SYS_WARN("Receive Set Dimensions [%s] '%s'.'%s' failed [hInst 0x%x, hObjCom 0x%x]", pInst ? pInst->EntityName() : "null", pInst ? pInst->GetObject()->Name() : "null", pComp ? pComp->Name() : "null", hVectorIn.GetInstanceHandle().Value(), hVectorIn.GetObjectComponentHandle().Value());
		return;
	}

	#ifndef CFG_SPEED
	if (!Maths::IsFinite(afDimensions[0]) || !Maths::IsFinite(afDimensions[1]) || !Maths::IsFinite(afDimensions[2]))
	{
		AUDIO_SYS_WARN("Receive Set Dimensions: a value is NAN. [%s] '%s'.'%s'", pInst ? pInst->EntityName() : "null", pInst ? pInst->GetObject()->Name() : "null", pComp ? pComp->Name() : "null");
		return;
	}
	#endif

	((VectorIn*)pComp)->SetDimensions(pInst, afDimensions);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveRegisterActionOutCallback
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveRegisterActionOutCallback(InstanceComponentHandle hActionOut, AudioActionOutCallback pCallback, U32 u32AppData)
{
	Instance* pInst = 0;
	Component* pComp = 0;

	if (! DereferenceInstanceComponentHandle(hActionOut, Component::flagActionOut, &pInst, &pComp))
	{
		AUDIO_SYS_WARN("Receive Register ActionOut Callback [%s] '%s'.'%s' failed [hInst 0x%x, hObjCom 0x%x]", pInst ? pInst->EntityName() : "null", pInst ? pInst->GetObject()->Name() : "null", pComp ? pComp->Name() : "null", hActionOut.GetInstanceHandle().Value(), hActionOut.GetObjectComponentHandle().Value());
		return;
	}

	((ActionOut*)pComp)->SetCallback(pCallback);
	((ActionOut*)pComp)->SetCallbackData(pInst, u32AppData);
}

////////////////////////////////////////////////////////////////////////////////
//	UnlinkActive
//		(remove the given instance from the active list)
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::UnlinkActive(Instance *pInstance)
{
	if (!pInstance->IsLinkedIntoActive())
		return;

	pInstance->LinkedIntoActive(false);

    if ((m_pFirstActive == pInstance) && (m_pLastActive == pInstance))
    {
        //  unique
        m_pFirstActive = NULL;
        m_pLastActive = NULL;
    }
    else if (m_pFirstActive == pInstance)
    {
        //  first
        m_pFirstActive = m_pFirstActive->NextActive();

        m_pFirstActive->SetPrevActive(NULL);
        pInstance->SetNextActive(NULL);
    }
    else if (m_pLastActive == pInstance)
    {
        //  last
        m_pLastActive = m_pLastActive->PrevActive();

        m_pLastActive->SetNextActive(NULL);
        pInstance->SetPrevActive(NULL);
    }
    else
    {
        //  rest
        pInstance->PrevActive()->SetNextActive(pInstance->NextActive());
        pInstance->NextActive()->SetPrevActive(pInstance->PrevActive());

        pInstance->SetPrevActive(NULL);
        pInstance->SetNextActive(NULL);
    }
}

/// add the given instance into the active list
void Audio::ObjectManager::LinkActive(Instance *pInstance)
{
	if (pInstance->IsLinkedIntoActive())
		return;

	pInstance->LinkedIntoActive(true);

	if ((m_pFirstActive == NULL) && (m_pLastActive == NULL))
        //  list empty
        m_pFirstActive = m_pLastActive = pInstance;
    else
    {
        //  add to end of list
        m_pLastActive->SetNextActive(pInstance);
        pInstance->SetPrevActive(m_pLastActive);
        m_pLastActive = pInstance;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveSuspend
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveSuspend(InstanceHandle hInstance)
{
	Instance* pInstance = GetInstance(hInstance);

	if (! pInstance)
	{
		AUDIO_SYS_WARN("Receive Suspend failed [hInst 0x%x]", hInstance.Value());
		return;
	}

	if (pInstance->IsSuspended())
		return;

	pInstance->GetObject()->IncrementSuspendedInstanceCounter();
//	m_u32NumInstancesActive--;

	#ifndef CFG_GOLD
	if (g_DebugLogSuspendAndResumingOfInstances)
	{
		AUDIO_SYS_DEBUG("Suspending instance '%s'", pInstance->GetObject()->Name());
	}
	#endif

	UnlinkActive(pInstance);
	
	pInstance->Suspend(true);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveResume
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveResume(InstanceHandle hInstance)
{
	Instance* pInstance = GetInstance(hInstance);

	if (! pInstance)
	{
		AUDIO_SYS_WARN("Receive Resume failed [hInst 0x%x]", hInstance.Value());
		return;
	}

	if (!pInstance->IsSuspended())
		return;

	pInstance->GetObject()->DecrementSuspendedInstanceCounter();
//	m_u32NumInstancesActive++;

	#ifndef CFG_GOLD
	if (g_DebugLogSuspendAndResumingOfInstances)
	{
		AUDIO_SYS_DEBUG("Resuming instance '%s'", pInstance->GetObject()->Name());
	}
	#endif

	LinkActive(pInstance);
	
	pInstance->Suspend(false);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveSuspend
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveMute(InstanceHandle hInstance)
{
	Instance* pInstance = GetInstance(hInstance);

	if (! pInstance)
	{
		AUDIO_SYS_WARN("Receive Mute failed [hInst 0x%x]", hInstance.Value());
		return;
	}

	pInstance->GetObject()->IncrementMutedInstanceCounter();

	#ifndef CFG_GOLD
	if (g_DebugLogMutingAndUnmutingOfInstances)
	{
		AUDIO_SYS_DEBUG("Muting instance '%s'", pInstance->GetObject()->Name());
	}
	#endif

	UnlinkActive(pInstance);

	if (! pInstance->IsMuted())
		m_u32NumInstancesMuted++;

	pInstance->Mute(true);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveResume
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveUnmute(InstanceHandle hInstance)
{
	Instance* pInstance = GetInstance(hInstance);

	if (! pInstance)
	{
		AUDIO_SYS_WARN("Receive Unmute failed [hInst 0x%x]", hInstance.Value());
		return;
	}

	pInstance->GetObject()->DecrementMutedInstanceCounter();
	
	#ifndef CFG_GOLD
	if (g_DebugLogMutingAndUnmutingOfInstances)
	{
		AUDIO_SYS_DEBUG("Unmuting instance '%s'", pInstance->GetObject()->Name());
	}
	#endif

	LinkActive(pInstance);
	
	if (pInstance->IsMuted())
		m_u32NumInstancesMuted--;

	pInstance->Mute(false);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveConnectControl
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveConnectControl(InstanceComponentHandle hControlOut, InstanceComponentHandle hControlIn)
{
	Instance* pInstOut = 0;
	Component* pCompOut = 0;
	Instance* pInstIn = 0;
	Component* pCompIn = 0;

	bool bOutputFailed = false;
	bool bInputFailed = false;

	if (! DereferenceInstanceComponentHandle(hControlOut, Component::flagControl | Component::flagOutputConnector, &pInstOut, &pCompOut))
		bOutputFailed = true;

	if (! DereferenceInstanceComponentHandle(hControlIn, Component::flagControl | Component::flagInputConnector, &pInstIn, &pCompIn))
		bInputFailed = true;

	if (bInputFailed || bOutputFailed)
	{
		AUDIO_SYS_WARN("Receive Connect Control failed - ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s' : invalid %s %s component/ instance.", 
			pInstOut ? pInstOut->EntityName() : "none",
			pInstOut ? pInstOut->GetObject()->Name() : "null", 
			pCompOut ? pCompOut->Name() : "null", 
			pInstIn ? pInstIn->EntityName() : "none",
			pInstIn ? pInstIn->GetObject()->Name() : "null", 
			pCompIn ? pCompIn->Name() : "null",
			bInputFailed ? "Input" : "",
			bOutputFailed ? "Output" : "");
		return;
	}

	//  connect
	ObjectComponentHandle hObjOut = ExtractObjectComponentHandle(hControlOut);
	U32 u32ObjOut = GetResourceFromObjectComponentHandle(hObjOut);
	U16 uComOut = ExtractComponentIndex(u32ObjOut);

	ObjectComponentHandle hObjIn = ExtractObjectComponentHandle(hControlIn);
	U32 u32ObjIn = GetResourceFromObjectComponentHandle(hObjIn);
	U16 uComIn = ExtractComponentIndex(u32ObjIn);

    ((ControlInput*)pCompIn)->SetOutput(pInstIn, pInstOut, uComOut);
    ((ControlOutput*)pCompOut)->SetInput(pInstOut, pInstIn, uComIn);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveConnectEvent
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveConnectEvent(InstanceComponentHandle hEventOut, InstanceComponentHandle hEventIn)
{
	Instance* pInstOut = 0;
	Component* pCompOut = 0;
	Instance* pInstIn = 0;
	Component* pCompIn = 0;

	bool bOutputFailed = false;
	bool bInputFailed = false;

	if (! DereferenceInstanceComponentHandle(hEventOut, Component::flagControl | Component::flagOutputConnector, &pInstOut, &pCompOut))
		bOutputFailed = true;

	if (! DereferenceInstanceComponentHandle(hEventIn, Component::flagControl | Component::flagInputConnector, &pInstIn, &pCompIn))
		bInputFailed = true;

	if (bInputFailed || bOutputFailed)
	{
		AUDIO_SYS_WARN("Receive Connect Event failed- ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s' : invalid %s %s component/ instance.", 
			pInstOut ? pInstOut->EntityName() : "none",
			pInstOut ? pInstOut->GetObject()->Name() : "null", 
			pCompOut ? pCompOut->Name() : "null", 
			pInstIn ? pInstIn->EntityName() : "none",
			pInstIn ? pInstIn->GetObject()->Name() : "null", 
			pCompIn ? pCompIn->Name() : "null",
			bInputFailed ? "Input" : "",
			bOutputFailed ? "Output" : "");
		return;
	}

    //  connect
	ObjectComponentHandle hObjIn = ExtractObjectComponentHandle(hEventIn);
	U32 u32ObjIn = GetResourceFromObjectComponentHandle(hObjIn);
	U16 uComIn = ExtractComponentIndex(u32ObjIn);

    ((EventOutput*)pCompOut)->SetInput(pInstOut, pInstIn, uComIn);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveConnectGeneric
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveConnectGeneric(InstanceComponentHandle hOut, InstanceComponentHandle hIn)
{
	Instance* pInstOut = 0;
	Component* pCompOut = 0;
	Instance* pInstIn = 0;
	Component* pCompIn = 0;

	bool bOutputFailed = false;
	bool bInputFailed = false;

	if (! DereferenceInstanceComponentHandle(hOut, Component::flagOutputConnector, &pInstOut, &pCompOut))
	{
		bOutputFailed = true;
	}

	if (! DereferenceInstanceComponentHandle(hIn, Component::flagInputConnector, &pInstIn, &pCompIn))
	{
		bInputFailed = true;
	}

	if (bInputFailed || bOutputFailed)
	{
		AUDIO_SYS_WARN("Receive Connect Generic failed- ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s' : invalid %s %s component/ instance. [hInst 0x%x to hInst 0x%x]", 
			pInstOut ? pInstOut->EntityName() : "none",
			pInstOut ? pInstOut->GetObject()->Name() : "null", 
			pCompOut ? pCompOut->Name() : "null", 
			pInstIn ? pInstIn->EntityName() : "none",
			pInstIn ? pInstIn->GetObject()->Name() : "null", 
			pCompIn ? pCompIn->Name() : "null",
			bInputFailed ? "Input" : "",
			bOutputFailed ? "Output" : "",
			hOut.GetInstanceHandle().Value(),
			hIn.GetInstanceHandle().Value());
		return;
	}

	switch (pCompOut->TypeId())
	{
		case Component::eMonoAudioOutput:
		{
			AUDIO_SYS_DEBUG("ReceiveConnectGeneric() : Mono :['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eMonoAudioInput)
				AUDIO_SYS_WARN("Mono audio output not matched in ReceiveConnectGeneric");
			else
				((MonoAudioOutput*)pCompOut)->OutputTo(pInstOut, ((MonoAudioInput*)pCompIn)->AudioInput(pInstIn));

			return;
		}

		case Component::eStereoAudioOutput:
		{
			AUDIO_SYS_DEBUG("ReceiveConnectGeneric() : Stereo :['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eStereoAudioInput)
				AUDIO_SYS_WARN("Stereo audio output not matched in ReceiveConnectGeneric");
			else
				((StereoAudioOutput*)pCompOut)->OutputTo(pInstOut, ((StereoAudioInput*)pCompIn)->AudioInput(pInstIn));

			return;
		}

		case Component::eSurroundAudioOutput:
		{
			AUDIO_SYS_DEBUG("ReceiveConnectGeneric() : Surround : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eSurroundAudioInput)
				AUDIO_SYS_WARN("Surround audio output not matched in ReceiveConnectGeneric");
			else
				((SurroundAudioOutput*)pCompOut)->OutputTo(pInstOut, ((SurroundAudioInput*)pCompIn)->AudioInput(pInstIn));

			return;
		}

		case Component::eSurroundLFEAudioOutput:
		{
			AUDIO_SYS_DEBUG("ReceiveConnectGeneric() : SurroundLFE : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eSurroundLFEAudioInput)
				AUDIO_SYS_WARN("SurroundLFE audio output not matched in ReceiveConnectGeneric");
			else
				((SurroundLFEAudioOutput*)pCompOut)->OutputTo(pInstOut, ((SurroundLFEAudioInput*)pCompIn)->AudioInput(pInstIn));

			return;
		}

		case Component::eEventOutput:
		{
			AUDIO_SYS_DEBUG("ReceiveConnectGeneric() : Event : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eEventInput)
				AUDIO_SYS_WARN("Event output not matched in ReceiveConnectGeneric");
			else
			{
				ObjectComponentHandle hObjIn = ExtractObjectComponentHandle(hIn);
				U32 u32ObjIn = GetResourceFromObjectComponentHandle(hObjIn);
				U16 uComIn = ExtractComponentIndex(u32ObjIn);

			    ((EventOutput*)pCompOut)->SetInput(pInstOut, pInstIn, uComIn);
			}

			return;
		}

		case Component::eControlOutput:
		{
			AUDIO_SYS_DEBUG("ReceiveConnectGeneric() : Control : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eControlInput)
				AUDIO_SYS_WARN("Control output not matched in ReceiveConnectGeneric");
			else
			{
				ObjectComponentHandle hObjOut = ExtractObjectComponentHandle(hOut);
				U32 u32ObjOut = GetResourceFromObjectComponentHandle(hObjOut);
				U16 uComOut = ExtractComponentIndex(u32ObjOut);

				ObjectComponentHandle hObjIn = ExtractObjectComponentHandle(hIn);
				U32 u32ObjIn = GetResourceFromObjectComponentHandle(hObjIn);
				U16 uComIn = ExtractComponentIndex(u32ObjIn);

				((ControlInput*)pCompIn)->SetOutput(pInstIn, pInstOut, uComOut);
				((ControlOutput*)pCompOut)->SetInput(pInstOut, pInstIn, uComIn);
			}

			return;
		}

		case Component::eCoordinateOutput:
		{
			AUDIO_SYS_DEBUG("ReceiveConnectGeneric() : Coordinate : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompOut->TypeId() != Component::eCoordinateOutput)
				AUDIO_SYS_WARN("Coordinate output not matched in ReceiveConnectGeneric");
			else
			{
				ObjectComponentHandle hObjOut = ExtractObjectComponentHandle(hOut);
				U32 u32ObjOut = GetResourceFromObjectComponentHandle(hObjOut);
				U16 uComOut = ExtractComponentIndex(u32ObjOut);

				ObjectComponentHandle hObjIn = ExtractObjectComponentHandle(hIn);
				U32 u32ObjIn = GetResourceFromObjectComponentHandle(hObjIn);
				U16 uComIn = ExtractComponentIndex(u32ObjIn);

				((CoordinateInput*)pCompIn)->SetOutput(pInstIn, pInstOut, uComOut);
				((CoordinateOutput*)pCompOut)->SetInput(pInstOut, pInstIn, uComIn);
			}

			return;
		}
	}

	U32 u32OutputFlags = pCompOut->Flags();
	U32 u32InputFlags = pCompIn->Flags();

	if ((u32OutputFlags & Component::flagEventConnector) && (u32OutputFlags & Component::flagOutputConnector)
		&& (u32InputFlags & Component::flagEventConnector) && (u32InputFlags & Component::flagInputConnector))
		ReceiveConnectEvent(hOut, hIn);
	else if ((u32OutputFlags & Component::flagControlConnector) && (u32OutputFlags & Component::flagOutputConnector)
		&& (u32InputFlags & Component::flagControlConnector) && (u32InputFlags & Component::flagInputConnector))
		ReceiveConnectControl(hOut, hIn);
	else
		AUDIO_SYS_WARN("Connector types don't match in ReceiveConnectGeneric");
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveDisconnectGeneric
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveDisconnectGeneric(InstanceComponentHandle hOut, InstanceComponentHandle hIn)
{
	Instance* pInstOut = 0;
	Component* pCompOut = 0;
	Instance* pInstIn = 0;
	Component* pCompIn = 0;

	bool bOutputFailed = false;
	bool bInputFailed = false;

	if (! DereferenceInstanceComponentHandle(hOut, Component::flagOutputConnector, &pInstOut, &pCompOut))
	{
		if (! DereferenceDoomedInstanceComponentHandle(hOut, Component::flagOutputConnector, &pInstOut, &pCompOut))
		{
			bOutputFailed = true;
		}
	}

	if (! DereferenceInstanceComponentHandle(hIn, Component::flagInputConnector, &pInstIn, &pCompIn))
	{
		if (! DereferenceDoomedInstanceComponentHandle(hIn, Component::flagInputConnector, &pInstIn, &pCompIn))
		{
			bInputFailed = true;
		}
	}

	if (bInputFailed || bOutputFailed)
	{
		AUDIO_SYS_WARN("Receive Disconnect Generic failed- ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s' : invalid %s %s component/ instance. [hInst 0x%x to hInst 0x%x]", 
			pInstOut ? pInstOut->EntityName() : "none",
			pInstOut ? pInstOut->GetObject()->Name() : "null", 
			pCompOut ? pCompOut->Name() : "null", 
			pInstIn ? pInstIn->EntityName() : "none",
			pInstIn ? pInstIn->GetObject()->Name() : "null", 
			pCompIn ? pCompIn->Name() : "null",
			bInputFailed ? "Input" : "",
			bOutputFailed ? "Output" : "",
			hOut.GetInstanceHandle().Value(),
			hIn.GetInstanceHandle().Value());
		return;
	}

	switch (pCompOut->TypeId())
	{
		case Component::eMonoAudioOutput:
		{
			AUDIO_SYS_DEBUG("ObjectManager::ReceiveConnectGeneric() : Mono : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eMonoAudioInput)
				AUDIO_SYS_WARN("Mono audio output output not matched in ReceiveDisconnectGeneric");
			else
				((MonoAudioOutput*)pCompOut)->DisconnectFrom(pInstOut, ((MonoAudioInput*)pCompIn)->AudioInput(pInstIn));

			return;
		}

		case Component::eStereoAudioOutput:
		{
			AUDIO_SYS_DEBUG("ObjectManager::ReceiveConnectGeneric() : Stereo : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eStereoAudioInput)
				AUDIO_SYS_WARN("Stereo output not matched in ReceiveDisconnectGeneric");
			else
				((StereoAudioOutput*)pCompOut)->DisconnectFrom(pInstOut, ((StereoAudioInput*)pCompIn)->AudioInput(pInstIn));

			return;
		}

		case Component::eSurroundAudioOutput:
		{
			AUDIO_SYS_DEBUG("ObjectManager::ReceiveConnectGeneric() : Surround : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eSurroundAudioInput)
				AUDIO_SYS_WARN("Surround output not matched in ReceiveDisconnectGeneric");
			else
				((SurroundAudioOutput*)pCompOut)->DisconnectFrom(pInstOut, ((SurroundAudioInput*)pCompIn)->AudioInput(pInstIn));

			return;
		}

		case Component::eSurroundLFEAudioOutput:
		{
			AUDIO_SYS_DEBUG("ObjectManager::ReceiveConnectGeneric() : SurroundLFE : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eSurroundLFEAudioInput)
				AUDIO_SYS_WARN("SurroundLFE output not matched in ReceiveDisconnectGeneric");
			else
				((SurroundLFEAudioOutput*)pCompOut)->DisconnectFrom(pInstOut, ((SurroundLFEAudioInput*)pCompIn)->AudioInput(pInstIn));

			return;
		}

		case Component::eEventOutput:
		{
			AUDIO_SYS_DEBUG("ObjectManager::ReceiveConnectGeneric() : Event : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eEventInput || pCompOut->TypeId() != Component::eEventOutput)
				AUDIO_SYS_WARN("Event output not matched in ReceiveDisconnectGeneric");
			else
			{
				EventOutput* pEvntOut = (EventOutput*)pCompOut;
				((EventOutput*)pCompOut)->SetInput(pInstOut, NULL, -1);
				((EventInput*)pCompIn)->SetOutput(pInstIn, NULL, -1);
			}

			return;
		}

		case Component::eControlOutput:
		{
			AUDIO_SYS_DEBUG("ObjectManager::ReceiveConnectGeneric() : Control : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eControlInput || pCompOut->TypeId() != Component::eControlOutput)
				AUDIO_SYS_WARN("Control output not matched in ReceiveDisconnectGeneric");
			else
			{
				ObjectComponentHandle hObjOut = ExtractObjectComponentHandle(hOut);
				U32 u32ObjOut = GetResourceFromObjectComponentHandle(hObjOut);
				U16 uComOut = ExtractComponentIndex(u32ObjOut);

				ObjectComponentHandle hObjIn = ExtractObjectComponentHandle(hIn);
				U32 u32ObjIn = GetResourceFromObjectComponentHandle(hObjIn);
				U16 uComIn = ExtractComponentIndex(u32ObjIn);

				//  disconnect
				((ControlOutput*)pCompOut)->SetInput(pInstOut, NULL, -1);
				((ControlInput*)pCompIn)->SetOutput(pInstIn, NULL, -1);
			}

			return;
		}

		case Component::eCoordinateOutput:
		{
			AUDIO_SYS_DEBUG("ObjectManager::ReceiveConnectGeneric() : Coordinate : ['%s'] '%s'.'%s' to ['%s'] '%s'.'%s'", pInstOut->EntityName(), pInstOut->GetObject()->Name(), pCompOut->Name(), pInstIn->EntityName(), pInstIn->GetObject()->Name(), pCompIn->Name());

			if (pCompIn->TypeId() != Component::eCoordinateInput || pCompOut->TypeId() != Component::eCoordinateOutput)
				AUDIO_SYS_WARN("Coordinate output not matched in ReceiveDisconnectGeneric");
			else
			{
				ObjectComponentHandle hObjOut = ExtractObjectComponentHandle(hOut);
				U32 u32ObjOut = GetResourceFromObjectComponentHandle(hObjOut);
				U16 uComOut = ExtractComponentIndex(u32ObjOut);

				ObjectComponentHandle hObjIn = ExtractObjectComponentHandle(hIn);
				U32 u32ObjIn = GetResourceFromObjectComponentHandle(hObjIn);
				U16 uComIn = ExtractComponentIndex(u32ObjIn);

				//  disconnect
				((CoordinateOutput*)pCompOut)->SetInput(pInstOut, NULL, -1);
				((CoordinateInput*)pCompIn)->SetOutput(pInstIn, NULL, -1);
			}

			return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	GetPath
////////////////////////////////////////////////////////////////////////////////
Util::C_String Audio::ObjectManager::GetPath()
{ 
	const char pchBuf[256] = {0};

	Util::C_String str;

	if (System::Get()->GetDatPathCallback()) 
	{
		System::Get()->GetDatPathCallback()(pchBuf, 255);

		str = pchBuf;
 
		// ensure trailing slash
		if (str.Length() > 0)
		{
			char c = str[str.Length()-1];
			if (c != '\\' && c != '/')
				str += '\\';
		}
	} 

	return str;
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveCreate
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveCreate(ObjectHandle hObject, char* pchFilename, IWaveManager* pWaveManager)
{
	if( m_ObjectHandles.IsHandleValid( hObject ) )
	{
		if( !m_ObjectHandles.IsReleasePending( hObject ) )
		{
			//	load definition or return previously loaded
			Util::C_String strFullPath = GetPath() + pchFilename;
			Definition* pDef = System::Get()->GetDefinitionManager()->Create(strFullPath.Chars());

			if( pDef )
			{
				if( pDef->GetObject() )
				{
					//	already created one, just map handle
					m_ObjectHandles.MapResourceToHandle(hObject, (HandleResource)pDef->GetObject());
				}
				else
				{
					Object* pObject = InternalCreate(pDef->FrameSkip());

					if( pObject )
					{
						AUDIO_SYS_DEBUG("Initializing Object '%s'. [hObj 0x%x]", pDef->Name(), hObject.Value());

						if( Audio::Success == pObject->Init(pDef, pWaveManager) )
						{
							m_ObjectHandles.MapResourceToHandle(hObject, (HandleResource)pObject);
						}
						else
						{
							AUDIO_SYS_ERROR("Create object '%s' failed (init failed) [hObj 0x%x]", pchFilename, hObject.Value());
							
							System::Get()->GetDefinitionManager()->Release(pDef);

							m_ObjectHandles.SetResourceAllocationFailed(hObject);
							delete pObject;
							pObject = NULL;
						}
					}
					else
					{
						AUDIO_SYS_ERROR("Create object '%s' failed (internal create) [hObj 0x%x]", pchFilename, hObject.Value());
						
						System::Get()->GetDefinitionManager()->Release(pDef);

						m_ObjectHandles.SetResourceAllocationFailed(hObject);
					}
				}
			}
			else
			{
				m_ObjectHandles.SetResourceAllocationFailed(hObject);
				AUDIO_SYS_ERROR("Create object '%s' failed (definition failed to load) [hObj 0x%x]", strFullPath.Chars(), hObject.Value());
			}
		}
		else
		{
			AUDIO_SYS_ERROR("Create object '%s' failed (release pending)", pchFilename);
		}
	}
	else
	{
		AUDIO_SYS_ERROR("Create object '%s' failed (invalid object handle passed in)", pchFilename);
	}

//	if (! m_ObjectHandles.IsHandleValid(hObject))
//	{
//		AUDIO_SYS_ERROR("Create object '%s' failed (invalid object handle passed in)", pchFilename);
//		return;
//	}
//
//	if (m_ObjectHandles.IsReleasePending(hObject))
//		return;
//
//	Util::C_String strFullPath = GetPath() + pchFilename;
//	
//	Definition* iDef = System::Get()->GetDefinitionManager()->Create(strFullPath.Chars());
//
//	if (! iDef)
//	{
//		AUDIO_SYS_ERROR("Create object '%s' failed (definition failed to load) [hObj 0x%x]", strFullPath.Chars(), hObject.Value());
//		m_ObjectHandles.SetResourceAllocationFailed(hObject);
//		return;
//	}
//
//	bool bAllOk = true;
//
//	Object* pObject = iDef->GetObject();
//	
//	if (! pObject)
//	{
//		pObject = InternalCreate(iDef->FrameSkip());
//
//		if (pObject)
//		{
//			AUDIO_SYS_DEBUG("Initializing Object '%s'. [hObj 0x%x]", iDef->Name(), hObject.Value());
//			bAllOk = (Audio::Success == pObject->Init(iDef, pWaveManager, pchEntityName));
//		}
//	}
//
//	if (pObject && bAllOk)
//		pObject->SetHandle(hObject);
//	else
//		bAllOk = false;
//
//	if (bAllOk)
//	{
//		if (! m_ObjectHandles.MapResourceToHandle(hObject, (S32)pObject))
//			bAllOk = false;
//	}
//
//	if (! bAllOk)
//	{
//		if (pObject)
//			delete pObject;
//
//		
//
//		AUDIO_SYS_ERROR("Create object '%s' failed (setup failed) [hObj 0x%x]", pchFilename, hObject.Value());
//		iDef->DecRefCount();
//		System::Get()->GetDefinitionManager()->RemoveFromList(iDef);
//		System::Get()->GetDefinitionManager()->Destroy(iDef);
//		m_ObjectHandles.SetResourceAllocationFailed(hObject);
//		return;
//	}
//
//    AUDIO_SYS_INFO("Created Object '%s'. [part of entity '%s']", pchFilename, pchEntityName);
//
//	iDef->SetObject(pObject);
//
//#if defined (AUDIO_ENABLE_PROFILING)
//	Profiler::Instance().ResetInfo();
//#endif
}

////////////////////////////////////////////////////////////////////////////////
//	DestroyImmediate
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::DestroyImmediate(ObjectHandle hObject, bool bReserveHandle)
{
	if (!m_ObjectHandles.IsHandleValid(hObject))
		return;

	// lock here?
	Object* pObject = GetDoomedObject(hObject);

	if (!pObject)
	{
		AUDIO_SYS_ERROR(DBSTR "GetDoomedObject returned NULL! [hObj 0x%x]", hObject.Value());
		return;
	}

	Definition* pDef = pObject->GetDefinition();

	if (!pDef)
	{
		AUDIO_SYS_ERROR(DBSTR "Destroying object when definition does not exist! [hObj 0x%x]", hObject.Value());
		return;
	}

	
	AUDIO_SYS_DEBUG("Received destroy on object %s, ref count is %d\n", pObject->Name(), pDef->RefCount());

	// if definition ref count is 1 then this object/definition is ready to be destroyed
	if (pDef->RefCount() == 1)
	{
		// should this be fatal..?
		if (pObject->NumInstances())
		{
			AUDIO_SYS_ERROR("Destroying object %s with active instances! [hObj 0x%x]", pObject->Name(), hObject.Value());
			return;
		}

		#ifndef CFG_SPEED
		// final sanity check
		for (Instance* pInst = m_pFirst; pInst; pInst = pInst->Next())
		{
			if (pInst->GetObject() == pObject || pInst->GetObject()->GetDefinition() == pDef)
			{
				AUDIO_SYS_FATAL("About to destroy object and definition that an instance is still referring to!");
				DebugBreak();
			}
		}
		#endif

		// profiler stuff
		#if defined (AUDIO_ENABLE_PROFILING)
		GetLock()->Acquire();
		for (int i=0; i<m_Objects.Size(); i++)
		{
			if (m_Objects[i] == pObject)
			{
				m_Objects.EraseByIndex(i);
				break;
			}
		}
		GetLock()->Release();
		#endif

		// release the objectno
		pObject->Release();
		delete pObject;
	}

	// release the definition/decrement ref count. 
	// will only actually release definition when ref count is 0
	System::Get()->GetDefinitionManager()->Release(pDef);


	if (m_ObjectHandles.IsReleasePending(hObject))
	{
		m_ObjectHandles.ReleaseHandle(hObject);

		if (bReserveHandle)
			m_ObjectHandles.ReserveHandle(hObject);
	}
	else
		m_ObjectHandles.SetResourceAllocationFailed(hObject);

	#if defined (AUDIO_ENABLE_PROFILING)
	// is this safe to call on the main thread?
	Profiler::Instance().ResetInfo();
	#endif

    return;
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveDestroy
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveDestroy(ObjectHandle hObject, bool bReserveHandle)
{
	if (!m_ObjectHandles.IsReleasePending(hObject))
	{
		AUDIO_SYS_ERROR("ObjectManager::ReceiveDestroy() object is not in a release pending state.");
		return;
	}

	// if bReserveHandle is true, the object is being destroyed as part of ReloadAllAudio, and we want to destroy it on the audio thread
	if (! bReserveHandle)
	{
		// send a message back to say it is now safe to destroy the object
		DestroyObjectQueueMessage msg;
		msg.hObject = hObject;
		msg.bReserveHandle = bReserveHandle;
		System::Get()->GetReturnQueue()->SendItem(Core::ReturnQueue::eDestroyObject, (void*)&msg, sizeof(DestroyObjectQueueMessage));
	}
	else
		DestroyImmediate(hObject, bReserveHandle);
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveCreateInstance
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveCreateInstance(ObjectHandle hObject, InstanceHandle hInstance, const char* pchGroupName, const char* pchEntityName)
{
	Object* pObj = GetObject(hObject);

	if (pObj)
	{
		AUDIO_SYS_DEBUG("*** Creating Instance of '%s'. [Part of entity '%s'] [hInst 0x%x. hObject 0x%x]", pObj->Name(), pchEntityName ? pchEntityName : "null", hInstance.Value(), hObject.Value());

		Instance* pInst = pObj->CreateInstance(pchGroupName, pchEntityName);
		
		if (pInst)
		{
			m_InstanceHandles.MapResourceToHandle(hInstance, (HandleResource)pInst);
			Activate(pInst);
			pInst->SetHandle(hInstance);
			m_u32NumTotalInstances++;

#if defined (AUDIO_ENABLE_PROFILING)
			Profiler::Instance().ResetInfo();
#endif
			return;
		}
	}

	// if we get here, something's failed
	{
		AUDIO_SYS_ERROR("Instance creation of '%s' failed. [hInst 0x%x. hObject 0x%x]", pObj ? pObj->Name() : "null object", hInstance.Value(), hObject.Value());
		m_InstanceHandles.SetResourceAllocationFailed(hInstance);
	}
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveDestroyInstance
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::ReceiveDestroyInstance(InstanceHandle hInstance, bool bReserveHandle)
{
	Instance* pInst = GetDoomedInstance(hInstance);

	if (pInst)
	{
		Object* pObj = pInst->GetObject();

		if (pObj)
		{
			Deactivate(pInst);
			AUDIO_SYS_DEBUG("Object::DestroyInstance: '%s' [hInst 0x%x]", pObj->Name(), hInstance.Value());
			if (pInst->IsSuspended())
				pObj->DecrementSuspendedInstanceCounter();
			pObj->DestroyInstance(pInst);
			m_u32NumTotalInstances--;
			m_InstanceHandles.ReleaseHandle(hInstance, bReserveHandle);

#if defined (AUDIO_ENABLE_PROFILING)
			Profiler::Instance().ResetInfo();
#endif

			return;
		}
	}

	AUDIO_SYS_ERROR("Receive Destroy Instance failed. [hInst 0x%x]", hInstance.Value());
}


Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectActionIn(ObjectHandle hObject, const char* pchName)					{ return GetObjectComponentHandle(hObject, eGetObjectActionIn, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectActionOut(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetObjectActionOut, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectParameterIn(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetObjectParameterIn, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectParameterOut(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetObjectParameterOut, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectVectorIn(ObjectHandle hObject, const char* pchName)					{ return GetObjectComponentHandle(hObject, eGetObjectVectorIn, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectControlInput(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetObjectControlInput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectEventInput(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetObjectEventInput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectVectorInput(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetObjectVectorInput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectGraphInput(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetObjectGraphInput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectMonoAudioInput(ObjectHandle hObject, const char* pchName)			{ return GetObjectComponentHandle(hObject, eGetObjectMonoAudioInput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectStereoAudioInput(ObjectHandle hObject, const char* pchName)			{ return GetObjectComponentHandle(hObject, eGetObjectStereoAudioInput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectSurroundAudioInput(ObjectHandle hObject, const char* pchName)		{ return GetObjectComponentHandle(hObject, eGetObjectSurroundAudioInput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectSurroundLFEAudioInput(ObjectHandle hObject, const char* pchName)	{ return GetObjectComponentHandle(hObject, eGetObjectSurroundLFEAudioInput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectControlOutput(ObjectHandle hObject, const char* pchName)			{ return GetObjectComponentHandle(hObject, eGetObjectControlOutput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectEventOutput(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetObjectEventOutput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectVectorOutput(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetObjectVectorOutput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectGraphOutput(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetObjectGraphOutput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectMonoAudioOutput(ObjectHandle hObject, const char* pchName)			{ return GetObjectComponentHandle(hObject, eGetObjectMonoAudioOutput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectStereoAudioOutput(ObjectHandle hObject, const char* pchName)		{ return GetObjectComponentHandle(hObject, eGetObjectStereoAudioOutput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectSurroundAudioOutput(ObjectHandle hObject, const char* pchName)		{ return GetObjectComponentHandle(hObject, eGetObjectSurroundAudioOutput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectSurroundLFEAudioOutput(ObjectHandle hObject, const char* pchName)	{ return GetObjectComponentHandle(hObject, eGetObjectSurroundLFEAudioOutput, pchName); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectComponent(ObjectHandle hObject, const char* pchName)				{ return GetObjectComponentHandle(hObject, eGetComponentGeneric, pchName); }
void Audio::ObjectManager::SetObjectFrameSkip(ObjectHandle hObject, U32 frameSkip)												{ GetObject(hObject)->SetFrameSkip(frameSkip); }

Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectActionInFromHash(ObjectHandle hObject, StringHash nameHash)					{ return GetObjectComponentHandleFromHash(hObject, eGetObjectActionInFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectActionOutFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetObjectActionOutFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectParameterInFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetObjectParameterInFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectParameterOutFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetObjectParameterOutFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectVectorInFromHash(ObjectHandle hObject, StringHash nameHash)					{ return GetObjectComponentHandleFromHash(hObject, eGetObjectVectorInFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectControlInputFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetObjectControlInputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectEventInputFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetObjectEventInputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectVectorInputFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetObjectVectorInputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectGraphInputFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetObjectGraphInputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectMonoAudioInputFromHash(ObjectHandle hObject, StringHash nameHash)			{ return GetObjectComponentHandleFromHash(hObject, eGetObjectMonoAudioInputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectStereoAudioInputFromHash(ObjectHandle hObject, StringHash nameHash)			{ return GetObjectComponentHandleFromHash(hObject, eGetObjectStereoAudioInputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectSurroundAudioInputFromHash(ObjectHandle hObject, StringHash nameHash)		{ return GetObjectComponentHandleFromHash(hObject, eGetObjectSurroundAudioInputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectSurroundLFEAudioInputFromHash(ObjectHandle hObject, StringHash nameHash)	{ return GetObjectComponentHandleFromHash(hObject, eGetObjectSurroundLFEAudioInputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectControlOutputFromHash(ObjectHandle hObject, StringHash nameHash)			{ return GetObjectComponentHandleFromHash(hObject, eGetObjectControlOutputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectEventOutputFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetObjectEventOutputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectVectorOutputFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetObjectVectorOutputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectGraphOutputFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetObjectGraphOutputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectMonoAudioOutputFromHash(ObjectHandle hObject, StringHash nameHash)			{ return GetObjectComponentHandleFromHash(hObject, eGetObjectMonoAudioOutputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectStereoAudioOutputFromHash(ObjectHandle hObject, StringHash nameHash)		{ return GetObjectComponentHandleFromHash(hObject, eGetObjectStereoAudioOutputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectSurroundAudioOutputFromHash(ObjectHandle hObject, StringHash nameHash)		{ return GetObjectComponentHandleFromHash(hObject, eGetObjectSurroundAudioOutputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectSurroundLFEAudioOutputFromHash(ObjectHandle hObject, StringHash nameHash)	{ return GetObjectComponentHandleFromHash(hObject, eGetObjectSurroundLFEAudioOutputFromHash, nameHash); }
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectComponentFromHash(ObjectHandle hObject, StringHash nameHash)				{ return GetObjectComponentHandleFromHash(hObject, eGetComponentGenericFromHash, nameHash); }

////////////////////////////////////////////////////////////////////////////////
//	ReuseObjectComponent
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::ReuseObjectComponent(ObjectHandle hObject, ObjectComponentHandle hObjComp, const char* pchName)
{
	U8 au8PacketBuffer[512] = {0};
	U32 uNameLen = strlen(pchName) + 1;

	U8* pBuffer = au8PacketBuffer;

	((GetComponentPacket*)pBuffer)->packet.uMessage = eGetComponentGeneric;
	((GetComponentPacket*)pBuffer)->hObject = hObject;
	((GetComponentPacket*)pBuffer)->hComponent = hObjComp;

	pBuffer += sizeof(GetComponentPacket);

	strcpy_s((char*)pBuffer, lengthof(au8PacketBuffer) - (pBuffer - au8PacketBuffer), pchName);

	pBuffer += uNameLen;

	BIZ_ASSERT((pBuffer - au8PacketBuffer) <= lengthof(au8PacketBuffer));

	AUDIO_SYS_DEBUG("ObjectManager::ReuseObjectComponent() '%s'", pchName?pchName:"");

	if (! System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, au8PacketBuffer, sizeof(ObjectManager::GetComponentPacket) + uNameLen))
		hObjComp.SetAsInvalid();

	return true;	
}

////////////////////////////////////////////////////////////////////////////////
//	ReuseObjectComponent
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::ReuseObjectComponent(ObjectHandle hObject, ObjectComponentHandle hObjComp, StringHash nameHash)
{
	GetComponentFromHashPacket thePacket;

	thePacket.packet.uMessage = eGetComponentGenericFromHash;
	thePacket.hObject = hObject;
	thePacket.nameHash = nameHash;
	thePacket.hComponent = hObjComp;

	AUDIO_SYS_DEBUG("ObjectManager::ReuseObjectComponent() (from hash) 0x%x", nameHash.GetValue());

	if (! System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(ObjectManager::GetComponentFromHashPacket)))
		hObjComp.SetAsInvalid();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	SetValue
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::SetValue(InstanceComponentHandle hParamIn, F32 fValue)
{
#ifndef CFG_SPEED
	if (! IsInstanceComponentValid(hParamIn))
	{
		static int count = 0;
		if (++count < 10)	// Stop the spam!
			AUDIO_SYS_WARN("Trying to set value using invalid instance component");
		return;
	}
#endif

	SetValuePacket thePacket;

	thePacket.packet.uMessage = eSetValue;
	thePacket.hParamIn = hParamIn;
	thePacket.fValue = fValue;

	System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(SetValuePacket));
}

////////////////////////////////////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::Value(InstanceComponentHandle hParamOut, U32 hDataTracker, AudioRequestDataCallback pCallback)
{
#ifndef CFG_SPEED
	if (! IsInstanceComponentValid(hParamOut))
	{
		AUDIO_SYS_WARN("Trying to retrieve value using invalid instance component");
		return;
	}
#endif

	GetValuePacket thePacket;

	thePacket.packet.uMessage = eValue;
	thePacket.hParamOut = hParamOut;
	thePacket.hDataTracker = hDataTracker;
	thePacket.pCallback = pCallback;

	System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(GetValuePacket));
}

////////////////////////////////////////////////////////////////////////////////
//	Execute
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::Execute(InstanceComponentHandle hActionIn)
{
#ifndef CFG_SPEED
	if (! IsInstanceComponentValid(hActionIn))
	{
		AUDIO_SYS_WARN("Trying to execute using invalid instance component");
		return;
	}
#endif

	ExecutePacket thePacket;

	thePacket.packet.uMessage = eExecute;
	thePacket.hActionIn = hActionIn;

	System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(ExecutePacket));
}

////////////////////////////////////////////////////////////////////////////////
//	SetDimensions
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::SetDimensions(InstanceComponentHandle hVectorIn, F32* afDimensions)
{
#ifndef CFG_SPEED
	if (! IsInstanceComponentValid(hVectorIn))
	{
		AUDIO_SYS_WARN("Trying to set dimensions using invalid instance component");
		return;
	}
#endif

	SetDimensionsPacket thePacket;

	thePacket.packet.uMessage = eSetDimensions;
	thePacket.hVectorIn = hVectorIn;
	memcpy(thePacket.afDimensions, afDimensions, sizeof(F32) * 3);

	System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(SetDimensionsPacket));
}

////////////////////////////////////////////////////////////////////////////////
//	GetObjectComponentHandle
////////////////////////////////////////////////////////////////////////////////
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectComponentHandle(ObjectHandle hObject, ObjectManager::Messages eMessage, const char* pchName)
{
	U8 au8PacketBuffer[512] = {0};

	U32 uNameLen = strlen(pchName) + 1;

	U8* pBuffer = au8PacketBuffer;

	ObjectComponentHandle hComponent = m_ObjectComponentHandles.RequestUnusedHandle();

	if (! m_ObjectComponentHandles.IsHandleValid(hComponent))
		return hComponent;

	((GetComponentPacket*)pBuffer)->packet.uMessage = eMessage;
	((GetComponentPacket*)pBuffer)->hObject = hObject;
	((GetComponentPacket*)pBuffer)->hComponent = hComponent;

	pBuffer += sizeof(GetComponentPacket);

	strcpy_s((char*)pBuffer, lengthof(au8PacketBuffer) - (pBuffer - au8PacketBuffer), pchName);

	pBuffer += uNameLen;

	BIZ_ASSERT((pBuffer - au8PacketBuffer) <= lengthof(au8PacketBuffer));

	AUDIO_SYS_DEBUG("ObjectManager::GetComponent() '%s'", pchName?pchName:"");

	if (! System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, au8PacketBuffer, sizeof(ObjectManager::GetComponentPacket) + uNameLen))
		hComponent.SetAsInvalid();

	return hComponent;
}

////////////////////////////////////////////////////////////////////////////////
//	GetObjectComponentHandle StringHash version
////////////////////////////////////////////////////////////////////////////////
Audio::ObjectComponentHandle Audio::ObjectManager::GetObjectComponentHandleFromHash(ObjectHandle hObject, ObjectManager::Messages eMessage, StringHash nameHash)
{
	ObjectComponentHandle hComponent = m_ObjectComponentHandles.RequestUnusedHandle();

	if (! m_ObjectComponentHandles.IsHandleValid(hComponent))
		return hComponent;

	GetComponentFromHashPacket thePacket;

	thePacket.packet.uMessage = eMessage;
	thePacket.hObject = hObject;
	thePacket.nameHash = nameHash;
	thePacket.hComponent = hComponent;

	AUDIO_SYS_DEBUG("ObjectManager::GetComponent() (from hash) 0x%x", nameHash.GetValue());

	if (! System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(ObjectManager::GetComponentFromHashPacket)))
		hComponent.SetAsInvalid();

	return hComponent;
}

////////////////////////////////////////////////////////////////////////////////
//	GetInstanceComponentHandle
////////////////////////////////////////////////////////////////////////////////
Audio::InstanceComponentHandle Audio::ObjectManager::GetInstanceComponentHandle(ObjectComponentHandle hObjComp, InstanceHandle hInstance)
{
	if (! m_ObjectComponentHandles.IsHandleValid(hObjComp))
	{
		AUDIO_SYS_WARN("Get Instance Component handle failed. [hInst 0x%x, hObjCom 0x%x]", hInstance.Value(), hObjComp.Value());
		return InstanceComponentHandle();
	}

	return InstanceComponentHandle(hInstance, hObjComp);
}

////////////////////////////////////////////////////////////////////////////////
//	DereferenceInstanceComponentHandle
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::DereferenceInstanceComponentHandle(InstanceComponentHandle hComponent, U32 u32Flags, Instance** ppInst, Component** ppComp, bool bInstanceIsDoomed)
{
	ObjectComponentHandle hObjComp = ExtractObjectComponentHandle(hComponent);

	if ((HandleResource)(*(ppInst)) == -1)	// [Blur2PC][x64]
		return false;

	if (bInstanceIsDoomed)
		*(ppInst) = GetDoomedInstance(ExtractInstanceHandle(hComponent));
	else
		*(ppInst) = GetInstance(ExtractInstanceHandle(hComponent));

	U32 u32ObjCompInternal = GetResourceFromObjectComponentHandle(hObjComp);

	// a bit hacky? could do with a better way of doing the doomed handle stuff
	if (bInstanceIsDoomed && (S32)u32ObjCompInternal == -1)
		u32ObjCompInternal = GetDoomedResourceFromObjectComponentHandle(hObjComp);

	ObjectHandle hObjHandle = ExtractObjectHandle(u32ObjCompInternal);
	U16 u16CompHandleInternal = ExtractComponentIndex(u32ObjCompInternal);

	if (u16CompHandleInternal != 0xffff)
	{
		Object* pObj = GetObject(hObjHandle);

		if ((! pObj) && bInstanceIsDoomed)
			pObj = GetDoomedObject(hObjHandle);

		if ((! *(ppInst)) || (! pObj))
			return false;

		else if (pObj != (*(ppInst))->GetObject())
			return false;

		*(ppComp) = pObj->GetComponent(u16CompHandleInternal);

		if (((*ppComp)->Flags() & u32Flags) != u32Flags)
			return false;
	}
	else
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseObjectComponentHandle
////////////////////////////////////////////////////////////////////////////////
bool Audio::ObjectManager::ReleaseObjectComponentHandle(ObjectComponentHandle hObjComp, bool bReserveHandle)
{
	if (! m_ObjectComponentHandles.FlagHandleForRelease(hObjComp))
		return false;

	ReleaseObjectComponentHandlePacket thePacket;

	thePacket.packet.uMessage = eReleaseObjectComponentHandle;
	thePacket.hObjComp = hObjComp;
	thePacket.bReserveHandle = bReserveHandle;

	AUDIO_SYS_DEBUG("ObjectManager::ReleaseObjectComponentHandle()");

	return (System::Get()->GetQueue()->SendItem(Core::SendQueue::eObjectManager, &thePacket, sizeof(ObjectManager::ReleaseObjectComponentHandlePacket)));
}

////////////////////////////////////////////////////////////////////////////////
//	GetPeakTempFrameBufferObjectName
////////////////////////////////////////////////////////////////////////////////
char* Audio::ObjectManager::GetPeakTempFrameBufferObjectName()
{ 
	#ifndef CFG_GOLD
	return m_pchPeakFrameBufferObjectName;
	#else
	return "<NOT AVAILABLE IN THIS BUILD CONFIGURATION>";
	#endif
}

////////////////////////////////////////////////////////////////////////////////
//	SetPeakTempFrameBufferObjectName
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::SetPeakTempFrameBufferObjectName(const char* pchName) 
{ 
	#ifndef CFG_GOLD
	strncpy_s(m_pchPeakFrameBufferObjectName, lengthof(m_pchPeakFrameBufferObjectName), pchName, _TRUNCATE);
	#endif
}

////////////////////////////////////////////////////////////////////////////////
//	GetPeakTempFrameBufferObjectName
////////////////////////////////////////////////////////////////////////////////
char* Audio::ObjectManager::GetPeakInstanceStateDataObjectName() 
{ 
	#ifndef CFG_GOLD
	return m_pchPeakInstanceStateDataObjectName;
	#else
	return "<NOT AVAILABLE IN THIS BUILD CONFIGURATION>";
	#endif
}

////////////////////////////////////////////////////////////////////////////////
//	SetPeakTempFrameBufferObjectName
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::SetPeakInstanceStateDataObjectName(const char* pchName) 
{ 
	#ifndef CFG_GOLD
	strncpy_s(m_pchPeakInstanceStateDataObjectName, lengthof(m_pchPeakInstanceStateDataObjectName), pchName, _TRUNCATE);
	#endif
}

////////////////////////////////////////////////////////////////////////////////
//	LogPeakTempFrameBufferUsage
////////////////////////////////////////////////////////////////////////////////
void Audio::ObjectManager::LogPeakTempInstanceBufferUsage() 
{
	#ifndef CFG_GOLD
	AUDIO_SYS_DEBUG("======================================");
	AUDIO_SYS_DEBUG("Peak temp frame buffer size is %d bytes (%d k) ['%s']", m_uPeakTempFrameBufferSize, m_uPeakTempFrameBufferSize/1024, m_pchPeakFrameBufferObjectName);
	AUDIO_SYS_DEBUG("Peak instance state data size is %d bytes (%d k) ['%s']", m_uPeakInstanceStateDataSize, m_uPeakInstanceStateDataSize/1024, m_pchPeakInstanceStateDataObjectName);
	AUDIO_SYS_DEBUG("======================================");
	#endif
}
		
#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
