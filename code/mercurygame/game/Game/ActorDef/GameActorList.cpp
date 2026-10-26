#include "stdafx.h"

#include <Game/ActorDef/GameActorList.h>

#include <actor_system/HandleManager.h>
#include <misc_shared/messaging/GameMessaging.h>
#include <shared_actors/XtActorCreation.h>
#include <xt/XtGame.h>

#include <GameApp.h>
#include <Game/Game.h>


// definition of function externed in XtActorCreation...
void XtLevelLoadErrorCallback(XtPackOpenStatus *status,void *context)
{
	LOGWARN(Game::g_Log, "Xt Level : %s", status->message);
}


namespace GameActorList_Impl
{

const U32 kMaxNoActors = 512;
const U32 kNoMsgQueues = 8;

#ifndef CFG_GOLD
class C_DebugWriteBackActorHandlesToCreationData : public Actor::C_ActorList::I_ConstActorListVisitor
{
public:
	virtual bool VisitHandleItem(const Actor::C_Actor* pActor)
	{
		// nasty bodge to write the actor handle back into the creation data, to cope with artery updates overwriting ActorStudio data
		if (pActor->GetCreationData().GetStorageType() == DataRef::kSorceryStorageType)
		{
			const Actor::C_ActorCreationData* pCreationData = pActor->GetCreationData().GetData();
			if(pCreationData)
				pCreationData->LinkActorHandle(pActor->GetHandle());
		}
		return true;
	}

};

Debug::Bool g_DebugActorStudioDataUpdate("ActorDebug.ActorStudioDataUpdate.Enable", true);

void DebugActorStudioDataUpdate(C_LevelActorCreationList*, void* pExtraData)
{
	// ActorStudio data artery update callback, the actor system wasn't 
	// designed to cope with this so we'll have to try to fool it

	if (g_DebugActorStudioDataUpdate && (pExtraData != NULL))
	{
		Game::C_GameActorList* pGameActorList = reinterpret_cast<Game::C_GameActorList*>(pExtraData);

		// write actor handles back into creation data so that we can get from the new xt data to the actors created from the old xt data
		pGameActorList->DebugWriteBackActorHandlesToCreationData();

		// if you really want to go down that road then you can
		// try hooking in something to tell actors that their 
		// creation data has changed...
	}
}
#endif // CFG_GOLD

} // namespace GameActorList_Impl


namespace Game
{

C_GameActorList::C_GameActorList(Actor::C_ActorFactory* pFactory, const Sys::I_ContextProvider* pDefaultContextProvider) :
	  Actor::C_ActorList(GameActorList_Impl::kMaxNoActors, GameActorList_Impl::kNoMsgQueues, pFactory, pDefaultContextProvider)
	, m_PendingActorCreators()
	, m_pActorListXtPack(NULL)
{
#ifndef CFG_GOLD
	BIZ_ASSERT(C_LevelActorCreationList::s_pCustomSorceryCallbackFn == NULL);
	C_LevelActorCreationList::s_pCustomSorceryCallbackFn = &GameActorList_Impl::DebugActorStudioDataUpdate;
	C_LevelActorCreationList::s_pCustomSorceryCallbackData = this;
#endif
}

C_GameActorList::~C_GameActorList()
{
#ifndef CFG_GOLD
	BIZ_ASSERT(C_LevelActorCreationList::s_pCustomSorceryCallbackData == this);
	C_LevelActorCreationList::s_pCustomSorceryCallbackFn = NULL;
	C_LevelActorCreationList::s_pCustomSorceryCallbackData = NULL;
#endif

	if (m_pActorListXtPack != NULL)
	{
		XtPackClose(C_GameApp::GetInstance()->GetXtDatabase(), m_pActorListXtPack);
		m_pActorListXtPack = NULL;
	}
}

bool C_GameActorList::CreateActors(Actor::I_ActorCreationList* pActorList)
{
	// use default config for now
	Actor::C_ActorCreationConfig config;

	// first phase of creation, actors request necessary data...
	if (!CreatePreLoad(*pActorList, config))
	{
		delete pActorList;
		return false;
	}

	// if we're waiting for data to load then finish the job later
	if (IsWaitingForLoad(*pActorList))
	{	
		m_PendingActorCreators.PushBack(pActorList);
		return true;
	}

	// data is ready now, attempt to finalise actor creation
	const bool ok = CreatePostLoad(*pActorList, config);
	delete pActorList;

	// Deprecated, use InternalDependenciesCreatedImpl() or ExternalDependenciesCreatedImpl() as appropriate.
//	BroadcastMessage(GameMessaging::E_GameMsg_PostCreateInit);

	return ok;
}

Actor::C_ActorHandle C_GameActorList::CreateActor(const DataRef::C_Ptr<Actor::C_ActorCreationData>& pCreationData)
{
	if (!pCreationData.IsNull() && CreateActors(new Actor::C_ActorCreationListSingle(pCreationData)))
	{
		return pCreationData->GetActorHandle();
	}

	return Actor::C_ActorHandle();
}

bool C_GameActorList::CreateActors(const Util::C_String& actorsFilename)
{
	if (m_pActorListXtPack != NULL)
	{
		LOGERROR(Game::g_Log, DBSTR "attempted to load actor list from %s when actors are already loaded", actorsFilename.Chars());
		return false;
	}

	// load the xt file
	{
		// TODO: [Oscar] is there a better way to get a file system object?
		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(actorsFilename);

		if (pFile == NULL)
		{
			LOGERROR(Game::g_Log, DBSTR "failed to open actors file %s", actorsFilename.Chars());
		}
		else
		{
			// read the contents
			const U32 length = (U32)pFile->Length();
			XtU8* pData = new XtU8 [length];

			if (pData == NULL)
			{
				LOGERROR(Game::g_Log, DBSTR "failed to allocated %d bytes", length);
			}
			else
			{
				if (!pFile->Read(pData, length))
				{
					LOGERROR(Game::g_Log, DBSTR "failed to read %d bytes", length);
				}
				else
				{
					// create the pack
					m_pActorListXtPack = XtPackOpen(C_GameApp::GetInstance()->GetXtDatabase(), pData, length, NULL);
				}

				// finished with data
				delete [] pData;
			}

			// finished with file
			delete pFile;
		}
	}

	if (m_pActorListXtPack == NULL)
	{
		LOGERROR(Game::g_Log, DBSTR "failed to load actors from %s", actorsFilename.Chars());
		return false;
	}

	// grab the actor lists from the pack
	C_LevelActorCreationDataList* pActorList = new C_LevelActorCreationDataList;
	{
		bool found = false;
		const XtType* pType = XtGetType<C_LevelActorCreationList>();

		for (size_t i = 0; i < m_pActorListXtPack->count; ++i)
		{
			XtRecord* pRecord = m_pActorListXtPack->records[i].record;

			if ((pRecord != NULL) && (pRecord->type == pType))
			{
				pActorList->AddActorListFromRecord(XtHandle<C_LevelActorCreationList>(pRecord));
				found = true;
			}
		}

		if (!found)
		{
			LOGERROR(Game::g_Log, DBSTR "no actor data found in %s", actorsFilename.Chars());
		}
	}

	// load the actors
	return CreateActors(pActorList);
}

void C_GameActorList::Update(const F32 timeStep)
{
	// create pending actors if resources are ready
	{
		C_PendingActorCreators::Iterator it = m_PendingActorCreators.Begin();

		while (it != m_PendingActorCreators.End())
		{
			Actor::I_ActorCreationList* pCreationList = *it;

			if (IsWaitingForLoad(*pCreationList))
			{
				++it;
			}
			else
			{
				it = m_PendingActorCreators.Erase(it);

				Actor::C_ActorCreationConfig config;
				CreatePostLoad(*pCreationList, config);

				delete pCreationList;
			}
		}
	}

	// update actors
	{
		const GameMessaging::C_PreUpdateMsg preUpdateMsg(timeStep);
		GetSubscriberSystem().BroadcastMessage(&preUpdateMsg);
		
		const GameMessaging::C_UpdateMsg updateMsg(timeStep);
		GetSubscriberSystem().BroadcastMessage(&updateMsg);
	}
	
	// process delayed messages
	ProcessMessageQueue(0);
	
}

#ifndef CFG_GOLD
void C_GameActorList::DebugWriteBackActorHandlesToCreationData() const
{
	GameActorList_Impl::C_DebugWriteBackActorHandlesToCreationData visitor;
	VisitActors(visitor);
}
#endif // CFG_GOLD

} // namespace Game
