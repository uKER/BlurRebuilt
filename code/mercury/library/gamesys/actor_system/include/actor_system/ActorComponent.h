#ifndef ACTORCOMPONENT_H
#define ACTORCOMPONENT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <actor_system/GameComponent.h>
#include <misc_shared/context/I_Anything.h>


namespace Actor
{

/// Actor component abstract base class interface.
class I_ActorComponent : public Component::I_GameComponent
{
	friend class C_Actor;

public:
	I_ActorComponent();
	virtual ~I_ActorComponent();

	C_ActorList*					GetActorList(void) const {return m_pActorList;}
	const C_ActorHandle				GetActorHandle(void) const {return m_ActorHandle;}
	const C_Actor*					GetActor(void) const;
	const I_ActorComponent*			QueryInterface(S32 interfaceID) const;	

	E_ActorMessageSendResult		SendMessageToActor(const C_ActorHandle dest, const Messaging::C_MessageBase *pMessage, S32 msgQueue = -1) const;
	inline E_ActorMessageSendResult	SendMessageToActor(const C_ActorHandle dest, Messaging::TMsgType msgType, S32 msgQueue = -1) const;
	inline E_ActorMessageSendResult	SendMessageToActor(const Messaging::C_MessageBase *pMessage, S32 msgQueue = -1) const;
	inline E_ActorMessageSendResult	SendMessageToActor(Messaging::TMsgType msgType, S32 msgQueue = -1) const;
	E_ActorMessageSendResult		SendMessageToComponent(const S32 component, const Messaging::C_MessageBase *pMessage, S32 msgQueue = -1) const;
	inline E_ActorMessageSendResult	SendMessageToComponent(const S32 component, Messaging::TMsgType msgType, S32 msgQueue = -1) const;

	static const Messaging::I_MessageTypeList* ClassActorMessageTypeList(); ///< Gets the list of message types that objects of this type want actors to forward to them.
	virtual const Messaging::I_MessageTypeList* GetActorMessageTypeList() const; ///< Gets the list of message types that this object wants the actor to forward to it.

	// from C_MessageHandler
	virtual bool					MessageHandler(const Messaging::C_MessageBase* pMessage) OVERRIDE SEALED; // from C_MessageHandler
	virtual bool					OnMessage(const Messaging::C_MessageBase* pMessage); ///< component message processing

	// Context related
	const Sys::I_Anything*			GetContext(const Sys::C_AnythingUid contextId) const;

	template <class T_Context>
	inline const T_Context*			GetContext() const {return static_cast<const T_Context*>(GetContext(T_Context::ClassUid()));}

#ifndef CFG_SPEED
	virtual void					DebugDumpMessageHandler(Debug::Logger& log, const char* prefix) const; // from Messaging::C_MessageHandler
#endif

protected:
	virtual void					SetPrimed(); // from Component::I_GameComponent

private:
	C_ActorHandle	m_ActorHandle;	///< handle of actor component belongs to
	C_ActorList*	m_pActorList;	///< pointer to actor list so that component can send messages and query interfaces

};

} // namespace Actor


/**
@brief Declares that actors should only dispatch certain message types.
Automation macro for filtering the message types that the actor forwards to a
component when a message is sent directly to an actor. By default a component
receives all such messages, but by overriding GetActorMessageTypeList() we can
limit the types of messages received.
@code
// in class definition we declare our intentions...
class C_MyComponentLeafClass : public C_MyComponentBaseClass
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()
};

// somewhere in the file where class methods are defined we list the message we want to be forwarded to the component...
// note that this file must include <misc_shared/Messaging/MessageTypeList.h>
ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_MyComponentLeafClass)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(GameMessaging::E_GameMsg_DebugRenderSpecific, GameMessaging::E_GameMsg_DebugRenderSpecific)
	ACTOR_COMPONENT_MESSAGE_TYPES_SINGLE(Actor::E_ActorMsg_GoToSleep)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_MyComponentLeafClass, C_MyComponentBaseClass)
@endcode
@see ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN ACTOR_COMPONENT_MESSAGE_TYPES_RANGE ACTOR_COMPONENT_MESSAGE_TYPES_SINGLE ACTOR_COMPONENT_MESSAGE_TYPES_END.
*/
#define ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE() \
	static const Messaging::I_MessageTypeList* ClassActorMessageTypeList(); \
	virtual const Messaging::I_MessageTypeList* GetActorMessageTypeList() const;

/**
@brief Begins the definition for ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE.
@param X_ComponentClass component type.
@see ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE ACTOR_COMPONENT_MESSAGE_TYPES_RANGE ACTOR_COMPONENT_MESSAGE_TYPES_SINGLE ACTOR_COMPONENT_MESSAGE_TYPES_END.
*/
#define ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(X_ComponentClass) \
	static const Messaging::C_MessageTypeRange g_##X_ComponentClass##_MessageTypes [] = \
	{ 

/**
@brief Adds a range of message types for ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE.
@param x_First first message type in range.
@param x_Last last message type in range.
@see ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN ACTOR_COMPONENT_MESSAGE_TYPES_SINGLE ACTOR_COMPONENT_MESSAGE_TYPES_END.
*/
#define ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(x_First, x_Last) \
		{x_First, x_Last}, 

/**
@brief Adds a single message type for ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE.
@param x_Type message type.
@see ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN ACTOR_COMPONENT_MESSAGE_TYPES_RANGE ACTOR_COMPONENT_MESSAGE_TYPES_END.
*/
#define ACTOR_COMPONENT_MESSAGE_TYPES_SINGLE(x_Type) \
		{x_Type, x_Type}, 

/**
@brief Ends the definition for ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE.
@param X_ComponentClass component type.
@param X_BaseClass component parent type.
@see ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN ACTOR_COMPONENT_MESSAGE_TYPES_RANGE ACTOR_COMPONENT_MESSAGE_TYPES_SINGLE.
*/
#define ACTOR_COMPONENT_MESSAGE_TYPES_END(X_ComponentClass, X_BaseClass) \
		{INT_MAX, INT_MIN } \
	}; \
	static const Messaging::C_MessageTypeListFixed g_##X_ComponentClass##_MessageTypeList(g_##X_ComponentClass##_MessageTypes, X_BaseClass::ClassActorMessageTypeList()); \
	const Messaging::I_MessageTypeList* X_ComponentClass::ClassActorMessageTypeList() {return &g_##X_ComponentClass##_MessageTypeList;} \
	const Messaging::I_MessageTypeList* X_ComponentClass::GetActorMessageTypeList() const {return &g_##X_ComponentClass##_MessageTypeList;}


#define ACTORCOMPONENT_INL
#include "ActorComponent.inl"
#undef ACTORCOMPONENT_INL

#endif
