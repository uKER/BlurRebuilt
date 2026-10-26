#ifndef BUTTONMESSAGING_H
#define BUTTONMESSAGING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessagingDeclare.h>
#include <btl/Containers/containers.h>
#include <btl/allocators/pool.h>

#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

class C_ButtonController;

// Stored message
class C_DeferredUiMessageStore : public Btl::ListElementSingle<C_DeferredUiMessageStore>
{
public:
	inline C_DeferredUiMessageStore();
	inline ~C_DeferredUiMessageStore();

	inline void Init(const C_ButtonHandle &dest, const Messaging::C_MessageBase *pMessage);	///< Send message to specific button
	inline void Init(const U32 mask, const Messaging::C_MessageBase* pMessage); ///< Broadcase message

	inline const bool								IsBroadcast() const;
	inline const U32								GetBroadcastMask() const;
	inline const C_ButtonHandle&					GetDestButton() const;
	inline const Messaging::C_MessageBase*			GetMessage() const;

private:
	C_ButtonHandle	m_Dest;				///< if as U32 is zero then this is a broadcast, otherwise specifies the actor to receive the message
	U32				m_ComponentOrMask;	///< if IsBroadcast() then this is the broadcast mask, otherwise it is the component id
	U8*				m_pMessageData;		///< strong reference, copy of message object

	void SetMessage(const Messaging::C_MessageBase* pMessage); ///< copies the message data
};

typedef Btl::List< C_DeferredUiMessageStore, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter, Btl::ListElementSingle, Btl::StandardAllocator<Btl::NullType>, Btl::Internal> > C_DeferredUiMessageList; ///< list for storing C_DeferredUiMessageStore objects
typedef Btl::DynamicPool<C_DeferredUiMessageStore> C_DeferredUiMessagePool; 

// Stored message list
class C_DeferredUiMessageQueue
{
public:
	C_DeferredUiMessageQueue();
	~C_DeferredUiMessageQueue();

	inline const bool	PushMessage(const C_ButtonHandle dest, const Messaging::C_MessageBase* pMessage);
	inline const bool	BroadcastMessage(const Messaging::C_MessageBase* pMessage, const U32 mask = 0xffffffff);
	const U32			ProcessMessages(C_ButtonController* pController);

private:
	C_DeferredUiMessageList		m_MessageFIFO; ///< stores messages
	C_DeferredUiMessagePool		m_MessagePool; ///< allocates messages
};

}; // namespace UI

#define BUTTON_MESSAGING_INL
#include "ButtonMessaging.inl"
#undef BUTTON_MESSAGING_INL

#endif // BUTTONMESSAGING_H
