#include "AudioPch.h"
#include "..\..\Include\AppInterfaceLayer.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	constructor
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::Subsystem::Subsystem()
{
	// temp. so broadcast message works
	m_u32IdFlags = 1;
	m_u32StateFlags = 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	destructor
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::Subsystem::~Subsystem()
{
	for (U32 i = 0; i < m_aEntityType.GetHighestNodeIndex(); i++)
	{
		if (m_aEntityType[i])
		{
			delete m_aEntityType[i];
			m_aEntityType.FreeByIndex(i);
		}
	}

	for (U32 i = 0; i < m_aEntityInstance.GetHighestNodeIndex(); i++)
	{
		if (m_aEntityInstance[i])
		{
			delete m_aEntityInstance[i];
			m_aEntityInstance.FreeByIndex(i);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SystemUpdate
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::Subsystem::SystemUpdate()
{
	if (! CheckStateFlags(flagSuspended))
	{
		Update();
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateEntityType
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityTypeHandle Audio::Subsystem::CreateEntityType(EntityDefinitionHandle hDefinition)
{
	EntityType* pType = InterfaceController::Get().PrivateCreateEntityType(hDefinition);

	if (! pType)
		return PagedArrayHandle();	// returns invalid handle

	return m_aEntityType.AddEntry(pType);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateEntityInstance
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityInstanceHandle Audio::Subsystem::CreateEntityInstance(EntityTypeHandle hParentEntityType)
{
	EntityType* pType = m_aEntityType.GetByHandle(hParentEntityType);

	if (! pType)
		return PagedArrayHandle();	// returns invalid handle

	EntityInstance* pInstance = InterfaceController::Get().PrivateCreateEntityInstance(pType);

	//if (! pInstance)
	//	return PagedArrayHandle();	// returns invalid handle

	//if (! InterfaceController::Get().RunFunctionInEntityInstance(pInstance, "CreateInstance"))
	//	return PagedArrayHandle();	// returns invalid handle

	#ifndef CFG_GOLD
	EntityDefinition* pDef = InterfaceController::Get().GetDefinition(pInstance);
	if (pDef)
		AUDIO_SYS_DEBUG("Created AudioEntity instance of '%s'", pDef->Name().Chars());
	#endif

	Audio::EntityInstanceHandle hRet = m_aEntityInstance.AddEntry(pInstance);

	InterfaceController::Get().RunFunctionInEntityInstance(GetAudioEntityHandle(hRet), "CreateInstance");

	return hRet;
}

 
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReleaseEntityType
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::Subsystem::ReleaseEntityType(EntityTypeHandle hEntityType)
{
	EntityType* pType = m_aEntityType.GetByHandle(hEntityType);

	if (! pType)
		return false;

	if (pType->GetInstanceRefCount() > 0)
	{
		Error("Trying to release an audio entity type that still has active instances");
		return false;
	}

	EntityDefinitionHandle hDef = pType->GetDefinitionHandle();
	EntityDefinition* pDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hDef);

	if (pDef)
		AUDIO_SYS_DEBUG("Release Entity Type '%s'", pDef->Name().Chars());

#ifdef GARBAGE_COLLECT_ON_ENTITY_RELEASE
	for (U32 i = 0; i < pDef->GetNumObjects(); i++)
		System::Get()->GetObjectManager()->Destroy(pType->GetObject(i));
#else
	// run all entity type scripts in reverse

	FunctionExecutionHistoryEntry historyEntry;

	while (pType->PopFunctionExecutionEntry(historyEntry))
		pDef->RunReverseEntityTypeFunction(historyEntry.hFunction, pType);
#endif

	delete pType;

	m_aEntityType.Free(hEntityType);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReleaseEntity
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::Subsystem::ReleaseEntityInstance(EntityInstanceHandle hEntity)
{
	EntityInstance* pEntityInstance = m_aEntityInstance.GetByHandle(hEntity);

	if (! pEntityInstance)
		return false;

	EntityDefinitionHandle hDef = pEntityInstance->GetDefinitionHandle();
	EntityDefinition* pDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hDef);

	AUDIO_SYS_DEBUG("Release Entity Instance '%s'", pDef->Name().Chars());

#ifdef GARBAGE_COLLECT_ON_ENTITY_RELEASE
	for (U32 i = 0; i < pDef->GetNumInstances(); i++)
		System::Get()->GetObjectManager()->DestroyObjectInstance(pEntityInstance->GetInstance(i));
#else
	// first, run all parameterised entity instance scripts in reverse

	FunctionExecutionHistoryEntry historyEntry;

	for (S32 i = pEntityInstance->GetNumFunctionExecutionHistoryEntries() - 1; i >= 0; i--)
	{
		pEntityInstance->GetFunctionExecutionEntry(i, historyEntry);

		if (historyEntry.eFunctionType == eParameterisedEntityInstanceFunction)
		{
			EntityInstance* pFunctionEntityInstance = InterfaceController::Get().GetEntityInstanceFromHandle(historyEntry.hEntity);
			EntityInstance* pFunctionParameterInstance = InterfaceController::Get().GetEntityInstanceFromHandle(historyEntry.hParameterEntity);

			if (pFunctionEntityInstance && pFunctionParameterInstance)
			{
				EntityDefinition* pFunctionDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(pFunctionEntityInstance->GetDefinitionHandle());

				if (pFunctionDef)
					pFunctionDef->RunReverseEntityInstanceParameterisedFunction(historyEntry.hFunction, pFunctionEntityInstance, pFunctionParameterInstance);
			}
		}
	}

	// next, run all non-parameterised entity instance scripts in reverse

	for (S32 i = pEntityInstance->GetNumFunctionExecutionHistoryEntries() - 1; i >= 0; i--)
	{
		pEntityInstance->GetFunctionExecutionEntry(i, historyEntry);

		if (historyEntry.eFunctionType == eEntityInstanceFunction)
			pDef->RunReverseEntityInstanceFunction(historyEntry.hFunction, pEntityInstance);
	}

#endif

	EntityType* pType = pEntityInstance->GetParentEntityType();

	if (pType)
		pType->DecRefCount();

	delete pEntityInstance;

	m_aEntityInstance.Free(hEntity);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Assert
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::Subsystem::InternalAssert(bool bExp, const char* strMsg)
{
	InterfaceController::Get().InternalAssert(bExp, strMsg);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Error
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::Subsystem::Error(const char* strMsg)
{
	InterfaceController::Get().Error(strMsg);
}
