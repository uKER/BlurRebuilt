#ifndef ACTOR_STUDIO_MESSAGING_H
#define ACTOR_STUDIO_MESSAGING_H

#include <misc_shared/messaging/MessageValue.h>
#include <actor_studio_msg_shared/MessageTypes.h>
#include <actor_system/ActorList.h>

class ArteryConnection;
class ArteryMessage;
struct XtPack;

namespace ActorStudioMsgGame
{

typedef Messaging::C_MessageValue<Maths::HMat43, ActorStudioMsgShared::E_ActorStudioMsg_Drag> C_DragMsg;
					
class C_ActorStudioMessaging
{
public:
	static C_ActorStudioMessaging& Instance();
	void Init();
	void Exit();
	void Update();
	
	void SetActorList(Actor::C_ActorList* pActorList);
	void SetActorPack(const XtPack* pActorPack);

private:
	C_ActorStudioMessaging();
	C_ActorStudioMessaging(const C_ActorStudioMessaging&);
	~C_ActorStudioMessaging();

	Ut::Vector<ArteryConnection *> m_clients;
	MT::Mutex m_clientMutex;

	static void StaticNetProc(ArteryConnection con, void *self);
	void NetProc(ArteryConnection con);
	void ProcessConnection(ArteryConnection conn);
	void HandleMessage(ArteryConnection& conn, ArteryMessage& msg);
	Actor::C_ActorHandle GetActorHandle(const U32 actorUid) const;
	
	volatile U8 m_updateCount;

	Actor::C_ActorList*	m_pActorList;
	const XtPack*		m_pActorPack;
};

}

#endif //ACTOR_STUDIO_MESSAGING_H
