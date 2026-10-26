#ifndef ACTORMESSAGES_H
#define ACTORMESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <actor_system/ActorMessageHandle.h>
#include <actor_system/ActorMessageValue.h>
#include <misc_shared/messaging/MessageValue.h>
#include <misc_shared/UID/FourCC.h>


namespace Actor
{

/**
@brief Base Actor related messages.
@ingroup actor_group_messages

Type identifiers for base actor system messages.
*/
enum E_ActorMsg
{
	E_ActorMsg_None = FOURCC_HI20('GSAM'),	///< should never be encountered

	E_ActorMsg_GoToSleep,				///< Tell actor to go to sleep.
	E_ActorMsg_WakeUp,					///< Tell actor to wake up.
	E_ActorMsg_SleepHeartbeat,			///< Sent periodically to sleeping actors.
	
	E_ActorMsg_Kill,					///< Tell an actor to go kill itself.

	E_ActorMsg_AddSubscriber,			///< @see C_AddSubscriberMsg.
	E_ActorMsg_RemoveSubscriber,		///< @see C_RemoveSubscriberMsg.
	//E_ActorMsg_EnableSubscriber,		///< @see C_EnableSubscriberMsg.
	E_ActorMsg_SetSubscriberMask,		///< @see C_SetSubscriberMaskMsg.

	E_ActorMsg_SetUpdateMask,			///< @see C_SetUpdateMaskMsg.

	//E_ActorMsg_CreateComponent,		///< @see C_CreateComponentMsg.

	E_ActorMsg_ComponentPrimed,			///< @see C_ComponentPrimedMsg.
	E_ActorMsg_ComponentCreated,		///< @see C_ComponentCreatedMsg.
	E_ActorMsg_ActorComponentCreated,	///< @see C_ActorComponentCreatedMsg.

	E_ActorMsg_Created,					///< @see C_ActorCreatedMsg.
	E_ActorMsg_Killed,					///< @see C_ActorKilledMsg.

	// if you add more message types please also update RegisterMessageDebugInfo()

	E_ActorMsg_End, 
	E_ActorMsg_Begin = E_ActorMsg_None + 1, 
	E_ActorMsg_Count = E_ActorMsg_End - E_ActorMsg_Begin
};


/**
@brief Tell an actor to subscribe to a certain message.
@ingroup actor_group_messages
When the actor list broadcasts a message of the specified type, the actor will
handle it through the subscription.
@see C_RemoveSubscriberMsg C_EnableSubscriberMsg C_SetSubscriberMaskMsg.
*/
typedef Messaging::C_MessageValue<Messaging::TMsgType, E_ActorMsg_AddSubscriber> C_AddSubscriberMsg;


/**
@brief Tell an actor to unsubscribe to a certain message.
@ingroup actor_group_messages
When the actor list broadcasts a message of the specified type, the actor will
not handle it.
@see C_AddSubscriberMsg C_EnableSubscriberMsg C_SetSubscriberMaskMsg.
*/
typedef Messaging::C_MessageValue<Messaging::TMsgType, E_ActorMsg_RemoveSubscriber> C_RemoveSubscriberMsg;


/**
@brief Tell an actor to enable/disable subscription to a certain message.
@ingroup actor_group_messages
When the subscription is disabled the actor will not handle the message 
broadcast.
@see C_AddSubscriberMsg C_RemoveSubscriberMsg C_SetSubscriberMaskMsg.
*/
#if 0
// [MarkC] Enable Subscriber is being phased out
class C_EnableSubscriberMsg : public Messaging::C_MessageDerived<C_EnableSubscriberMsg, E_ActorMsg_EnableSubscriber>
{
public:
	Messaging::TMsgType		m_MessageType;		///< type identifier for the message to enable/disable subscription of
	bool					m_Enable;			///< true to enable, false to disable

	/** 
	@param messageType type identifier for the message to enable/disable subscription of.
	@param yn true to enable, false to disable
	**/
	explicit C_EnableSubscriberMsg(Messaging::TMsgType messageType, bool yn = true) : m_MessageType(messageType), m_Enable(yn) {}

};
#endif

/**
@brief Set the bitmask used to filter this message subscription.
@ingroup actor_group_messages
If the subscriber mask does not have at least one bit matching the broadcast
then the actor will not handle the message broadcast.
@see C_AddSubscriberMsg C_RemoveSubscriberMsg C_EnableSubscriberMsg.
*/
class C_SetSubscriberMaskMsg : public Messaging::C_MessageDerived<C_SetSubscriberMaskMsg, E_ActorMsg_SetSubscriberMask>
{
public:
	Messaging::TMsgType		m_MessageType;		///< type identifier for the message to mask subscription of
	U32						m_Mask;				///< bitmask to use

	/** 
	@param messageType type identifier for the message to mask subscription of.
	@param mask bitmask to use.
	*/
	C_SetSubscriberMaskMsg(Messaging::TMsgType messageType, U32 mask) : m_MessageType(messageType), m_Mask(mask) {}

};


/**
@brief Set the bitmask used to filter GameMessaging::E_GameMsg_Update messages.
@ingroup actor_group_messages
This is a shorthand for C_SetSubscriberMaskMsg using the update message type.
*/
typedef Messaging::C_MessageValue<U32, E_ActorMsg_SetUpdateMask> C_SetUpdateMaskMsg;


/**
@brief Message to create a component in the actor.
Sends a component creation object to add a new component to the receiver.
*/
//class C_CreateComponentMsg : public Messaging::C_MessageDerived<C_CreateComponentMsg, E_ActorMsg_CreateComponent>
//{
//public:
//	DataRef::C_Ptr<Component::I_ComponentCreator>	m_pCreator; ///< pointer to component creation data - must be permanent in memory!
//
//	/// @param creator stored by reference so must be permanent in memory.
//	explicit C_CreateComponentMsg(const Component::I_ComponentCreator& creator) : m_Creator(creator) {}
//
//};


/**
@brief Notifies the actor when a component has primed.
@ingroup actor_group_messages
This should be handled by C_Actor whilst in E_LifetimeState_Priming.
GetValue() returns the component interface type.
*/
typedef Messaging::C_MessageValue<Component::C_ComponentInterfaceId, E_ActorMsg_ComponentPrimed> C_ComponentPrimedMsg;


/**
@brief Notifies the actor when a component has created.
@ingroup actor_group_messages
Sent to the actor whenever a component is created for it.
GetValue() returns the component interface type.
*/
typedef Messaging::C_MessageValue<Component::C_ComponentInterfaceId, E_ActorMsg_ComponentCreated> C_ComponentCreatedMsg;


/**
@brief Broadcast whenever a component is created for an actor.
@ingroup actor_group_messages
GetActorHandle() returns the actor for which the component was created.
GetValue() returns the component interface type.
*/
typedef C_MessageActorValue<Component::C_ComponentInterfaceId, E_ActorMsg_ActorComponentCreated> C_ActorComponentCreatedMsg;


/**
@brief Broadcast after an actor is sucessfully created.
@ingroup actor_group_messages
GetActorHandle() gives the actor that was created.
*/
typedef C_MessageActorHandle<E_ActorMsg_Created> C_ActorCreatedMsg;


/**
@brief Broadcast before an actor is killed.
@ingroup actor_group_messages
GetActorHandle() gives the actor that will be killed.
*/
typedef C_MessageActorHandle<E_ActorMsg_Killed> C_ActorKilledMsg;


#ifndef CFG_GOLD

/// Registers debug info for message types in E_ActorMsg
extern void RegisterMessageDebugInfo();

#endif // ndef CFG_SPEED

} // namespace Actor


#endif
