#ifndef ACTOR_INL
#error Actor.inl should only be included from Actor.h
#endif

#include <misc_shared/Context/I_Anything.h>
#include <misc_shared/Messaging/MessageBase.h>


namespace Actor
{

/**
@brief Tests if this system flag is set.
@param flag system flag to test.
@returns true if the flag is set.
*/
inline const bool C_Actor::TestActorFlag(const E_ActorFlags flag) const
{
	return (m_ActorFlags & flag) == (U32)flag;
}

/**
@brief Sets or clears this system flag.
@param flag system flag to set.
@param value true to set or false to clear.
*/
inline void C_Actor::SetActorFlag(const E_ActorFlags flag, const bool value)
{
	if (value)
		m_ActorFlags |= flag;
	else
		m_ActorFlags &= ~flag;
}

template <class T_Context>
inline const T_Context* C_Actor::GetContext() const 
{
	const Sys::I_Anything* pContext = GetContext(T_Context::ClassUid());
	return (pContext == NULL) ? NULL : Sys::AnythingCast<T_Context>(pContext);
}

template <class T_ActorComponent>
inline const T_ActorComponent* C_Actor::QueryInterface() const
{
	const I_ActorComponent* pActorComponent = QueryInterface(T_ActorComponent::InterfaceType());
	return (pActorComponent == NULL) ? NULL : static_cast<const T_ActorComponent*>(pActorComponent);
}

template <class T_ActorComponent>
inline bool C_Actor::QueryInterface(const T_ActorComponent** ppComponent) const
{
	const T_ActorComponent* pActorComponent = QueryInterface<T_ActorComponent>();
	
	BIZ_ASSERT(ppComponent != NULL);
	*ppComponent = pActorComponent;
	
	return (pActorComponent != NULL);
}

inline U32 C_Actor::GetNumComponents() const
{
	return static_cast<U32>(m_ComponentInterfaces.Size());
}

inline const I_ActorComponent* C_Actor::GetComponent(const U32 index) const
{
	return (index < static_cast<U32>(m_ComponentInterfaces.Size())) ? m_ComponentInterfaces[index].second : NULL;
}

/**
@brief Sends a message to another actor.
@param dest actor to receive the message.
@param msgType type identifier for message, must not require additional data.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
inline E_ActorMessageSendResult C_Actor::SendMessageToActor(const C_ActorHandle dest, Messaging::TMsgType msgType, S32 msgQueue) const
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	return SendMessageToActor(dest, &message, msgQueue);
}

/**
@brief Sends a message to a component of this actor.
@param componentIf type identifier of the component to receive the message.
@param msgType type identifier for message, must not require additional data.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
inline E_ActorMessageSendResult C_Actor::SendMessageToComponent(Component::C_ComponentInterfaceId componentIf, Messaging::TMsgType msgType, S32 msgQueue) const
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	return SendMessageToComponent(componentIf, &message, msgQueue);
}

/**
@brief Broadcasts the message to all actors subscribed.
@param msgType type identifier for message, must not require additional data.
@param mask bitmask to filter subscribers with.
*/
inline void C_Actor::BroadcastMessage(Messaging::TMsgType msgType, U32 mask) const 
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(msgType));
	BroadcastMessage(&message, mask);
}

/**
@brief Actor message processing
@param msgType type identifier for message, must not require additional data.
*/
inline bool C_Actor::OnMessageSimple(const Messaging::TMsgType msgType)
{
	const Messaging::C_MessageBase& simpleMessage = Messaging::BuildSimpleMessage(msgType);
	return OnMessage(&simpleMessage);
}

/**
@brief Complex category filter.

Using this filter the caller can construct complex filters for actors based on
the user-defined catergory bits that have been assigned to the actor.
@code
enum E_MyCategory
{
	E_MyCategory_Animal = (1 << 0),
	E_MyCategory_Vegetable = (1 << 1), 
	E_MyCategory_Mineral = (1 << 2),
	E_MyCategory_Green = (1 << 3), 
	E_MyCategory_Soft = (1 << 4), 
	E_MyCategory_Flowering = (1 << 5), 
};

pFrogActor->SetCategory(E_MyCategory_Animal | E_MyCategory_Green | E_MyCategory_Soft);
pPlantActor->SetCategory(E_MyCategory_Vegetable | E_MyCategory_Green | E_MyCategory_Soft | E_MyCategory_Flowering);
pEmeraldActor->SetCategory(E_MyCategory_Mineral | E_MyCategory_Green);

const U32 matchAny = E_MyCategory_Animal | E_MyCategory_Vegetable | E_MyCategory_Mineral;
const U32 matchAll = E_MyCategory_Green | E_MyCategory_Soft;
const U32 matchNone = E_MyCategory_Flowering;

BIZ_ASSERT(pFrogActor->MatchCategory(matchAny, matchAll, matchNone)); // animal, green and soft, not flowering
BIZ_ASSERT(!pPlantActor->MatchCategory(matchAny, matchAll, matchNone)); // flowering!
BIZ_ASSERT(!pEmeraldActor->MatchCategory(matchAny, matchAll, matchNone)); // not soft!
@endcode

@param matchAnyCategory actor must be in at least one of these flagged categories.
@param matchAllCategory actor must be in all of these flagged categories.
@param matchNoneCategory actor must not be in any of these flagged categories.
@returns true if all conditions are passed.
*/
inline const bool C_Actor::MatchCategory(const U32 matchAnyCategory, const U32 matchAllCategory, const U32 matchNoneCategory) const
{
	const U32 categoryBits = m_ActorCategory;
	return ((categoryBits & matchAnyCategory) || (matchAnyCategory == 0)) && 
		((categoryBits & matchAllCategory) == matchAllCategory) && 
		((categoryBits & matchNoneCategory) == 0);
}

/**
@brief Should be called before iterating over m_ComponentIFMap.
*/
inline void C_Actor::LockComponentsDebug() const 
{
#ifdef DEBUG_COMPONENTS
	++m_ComponentsLockCount;
#endif
}

/**
@brief Should be called after iterating over m_ComponentIFMap.
*/
inline void C_Actor::UnlockComponentsDebug() const 
{
#ifdef DEBUG_COMPONENTS
	if( m_ComponentsLockCount == 0 )
	{
		BIZ_ASSERT_FAIL("unlock called without corresponding lock, or something REALLY weird is happening");
	}
	--m_ComponentsLockCount;
#endif
}

/**
@brief Should be called before modifying m_ComponentIFMap.
*/
inline void C_Actor::CheckUnlockedComponentsDebug() const 
{
#ifdef DEBUG_COMPONENTS
	if( m_ComponentsLockCount != 0 )
	{
		BIZ_ASSERT_FAIL("components are being created/destroyed whilst someone is iterating over the components, this is almost certainly going to cause something Really Bad to happen");
	}
#endif
}




/**
@brief Downcast an actor if it has the appropriate type identifier.
Note that this should only be used for casting to the final type of the actor, 
not for base classes from which the actor inherits.
@param pActor actor to cast.
@returns derived class pointer, or NULL if the actor has the wrong type.
*/
template <class DerivedActor> 
inline const DerivedActor* ActorCast_Safe(const C_Actor* pActor) 
{
	return ((pActor != NULL) && (pActor->GetActorTypeId() == DerivedActor::TypeId())) ? static_cast<const DerivedActor*>(pActor) : NULL;
}

/**
@brief Downcast an actor, asserting if it has the wrong type identifier.
Note that this should only be used for casting to the final type of the actor, 
not for base classes from which the actor inherits.
@param pActor actor to cast.
@returns derived class pointer.
*/
template <class DerivedActor> 
inline const DerivedActor* ActorCast_Assert(const C_Actor* pActor) 
{
	BIZ_ASSERT_MSG((pActor == NULL) || (pActor->GetActorTypeId() == DerivedActor::TypeId()), "wrong actor type"); 
	return static_cast<const DerivedActor*>(pActor);
}

/**
@brief Downcast an actor ignoring the type identifier.
Note that this should only be used for casting to the final type of the actor, 
not for base classes from which the actor inherits.
@param pActor actor to cast.
@returns derived class pointer.
*/
template <class DerivedActor> 
inline const DerivedActor* ActorCast_Reckless(const C_Actor* pActor) 
{
	return static_cast<const DerivedActor*>(pActor);
}

/**
@ingroup actor_group_main
@brief Downcast an actor.
Note that this should only be used for casting to the final type of the actor, 
not for base classes from which the actor inherits.
@param pActor actor to cast.
@returns derived class pointer.
*/
template <class DerivedActor> 
inline const DerivedActor* ActorCast(const C_Actor* pActor) 
{
	return ActorCast_Assert<DerivedActor>(pActor);
}

/**
@brief Downcast an actor if it has the appropriate type identifier.
Note that this should only be used for casting to the final type of the actor, 
not for base classes from which the actor inherits.
@param pActor actor to cast.
@returns derived class pointer, or NULL if the actor has the wrong type.
*/
template <class DerivedActor> 
inline DerivedActor* ActorCast_Safe(C_Actor* pActor) 
{
	return ((pActor != NULL) && (pActor->GetActorTypeId() == DerivedActor::TypeId())) ? static_cast<DerivedActor*>(pActor) : NULL;
}

/**
@brief Downcast an actor, asserting if it has the wrong type identifier.
Note that this should only be used for casting to the final type of the actor, 
not for base classes from which the actor inherits.
@param pActor actor to cast.
@returns derived class pointer.
*/
template <class DerivedActor> 
inline DerivedActor* ActorCast_Assert(C_Actor* pActor) 
{
	BIZ_ASSERT_MSG((pActor != NULL) && (pActor->GetActorTypeId() == DerivedActor::TypeId()), "wrong actor type"); 
	return static_cast<DerivedActor*>(pActor);
}

/**
@brief Downcast an actor ignoring the type identifier.
Note that this should only be used for casting to the final type of the actor, 
not for base classes from which the actor inherits.
@param pActor actor to cast.
@returns derived class pointer.
*/
template <class DerivedActor> 
inline DerivedActor* ActorCast_Reckless(C_Actor* pActor) 
{
	return static_cast<DerivedActor*>(pActor);
}

/**
@ingroup actor_group_main
@brief Downcast an actor.
Note that this should only be used for casting to the final type of the actor, 
not for base classes from which the actor inherits.
@param pActor actor to cast.
@returns derived class pointer.
*/
template <class DerivedActor> 
inline DerivedActor* ActorCast(C_Actor* pActor) 
{
	return ActorCast_Assert<DerivedActor>(pActor);
}

} // namespace Actor
