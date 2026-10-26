#include "ActorStudioMsgGamePch.h"

#include <actor_studio_msg_game/ActorStudioMessaging.h>
#include <actor_studio_msg_shared/MessageTypes.h>
#include <actor_studio_msg_shared/Messages.h>
#include <misc_shared/Messaging/MessageValue.h>
#include <shared_actors/XtActorCreation.h>
#include <misc_shared/Messaging/MessageDebug.h>
#include <shared_actors/SharedActorMessages.h>


#pragma warning (push, 3)
#ifdef PLATFORM_PC
#include <Winsock2.h>
#endif
#include <System/Artery/Artery.h>
#include <system/serialise.h>
#include <system/serialise/I_FileSystem.h>
#include <system/serialise/C_MemoryStream.h>
#include <core/threads.h>
#pragma warning (pop)

namespace ActorStudioMsgGame
{

C_ActorStudioMessaging& C_ActorStudioMessaging::Instance()
{
	static C_ActorStudioMessaging s_instance;
	return s_instance;
}

C_ActorStudioMessaging::C_ActorStudioMessaging() :
	m_pActorList(0),
	m_pActorPack(0)
{
	#ifndef CFG_GOLD
	const char* messageNames [] = 
	{
		"E_ActorStudioMsg_Drag", 
		"E_ActorStudioMsg_EndDrag", 
		"E_ActorStudioMsg_Selected", 
		"E_ActorStudioMsg_Deselected",
	};

	StaticAssert(lengthof(messageNames) == ActorStudioMsgShared::E_ActorStudioMsg_Count, array_size_must_match_enum_count);
	Messaging::RegisterMessageDebugInfo(messageNames, ActorStudioMsgShared::E_ActorStudioMsg_Count, ActorStudioMsgShared::E_ActorStudioMsg_Begin);
	#endif
}

C_ActorStudioMessaging::~C_ActorStudioMessaging()
{
	Exit();
}

#pragma warning (disable: 4390) // ';' : empty controlled statement found;

void C_ActorStudioMessaging::Init()
{
	ArteryRegister("ActorStudioMessaging", StaticNetProc, this);
}

void C_ActorStudioMessaging::Exit()
{
	if (m_clientMutex.Wait())
	{
		for (S32 i = m_clients.Size() - 1; i >= 0; i--)
		{
			if (m_clients[i])
				m_clients[i]->Close();
		}

		m_clients.Clear();
		m_clientMutex.Release();
	}
}

void C_ActorStudioMessaging::Update()
{
	if (m_clientMutex.Wait())
	{
		S32 i = 0;
		// Go through the list of clients:
		for (i = 0; i < m_clients.Size(); i++)
		{
			ProcessConnection(*m_clients[i]);
		}
		
		m_clientMutex.Release();
	}
	
	m_updateCount++;
}

void C_ActorStudioMessaging::SetActorList(Actor::C_ActorList* pActorList) 
{ 
	m_pActorList = pActorList; 
}

void C_ActorStudioMessaging::SetActorPack(const XtPack* pActorPack) 
{ 
	m_pActorPack = pActorPack; 
}

void C_ActorStudioMessaging::StaticNetProc(ArteryConnection con, void *self)
{
	((C_ActorStudioMessaging *)self)->NetProc(con);
}

void C_ActorStudioMessaging::NetProc(ArteryConnection con)
{
	if (!m_clientMutex.Wait())
		return;

	m_clients.PushBack(&con);
	m_clientMutex.Release();
	
	ArteryWaitContext ctx;
	while(con.IsConnected())
		con.Wait(ctx);

	if (!m_clientMutex.Wait())
		return;

	S32 i = 0;
	for (i = m_clients.Size() - 1; i >= 0; i--)
	{
		if (m_clients[i] == &con)
		{
			m_clients[i] = 0;
			m_clients.EraseByIndex(i);
		}
	}

	m_clientMutex.Release();
}

void C_ActorStudioMessaging::ProcessConnection(ArteryConnection conn)
{
	ArteryMessage msg;

	while (conn.IsConnected() && conn.TryReceive(msg))
	{
		HandleMessage(conn, msg);
	}

	//TODO: Send response to ActorStudio
}

// Handle messages sent by the client
void C_ActorStudioMessaging::HandleMessage(ArteryConnection& conn, ArteryMessage& msg)
{
	if(!m_pActorList || !m_pActorPack)
		return;

	ArteryMessageReader reader(msg);

	S32 len = msg.DataLength();
	U8* data = (U8 *)msg.Data();
	
	// read the message 'header'
	Messaging::TMsgType messageType;
	U32 actorUid;

	reader.Read(messageType);
	reader.Read(actorUid);

	const Actor::C_ActorHandle destinationActorHandle = GetActorHandle(actorUid);

	switch (messageType)
	{
	case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		{
			ActorStudioMsgShared::C_DragMsg data;

			F32 x = 0.f, y=0.f, z=0.f, w=0.f;

			reader.Read(x);
			reader.Read(y);
			reader.Read(z);
			data.m_Position = Maths::HVec3(x, y, z);

			reader.Read(x);
			reader.Read(y);
			reader.Read(z);
			reader.Read(w);
			data.m_Orientation = Maths::C_Quaternion(x, y, z, w);

			Maths::HMat43 worldMatrix = Maths::HMat33( data.m_Orientation ) || data.m_Position;;
			const ActorStudioMsgGame::C_DragMsg actorMsg(worldMatrix);

			//Maths::C_QuatTrans qt(data.m_Orientation, data.m_Position);
			//const ActorMsg::C_ActorSetTransformMsg actorMsg(qt);
			m_pActorList->SendMessageToActor(destinationActorHandle, &actorMsg);
		}
		break;

	case ActorStudioMsgShared::E_ActorStudioMsg_EndDrag: 
		{
			m_pActorList->SendMessageToActor(destinationActorHandle, messageType);
		}
		break;

	case ActorStudioMsgShared::E_ActorStudioMsg_Selected:
	case ActorStudioMsgShared::E_ActorStudioMsg_Deselected:
		{
			// there is no additional data for these types, just send the message straight to the actor
			m_pActorList->SendMessageToActor(destinationActorHandle, messageType);
		}
		break;

	default:
		// we don't know how to handle this message type
		break;
	}
}

Actor::C_ActorHandle C_ActorStudioMessaging::GetActorHandle(const U32 actorUid) const
{
	const XtType* pType = XtGetType<C_LevelActorCreationList>();

	if(m_pActorPack)
	{
		for (size_t i = 0; i < m_pActorPack->count; ++i)
		{
			XtRecord* pRecord = m_pActorPack->records[i].record;

			if ((pRecord != NULL) && (pRecord->type == pType)) // NOTE: there's an Xt function that tests for type inheritence that would be more future-proof, but the details escape me...
			{
				const C_LevelActorCreationList* pActorList = XtHandle<C_LevelActorCreationList>(pRecord);

				for (size_t j = 0; j < (size_t)pActorList->m_LevelActors.count; ++j)
				{
					const Actors::C_GameActorCreate* pActorCreate = pActorList->m_LevelActors[j];

					if ((pActorCreate != NULL) && !pActorCreate->m_IsCommented && (pActorCreate->GetSorceryUid() == actorUid))
					{
						return pActorCreate->GetActorHandle();
					}
				}
			}
		}
	}

	return Actor::C_ActorHandle();
}

}
