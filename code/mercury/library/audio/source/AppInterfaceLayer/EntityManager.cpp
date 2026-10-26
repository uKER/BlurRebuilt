#include "AudioPch.h"

#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#endif

//using namespace Audio;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	(Constructor)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityManager::EntityManager()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	(destructor)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityManager::~EntityManager()
{
	for (U32 i = 0; i < m_apDefinition.GetHighestNodeIndex(); i++)
	{
		if (m_apDefinition[i])
		{
			delete m_apDefinition[i];
			m_apDefinition.FreeByIndex(i);
		}
	}

	for (S32 i = 0; i < m_apCallback.Size(); i++)
	{
		delete m_apCallback[i];
		m_apCallback[i] = 0;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateDefinition
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDefinitionHandle Audio::EntityManager::CreateDefinition()
{
	EntityDefinition* pDef = new EntityDefinition();

	if (pDef)
		return m_apDefinition.AddEntry(pDef);

	return PagedArrayHandle();	// returns invalid handle
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReleaseDefinition
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::EntityManager::ReleaseDefinition(Audio::EntityDefinitionHandle hDefinition)
{
	EntityDefinition* pDef = m_apDefinition.GetByHandle(hDefinition);

	if (! pDef)
		return false;

	delete pDef;

	return m_apDefinition.Free(hDefinition);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetDefinitionFromHandle
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDefinition* Audio::EntityManager::GetDefinitionFromHandle(Audio::EntityDefinitionHandle hDef)
{
	return m_apDefinition.GetByHandle(hDef);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SpawnEntityType
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityType* Audio::EntityManager::SpawnEntityType(Audio::EntityDefinitionHandle hDef)
{
	EntityDefinition* pDef = m_apDefinition.GetByHandle(hDef);

	if (! pDef)
		return 0;

	EntityType* pEntity = pDef->SpawnEntityType();

	AUDIO_SYS_DEBUG("Create Entity Type '%s'", pDef->Name().Chars());

	if (pEntity)
		pEntity->SetDefinition(hDef);

	return pEntity;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SpawnEntityInstance
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityInstance* Audio::EntityManager::SpawnEntityInstance(Audio::EntityType* pEntityType)
{
	EntityDefinition* pDef = m_apDefinition.GetByHandle(pEntityType->GetDefinitionHandle());

	if (pDef)
	{
		EntityInstance* pEntity = pDef->SpawnEntityInstance();

		if (pEntity)
		{
			pEntity->SetEntityType(pEntityType);
			pEntityType->IncRefCount();
			pEntity->SetDefinition(pEntityType->GetDefinitionHandle());
		}

		return pEntity;
	}

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddCallbackData
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
U32 Audio::EntityManager::AddCallbackData(EntityInstance* pEntity, EntityDataMemberHandle hDataMember)
{
	EntityCallback* pCallback = new EntityCallback();

	pCallback->m_pEntity = pEntity;
	pCallback->m_hDataMember = hDataMember;

	m_apCallback.PushBack(pCallback);

	return m_apCallback.Size() - 1;
}


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
