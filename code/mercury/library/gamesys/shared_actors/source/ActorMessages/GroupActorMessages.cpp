#include "SharedActorsPch.h"

#include <shared_actors/actormessages/GroupActorMessages.h>

#include <shared_actors/XtActorMessages.h>
#include <shared_actors/XtSharedLevelActorParams.h>





namespace ActorMsg
{

/// Exposes C_GroupAddActorMsg to ActorStudio.
class C_XtGroupAddActorMsg : public C_XtActorMessageBase
{
public:
	Actors::C_XtParamActor* m_pActor;

	C_XtGroupAddActorMsg() : m_pActor(NULL)
	{
	}

	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext)
	{
		m_Message.SetActorHandle((m_pActor != NULL) ? m_pActor->GetActorHandle(pActorContext) : Actor::C_ActorHandle());
		return &m_Message;
	}

private:
	C_GroupAddActorMsg m_Message;

};


/// Exposes C_GroupRemoveActorMsg to ActorStudio.
class C_XtGroupRemoveActorMsg : public C_XtActorMessageBase
{
public:
	Actors::C_XtParamActor* m_pActor;

	C_XtGroupRemoveActorMsg() : m_pActor(NULL)
	{
	}

	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext)
	{
		m_Message.SetActorHandle((m_pActor != NULL) ? m_pActor->GetActorHandle(pActorContext) : Actor::C_ActorHandle());
		return &m_Message;
	}

private:
	C_GroupRemoveActorMsg m_Message;

};

/// Exposes C_GroupInheritMsg to ActorStudio.
class C_XtGroupInheritMsg : public C_XtActorMessageBase
{
public:
	Actors::C_XtParamActor* m_pParent;
	Actors::C_XtParamActor* m_pChild;

	C_XtGroupInheritMsg() : m_pParent(NULL), m_pChild(NULL)
	{
	}

	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext)
	{
		m_Message.SetActorHandle((m_pParent != NULL) ? m_pParent->GetActorHandle(pActorContext) : Actor::C_ActorHandle());
		m_Message.SetValue((m_pChild != NULL) ? m_pChild->GetActorHandle(pActorContext) : Actor::C_ActorHandle());
		return &m_Message;
	}
private:
	C_GroupInheritMsg m_Message;

};


/// Exposes E_GroupMsg_EnableMessages to ActorStudio.
class C_XtGroupEnableMessagesMsg : public C_XtGenericMessage
{
public:
	C_XtGroupEnableMessagesMsg() :  C_XtGenericMessage(E_GroupMsg_EnableMessages)
	{
	}


};


/// Exposes E_GroupMsg_DisableMessages to ActorStudio.
class C_XtGroupDisableMessagesMsg : public C_XtGenericMessage
{
public:
	C_XtGroupDisableMessagesMsg() : C_XtGenericMessage(E_GroupMsg_DisableMessages)
	{

	}

};



/// Exposes E_GroupMsg_CreateActorList to ActorStudio.
class C_XtGroupCreateActorListMsg : public C_XtGenericMessage
{
public:
	C_XtGroupCreateActorListMsg(): C_XtGenericMessage(E_GroupMsg_CreateActorList)
	{

	}

};

/// Exposes E_GroupMsg_DeleteActorsMessages to ActorStudio.
class C_XtGroupDeleteActorsMsg : public C_XtGenericMessage
{
public:
	C_XtGroupDeleteActorsMsg(): C_XtGenericMessage(E_GroupMsg_DeleteActors)
	{
	}

};

/// Exposes C_GroupSetForwardingVolumeMsg to ActorStudio.
class C_XtGroupSetForwardingVolumeMsg : public C_XtActorMessageBase
{
public:
	Actors::C_XtParamActor* m_pActor;

	C_XtGroupSetForwardingVolumeMsg() : m_pActor(NULL)
	{
	}

	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext)
	{
		m_Message.SetActorHandle((m_pActor != NULL) ? m_pActor->GetActorHandle(pActorContext) : Actor::C_ActorHandle());
		return &m_Message;
	}

private:
	C_GroupSetForwardingVolumeMsg m_Message;

};

} // namespace ActorMsg

XT_TYPE(ActorMsg::C_XtGroupAddActorMsg, "GroupAddActorMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_pActor, "ActorToAdd")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtGroupRemoveActorMsg, "GroupRemoveActorMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_pActor, "ActorToRemove")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtGroupInheritMsg, "GroupInheritMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_pParent, "ParentActor")
XT_FIELD(m_pChild, "ChildActor")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtGroupEnableMessagesMsg, "GroupEnableMessagesMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtGroupDisableMessagesMsg, "GroupDisableMessagesMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtGroupCreateActorListMsg, "GroupCreateActorListMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtGroupDeleteActorsMsg, "GroupDeleteActorsMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtGroupSetForwardingVolumeMsg, "GroupSetForwardingVolumeMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_pActor, "VolumeActor")
XT_END_TYPE


XT_LIBRARY(GroupActorMessages)
XT_REGISTER_TYPE(ActorMsg::C_XtGroupAddActorMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtGroupRemoveActorMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtGroupInheritMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtGroupEnableMessagesMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtGroupDisableMessagesMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtGroupCreateActorListMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtGroupDeleteActorsMsg)
XT_END_LIBRARY


