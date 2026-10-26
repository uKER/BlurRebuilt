#include "SharedActorsPch.h"

#include <actor_system/Actor.h>
#include <actor_system/ActorComponent.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/ActorList.h>
#include <actor_system/ActorMessages.h>
#include <misc_shared/DataRef/Sorcery.h>
#include <shared_actors/XtActorCreation.h>

//#include "XtActorMessages.h"



namespace XtActorCreation
{

bool CreateActors(const Actor::I_ActorCreationList* pCreationList, Actor::C_ActorList* pActorList)
{
	Actor::C_ActorCreationConfig config;

	if (!pActorList->CreatePreLoad(*pCreationList, config))
	{
		LOGERROR(Actor::GetDebugLogLevelDesign(), DBSTR "Failed to create level actors at pre-load");
		return false;
	}

	if (pActorList->IsWaitingForLoad(*pCreationList))
	{
		// TODO: [Oscar] deprecate this route, caller should manage priming
		LOGERROR(Actor::GetDebugLogGeneral(), DBSTR "Level actors are priming, create will fail");
	}

	if (!pActorList->CreatePostLoad(*pCreationList, config))
	{
		LOGERROR(Actor::GetDebugLogLevelDesign(), DBSTR "Failed to create level actors at post-load");
		return false;
	}

	return true;
}

#ifndef CFG_GOLD

void DebugRender(const Actor::I_ActorCreationList* pCreationList, DbUtil::I_DebugRenderPtr debugRenderPtr)
{
	const U32 numCreators = pCreationList->CalcNumCreators();

	for (U32 i = 0; i < numCreators; ++i)
	{
		Actor::C_ActorCreationData::C_DataRef creator = pCreationList->GetCreator(i);

		if (!creator.IsNull())
			creator->DebugRender(debugRenderPtr);
	}
}

#endif

} // namespace XtActorCreation




// Methods
// -------


C_LevelActorCreationList::F_CustomSorceryCallback C_LevelActorCreationList::s_pCustomSorceryCallbackFn = NULL;
void* C_LevelActorCreationList::s_pCustomSorceryCallbackData = NULL;

/**
Invokes pCallbackFn for each actor creation data in the list.
@param pCallbackFn callback function to invoke.
*/
void C_LevelActorCreationList::DoSorceryCallback(Actor::F_ActorCreationDataSorceryCallback pCallbackFn)
{
	for (S32 i = 0; i < m_LevelActors.count; ++i)
	{
		Actors::C_GameActorCreate* pCreateData = m_LevelActors[i];

		if ((pCreateData != NULL) && !pCreateData->m_IsCommented)
			pCallbackFn(pCreateData);
	}
}

/**
Invokes pCallbackFn for each component creation data in the list.
@param pCallbackFn callback function to invoke.
*/
void C_LevelActorCreationList::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	for (S32 i = 0; i < m_LevelActors.count; ++i)
	{
		Actors::C_GameActorCreate* pCreateData = m_LevelActors[i];

		if ((pCreateData != NULL) && !pCreateData->m_IsCommented)
			pCreateData->DoSorceryCallback(pCallbackFn);
	}
}

U32 C_LevelActorCreationList::CalcNumCreators() const
{
	return m_LevelActors.count;
}

Actor::C_ActorCreationData::C_DataRef C_LevelActorCreationList::GetCreator(const U32 index) const
{
	if (index < (U32)m_LevelActors.count)
	{
		Actors::C_GameActorCreate* pCreateData = m_LevelActors.elements[index];

		if (pCreateData != NULL)
		{
			if (!pCreateData->m_IsCommented)
			{
				Actor::C_ActorCreationData::C_DataRef ptr = DataRef::SorceryLookup<Actor::C_ActorCreationData>(pCreateData);

				if (!ptr.IsNull())
					return ptr;
				else
					LOGERROR(Actor::GetDebugLogLevelDesign(), "Level Actor %s is missing from DataRef sorcery lookup", pCreateData->m_Name.string);
			}
		}
		else
			LOGWARN(Actor::GetDebugLogLevelDesign(), "Level Actor %d is NULL", index);
	}

	return Actor::C_ActorCreationData::C_DataRef();
}


//Level Actor Creation Data
C_LevelActorCreationData::C_LevelActorCreationData():m_LevelActorCreationPack(NULL)
{

}
C_LevelActorCreationData::C_LevelActorCreationData(XtHandle<C_LevelActorCreationList> hRecord):
	m_LevelActorCreationPack(NULL),
	m_LevelActorCreationList(hRecord)
{
	if (hRecord)
		AddCreationListComposite(hRecord.Object());

}

C_LevelActorCreationData::~C_LevelActorCreationData()
{
	//Free();
}

extern void XtLevelLoadErrorCallback(XtPackOpenStatus *status,void *context);

// Load the level actor creation pak file
bool C_LevelActorCreationData::Load(XtDb* pXtDb, Sys::I_FileSystem &fileSys, const Util::C_String &fullPath)
{
	bool bFailed = false;
	m_pXtDb = pXtDb;

	if (m_LevelActorCreationList)
		RemoveCreationListComposite(m_LevelActorCreationList.Object());

	Free();

	if (Sys::I_File* file = fileSys.Open(fullPath))
	{
		int		length	= (int)file->Length();
		XtU8*	data	= new XtU8[length];

		if (data)
		{
			if (file->Read(data, length))
			{
				XtPackOpenParams params;

				params.callback = XtLevelLoadErrorCallback;

				m_LevelActorCreationPack = XtPackOpen(pXtDb, data, length, &params);

				if (!m_LevelActorCreationPack)
					bFailed = true;
			}
			else
				bFailed = true;

			delete [] data;
		}
		else
			bFailed = true;

		delete file;
	}
	else
		bFailed = true;

	if (bFailed)
	{
// 		LOGERROR( Game::g_Log, "No New actor placement info for this level. It needs to re-exported with the new scene convertor.\n");
// 		LOGERROR( BuildTest::g_Log, "No New actor placement info for this level. It needs to re-exported with the new scene convertor.\n");
		return false;
	}

	if (m_LevelActorCreationPack)
	{
		XtType* type = XtGetType<C_LevelActorCreationList>();

		for(int i = 0; i < (int)m_LevelActorCreationPack->count; i++)
		{
			XtRecord* record = m_LevelActorCreationPack->records[i].record;

			if ((record != NULL) && (record->type == type))
			{
				m_LevelActorCreationList = XtHandle<C_LevelActorCreationList>(record);
				break;
			}
		}
	}

	if (m_LevelActorCreationList)
		AddCreationListComposite(m_LevelActorCreationList.Object());

	return true;
}

void C_LevelActorCreationData::Free()
{
	if (m_LevelActorCreationPack)
	{
		XtPackClose(m_pXtDb, m_LevelActorCreationPack);
		m_LevelActorCreationPack = NULL;
	}
}

XtId C_LevelActorCreationData::GetActorListRecordId() const
{
	return m_LevelActorCreationList.Id();
}

// Level Actor Creation Data List 

C_LevelActorCreationDataList::C_LevelActorCreationDataList()
{

};

C_LevelActorCreationDataList::~C_LevelActorCreationDataList()
{
	Free();
};

bool C_LevelActorCreationDataList::AddActorList(C_LevelActorCreationData* pActorList)
{
	if (pActorList == NULL)
		return false;

#ifndef CFG_GOLD
	// check if we've already got this list
	{
		const XtId newId = pActorList->GetActorListRecordId();

		for (Btl::Vector<C_LevelActorCreationData*>::ConstIterator it = m_LevelCreationDataList.Begin(); it != m_LevelCreationDataList.End(); ++it)
		{
			if ((*it)->GetActorListRecordId() == newId)
			{
				BIZ_ASSERT_FAIL_SPEED("Attempted to add duplicate ActorList record 0x%8x (will crash Master)", newId);
				pActorList->Free();
				delete pActorList;
				return false;
			}
		}
	}
#endif

	m_LevelCreationDataList.PushBack(pActorList);
	AddCreationListComposite(pActorList);
	return true;
}

bool C_LevelActorCreationDataList::AddActorListFromFile(XtDb* pXtDb, Sys::I_FileSystem &fileSys, const Util::C_String &fullPath)
{
	C_LevelActorCreationData* pLevelCreation = new C_LevelActorCreationData;

	if(pLevelCreation->Load(pXtDb, fileSys, fullPath))
	{
		return AddActorList(pLevelCreation);
	}
	else
	{
		delete pLevelCreation;
		return false;
	}
}

bool C_LevelActorCreationDataList::AddActorListFromRecord(XtHandle<C_LevelActorCreationList> hRecord)
{
	if(hRecord)
	{
		C_LevelActorCreationData* pLevelCreation = new C_LevelActorCreationData(hRecord);
		return AddActorList(pLevelCreation);
	}
	else
		return false;
}


void C_LevelActorCreationDataList::Free(void)
{
	for(S32 listNo = 0; listNo < m_LevelCreationDataList.Size(); listNo++)
	{
		C_LevelActorCreationData* pLevelCreation = m_LevelCreationDataList[listNo];
		RemoveCreationListComposite(pLevelCreation);
		pLevelCreation->Free();
		delete pLevelCreation;
	}

	m_LevelCreationDataList.Resize(0);
}

// XT Annotations
// --------------

XT_TYPE(C_LevelPlatform, "Platform")
	XT_FIELD(m_PlatformName, "PlatformName")
XT_END_TYPE

XT_TYPE(Level::C_GameData, "GameData")
XT_END_TYPE

// Level Actor List
static void LevelActorListXtCallback(XtEventType eventType, XtRecord* pRecord, C_LevelActorCreationList* pData, XtCallbackContext* pContext)
{
	// this handles the patching-up and serialisation of DataRef::C_Ptr<C_ActorCreationData>
	DataRef::SorceryCallback<Actor::C_ActorCreationData, C_LevelActorCreationList>(eventType, pRecord, pData, pContext);
	DataRef::SorceryCallback<Component::I_ComponentCreator, C_LevelActorCreationList>(eventType, pRecord, pData, pContext);

	if ((eventType == kXtEtConstruct) && (C_LevelActorCreationList::s_pCustomSorceryCallbackFn != NULL))
	{
		(*C_LevelActorCreationList::s_pCustomSorceryCallbackFn)(pData, C_LevelActorCreationList::s_pCustomSorceryCallbackData);
	}
}

XT_TYPE(C_LevelActorCreationList, "LevelActorList")
XT_CALLBACK(LevelActorListXtCallback)
XT_FIELD(m_LevelActors, "LevelActors")
XT_FIELD(m_LevelPlatforms, "Platforms")
XT_FIELD(m_LevelData, "LevelData")
XT_END_TYPE

XT_LIBRARY(XtActorCreation)
XT_DEPENDS_ON(LevelAnimation)
XT_REGISTER_TYPE(C_LevelPlatform)
XT_REGISTER_TYPE(C_LevelActorCreationList)
XT_REGISTER_TYPE(Level::C_GameData)
XT_END_LIBRARY

