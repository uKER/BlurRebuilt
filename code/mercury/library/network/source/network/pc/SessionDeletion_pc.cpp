#include <StdAfx.h>
#include <Network\Core.h>
#include <Network\SessionDeletion.h>
#include <Network\Securekey.h> 
#include <Network\Session.h>
#include <bdNet\bdNet.h>
#include <bdLobby\bdLobbyService.h>
#include <bdLobby\bdMatchMaking\bdMatchMaking.h>
#include <bdLobby\bdRemoteTaskManager\bdRemoteTask.h>
#include <bdSocket\bdSocketRouter.h>
#include <Network\DemonwareSession.h>
#include "..\Shared\SessionCommand_demonware.h" 

namespace NetPlay
{
	// static instance..
	C_SessionDeletion C_SessionDeletion::s_SessionDeletion;

	C_SessionDeletion::C_SessionDeletion() : m_Initialised(false), m_pDeleteCommandStorage(NULL), m_pDeleteCommandPool(NULL), m_pDeleteCommandList(NULL)
	{
	}

	C_SessionDeletion::~C_SessionDeletion()
	{
	}

	bool C_SessionDeletion::Initialise(const U32 deleteQueueSize)
	{
		if (m_Initialised)
			return true;

		assert(deleteQueueSize);
		if (0 == deleteQueueSize)
			return false;

		m_pDeleteCommandStorage = new C_SessionDeletionCommand[deleteQueueSize];
		m_pDeleteCommandPool = reinterpret_cast<void*> (new C_SessionDeletionCommandList);
		m_pDeleteCommandList = reinterpret_cast<void*> (new C_SessionDeletionCommandList);

		if (m_pDeleteCommandStorage && m_pDeleteCommandPool && m_pDeleteCommandList)
		{
			// put all delete commands into pool
			C_SessionDeletionCommandList* pPool = reinterpret_cast<C_SessionDeletionCommandList*> (m_pDeleteCommandPool);
			for (U32 i=0; i< deleteQueueSize; i++)
				pPool->PushBack(m_pDeleteCommandStorage[i]);

			m_Initialised = true;
		}
		else
			Cleanup();

		return m_Initialised;

	}

	void C_SessionDeletion::Cleanup(void)
	{
		if (m_pDeleteCommandList)
		{
			C_SessionDeletionCommandList* p = reinterpret_cast<C_SessionDeletionCommandList*> (m_pDeleteCommandList);
			if (!p->Empty())
			{
				LOGWARN( g_Log, "C_SessionDeletion::Cleanup() - Closing down when delete commands active.");
				while (!p->Empty())
				{
					C_SessionDeletionCommandList::Iterator i = p->Begin();
					NetPlay::DemonwareRemoteTask::Close(i->m_AsyncTask);
					p->Erase(i); // erase item from list and move to next, no point adding it to the pool as that will be blitzed next.
				}
			}
			p->Clear();
			delete p;
			m_pDeleteCommandList = NULL;
		}

		// remove any items from the delete command pool
		if (m_pDeleteCommandPool)
		{
			C_SessionDeletionCommandList* p = reinterpret_cast<C_SessionDeletionCommandList*> (m_pDeleteCommandPool);
			p->Clear();
			delete p;
			m_pDeleteCommandPool = NULL;
		}

		// release delete command storage..
		if (m_pDeleteCommandStorage)
		{
			delete [] m_pDeleteCommandStorage;
			m_pDeleteCommandStorage = NULL;
		}

		m_Initialised = false;
	}

	void C_SessionDeletion::Poll(void)
	{
		if (!m_Initialised)
			return;

		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();

		C_SessionDeletionCommandList* pDeleteList = reinterpret_cast<C_SessionDeletionCommandList*> (m_pDeleteCommandList);
		C_SessionDeletionCommandList* pDeletePool = reinterpret_cast<C_SessionDeletionCommandList*> (m_pDeleteCommandPool);
		if (!pDeleteList->Empty())
		{
			C_SessionDeletionCommandList::Iterator i = pDeleteList->Begin();
			if (i->m_Active)
			{
				if (!NetPlay::DemonwareRemoteTask::IsBusy(i->m_AsyncTask))
				{
					bool error = NetPlay::DemonwareRemoteTask::CheckState(i->m_AsyncTask) == NetPlay::DemonwareRemoteTask::eFailed ? true : false;

					LOGINFO( g_Log, "C_SessionDeletion : Delete complete");
					NetPlay::DemonwareRemoteTask::Close(i->m_AsyncTask);

					// erase item from list and give back to pool..
					C_SessionDeletionCommandList::Iterator cur = i;
					i = pDeleteList->Erase(cur);
					pDeletePool->PushBack(*cur);
				}
				else
				{
					return;
				}
			}

			if (pDeleteList->Empty())
				return;

			i = pDeleteList->Begin();
			bool error = false;	
			if (lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
			{
				bdMatchMaking* pMM = lobbyService->getMatchMaking();
				if (pMM)
				{
					bdSessionID sessionId;
					memcpy(sessionId.m_sessionID.ab, i->m_CommandData.m_Delete.keyId, BD_SECURITY_ID_LENGTH);

					i->m_AsyncTask = pMM->deleteSession(sessionId);
					if (NetPlay::DemonwareRemoteTask::IsBusy(i->m_AsyncTask))
						i->m_Active = true;
					else
						error = true;
				}
				else
					error = true;
			}
			else 
				error = true;

			if (error)
			{
				LOGINFO( g_Log, "C_SessionDeletion : failed to start deletion process.");

				// erase item from list and give back to pool..
				C_SessionDeletionCommandList::Iterator cur = i;
				i = pDeleteList->Erase(cur);
				pDeletePool->PushBack(*cur);
			}
		}
	}

	U32 C_SessionDeletion::ActiveCount(void)
	{
		U32 active = 0;
		if (m_Initialised)
		{
			C_SessionDeletionCommandList* pDeleteList = reinterpret_cast<C_SessionDeletionCommandList*> (m_pDeleteCommandList);
			active = pDeleteList->Size();
		}
		return active;
	}

	U32 C_SessionDeletion::Remaining(void)
	{
		U32 remaining = 0;
		if (m_Initialised)
		{
			C_SessionDeletionCommandList* pDeletePool = reinterpret_cast<C_SessionDeletionCommandList*> (m_pDeleteCommandPool);
			remaining = pDeletePool->Size();
		}
		return remaining;
	}

	bool C_SessionDeletion::Add(C_Session& toDelete)
	{
		if (m_Initialised)
		{
			if (Remaining() == 0)
			{
				LOGFATAL(g_Log, "C_SesionDeletion::Add()  - Not enough storage allocated to add session to deletion queue.");
				return false;
			}

			C_DemonwareSession* pSession = toDelete.m_pDemonwareSession;
			assert(pSession);
			if (!pSession)
				return false;

			C_SessionDeletionCommandList* pList = reinterpret_cast<C_SessionDeletionCommandList*> (m_pDeleteCommandList);
			C_SessionDeletionCommandList* pPool = reinterpret_cast<C_SessionDeletionCommandList*> (m_pDeleteCommandPool);

			C_SessionDeletionCommandList::Iterator i = pPool->Begin();
			pPool->Erase(pPool->Begin());
			i->m_Command	=	C_SessionDeletionCommand::eCommandDeleteSession;

			// assume session was actually created on the server. so key id will be valid;
			memcpy(i->m_CommandData.m_Delete.keyId, pSession->m_sessionID.ab, BD_SECURITY_ID_LENGTH);

			// add to list for asynchronous processing.
			pList->PushBack(*i);
			return true;
		}

		return false;
	}
}
