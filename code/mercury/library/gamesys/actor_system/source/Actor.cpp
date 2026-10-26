#include "ActorSystemPch.h"

#include <algorithm>

#include <actor_system/ActorFactory.h>
#include <actor_system/ActorPerf.h>
#include <actor_system/ActorSys.h>
#include <actor_system/GameComponentFactory.h>
#include <core/BizAssert.h>
#include <misc_shared/context/ContextProvider.h>
#include <misc_shared/messaging/GameMessaging.h>
#include <misc_shared/messaging/MessageTypeList.h>

#if defined(ACTOR_SYSTEM_OPTIMIZE_OFF)
#pragma optimize( "", off ) 
#endif

#ifdef CFG_DEBUG
int g_NoActors = 0;
#endif




struct C_ActorComponentMessageTypeHandler
{
	Messaging::C_MessageTypeRange	m_MsgTypeRange;
	Actor::I_ActorComponent*		m_pComponent;

	C_ActorComponentMessageTypeHandler() : m_MsgTypeRange(Messaging::kInvalidMessageTypeRange), m_pComponent(NULL) {}
	C_ActorComponentMessageTypeHandler(const Messaging::C_MessageTypeRange& range, Actor::I_ActorComponent* pComponent) : m_MsgTypeRange(range), m_pComponent(pComponent) {}
};




namespace Actor
{

/**
@class C_Actor
@ingroup actor_group_main
*/

C_Actor::C_Actor()
	: Util::C_HandleItem<C_Actor>()
	, Messaging::C_MessageHandler()
	, m_pActorCreateData()
	, m_LifetimeState(ActorSys::E_LifetimeState_Constructed)
	, m_Type(E_ActorTypeId_Null)
	, m_pActorList(NULL)
	, m_ActorFlags(0)
	, m_ActorCategory(0)
	, m_ActorGroup(0)
	, m_ComponentInterfaces()
	, m_SpecificMsgHandlers()
{
#ifdef CFG_DEBUG
	g_NoActors++;
#endif
#ifdef DEBUG_COMPONENTS
	m_ComponentsLockCount = 0;
#endif
}

C_Actor::~C_Actor()
{
	BIZ_ASSERT(GetHandle() == C_HandleType());

	// Destroy components:
	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it!= m_ComponentInterfaces.End(); ++it)
	{
		Component::C_GameComponentFactory::GetInstance()->DestroyComponent(it->second);
	}

#ifdef CFG_DEBUG
	g_NoActors--;
#endif
}

/**
@brief Called before the actor is initialised.
@param pCreationData creation data.
*/
void C_Actor::SetCreationData(const DataRef::C_Ptr<C_ActorCreationData>& pCreationData)
{
	BIZ_ASSERT(GetLifetimeState() == ActorSys::E_LifetimeState_Constructed);
	m_pActorCreateData = pCreationData;
}

/**
@brief Called to move the actor from the Constructed state.
@returns true if the actor was initialised.
*/
bool C_Actor::Initialise()
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetMemoryTracker());
#endif

	if (GetLifetimeState() != ActorSys::E_LifetimeState_Constructed)
		return false;

	if (InitialiseImpl())
	{
		BIZ_ASSERT(GetLifetimeState() == ActorSys::E_LifetimeState_Constructed);
		SetLifetimeState(ActorSys::E_LifetimeState_Initialised);
		return true;
	}
	else
	{
		BIZ_ASSERT(GetLifetimeState() == ActorSys::E_LifetimeState_Constructed);
		SetLifetimeState(ActorSys::E_LifetimeState_Failed);
		return false;
	}
}

/**
@brief Called to move the actor from the Constructed state.
@param stream provided by SaveComponentCreation.
@returns true if the actor was initialised.
*/
bool C_Actor::Initialise(Sys::C_ContextualStream& stream)
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetMemoryTracker());
#endif

	if (GetLifetimeState() != ActorSys::E_LifetimeState_Constructed)
		return false;

	DeserialiseForSave(stream);
	SetLifetimeState(ActorSys::E_LifetimeState_Initialised);
	return true;
}

/**
@brief Called to move the actor from the Constructed state.
Client actors can be enslaved to the server, skipping initialisation.
@returns false if the actor could not be enslaved.
*/
bool C_Actor::Enslave()
{
	if (GetLifetimeState() != ActorSys::E_LifetimeState_Constructed)
		return false;

	SetLifetimeState(ActorSys::E_LifetimeState_Initialised);
	return true;
}

/**
@brief Called to move the actor from the Initialised state.
Requests resources for loading.
@returns true if the actor was priming or primed.
*/
bool C_Actor::Prime()
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetMemoryTracker());
#endif

	if (GetLifetimeState() != ActorSys::E_LifetimeState_Initialised)
		return false;

	SetLifetimeState(ActorSys::E_LifetimeState_Priming);
	bool primed = true;

	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
	{
		if (!it->second->Prime())
		{
			LOGWARN(GetDebugLogLevelDesign(this), DBSTR"%s: component prime failed", GetActorName());
			SetLifetimeState(ActorSys::E_LifetimeState_Failed);
			return false;
		}

		if (it->second->GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
			primed = false;
	}

	if (!PrimeImpl())
	{
		SetLifetimeState(ActorSys::E_LifetimeState_Priming);
		SetActorFlag(E_ActorFlag_ActorPriming, true);
		primed = false;
	}

	if (primed)
		SetLifetimeState(ActorSys::E_LifetimeState_Primed);

	return true;
}

/**
@brief Called to move the actor from the Priming state.
Derived classes should call this is PrimeImpl returned false.
*/
void C_Actor::SetPrimed()
{
	BIZ_ASSERT(GetLifetimeState() == ActorSys::E_LifetimeState_Priming);
	SetActorFlag(E_ActorFlag_ActorPriming, false);
	PollForPrimed();
}

/**
@brief Called when the actor or a component becomes primed.
Checks if the actor and all of it's components are primed, and if so moves the
actor state from Priming to Primed.
*/
void C_Actor::PollForPrimed()
{
	BIZ_ASSERT(GetLifetimeState() == ActorSys::E_LifetimeState_Priming);

	// if the actor itself is still priming then bail
	if (TestActorFlag(E_ActorFlag_ActorPriming))
		return;

	// if any component is still priming them bail
	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
	{
		switch (it->second->GetLifetimeState())
		{
		case ActorSys::E_LifetimeState_Initialised:
		case ActorSys::E_LifetimeState_Priming:
			return;
		}
	}

	// we're all primed!
	SetLifetimeState(ActorSys::E_LifetimeState_Primed);
}

/**
@brief Called to move the actor from the Primed state.
@returns true if the actor was created.
*/
bool C_Actor::Create()
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetMemoryTracker());
#endif

	if (GetLifetimeState() != ActorSys::E_LifetimeState_Primed)
		return false;

	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
	{
		if (!it->second->Create())
		{
			SetLifetimeState(ActorSys::E_LifetimeState_Failed);
			return false;
		}
	}

	if (!CreateImpl())
	{
		SetLifetimeState(ActorSys::E_LifetimeState_Failed);
		return false;
	}

	SetLifetimeState(ActorSys::E_LifetimeState_Created);

	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
	{
		ComponentCreated(it->first);
	}

	return true;
}

/**
Derived actors can override this to perform one-time setup after the internal
dependencies of all components have been resolved.
*/
void C_Actor::InternalDependenciesCreated()
{
	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
	{
		it->second->InternalDependenciesCreated();
	}

	InternalDependenciesCreatedImpl();
}

/**
Derived actors can override this to perform one-time setup dependent upon 
other actors in the same creation list.
*/
void C_Actor::ExternalDependenciesCreated()
{
	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
	{
		it->second->ExternalDependenciesCreated();
	}

	ExternalDependenciesCreatedImpl();
}

/**
@brief Called to kill the actor.
*/
void C_Actor::Kill()
{
	if (GetLifetimeState() != ActorSys::E_LifetimeState_Killed)
	{
		const C_ActorKilledMsg message(GetHandle());
		m_pActorList->BroadcastMessage(&message);

		for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
		{
			it->second->Kill();
		}
		
		KillImpl();
		m_LifetimeState = ActorSys::E_LifetimeState_Killed;
		m_pActorList->AddKilledActor(this);
	}
}

/**
Derived classes should override this if they want to initialise components.
@returns true if successful.
*/
bool C_Actor::InitialiseImpl()
{
	return true;
}

/**
Derived classes should override this if they want to load resources.
@returns true if successful.
*/
bool C_Actor::PrimeImpl()
{
	return true;
}

/**
Derived classes should override this if they contain components or otherwise
need to register with the game world.
@returns true if successful.
*/
bool C_Actor::CreateImpl()
{
	if(FlagUpdate())
		AddSubscriber(GameMessaging::E_GameMsg_Update);

	return true;
}

/**
Derived classes should override this if they need to disable anything for the
short time before destruction.
*/
void C_Actor::KillImpl()
{
	// intentionally blank
}

/**
Derived classes can override this to perform processing when all components
in the actor have had InternalDependenciesCreated called.
*/
void C_Actor::InternalDependenciesCreatedImpl()
{
}

/**
Derived classes can override this to perform processing when all actors in the
same creation list have been created.
*/
void C_Actor::ExternalDependenciesCreatedImpl()
{
}

/**
Derived actor classes should handle any specific messages and pass on any
unhandled messages to their direct base class.
@code
class MyActorClass : public Actor::C_Actor
{
	bool MyActorClass::OnMessage(const Messaging::C_MessageBase* pMessage)
	{
		switch (pMessage->Type())
		{
		case E_MyMessage_Foo:
			return HandleFooMessage(Messaging::MessageCast<C_FooMessage>(pMessage));

		case E_MyMessage_Bar:
			return HandleBarMessage(Messaging::MessageCast<C_BarMessage>(pMessage));

		default:
			return Actor::C_Actor::OnMessage(pMessage);
		}
	}
};
@endcode
*/
bool C_Actor::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		// Kill the Actor
	case E_ActorMsg_Kill:
		Kill();
		break;

		// Add a subscriber
	case E_ActorMsg_AddSubscriber:
		AddSubscriber(Messaging::MessageCast<C_AddSubscriberMsg>(pMessage)->GetValue());
		break;

		// Remove a subscriber
	case E_ActorMsg_RemoveSubscriber:
		RemoveSubscriber(Messaging::MessageCast<C_RemoveSubscriberMsg>(pMessage)->GetValue());
		break;

		// Enable/Disable Subscriber
		// [MarkC] Removed - Enable Subscriber is being phased out
	/*
	case E_ActorMsg_EnableSubscriber:
		{
			const C_EnableSubscriberMsg *pEnableSubscriberMsg = Messaging::MessageCast<C_EnableSubscriberMsg>(pMessage);
			EnableSubscriber(pEnableSubscriberMsg->m_MessageType, pEnableSubscriberMsg->m_Enable);
			break;
		}
*/
		// Set the subscriber filter mask bits
	case E_ActorMsg_SetSubscriberMask:
		{
			const C_SetSubscriberMaskMsg *pSetSubscriberMaskMsg = Messaging::MessageCast<C_SetSubscriberMaskMsg>(pMessage);
			SetSubscriberMask(pSetSubscriberMaskMsg->m_MessageType, pSetSubscriberMaskMsg->m_Mask);
			break;
		}

		// Set the actor update mask - does this need to be a separate message?
	case E_ActorMsg_SetUpdateMask:
		SetSubscriberMask(GameMessaging::E_GameMsg_Update, Messaging::MessageCast<C_SetUpdateMaskMsg>(pMessage)->GetValue());
		break;

		// Create an Actor component
	//case E_ActorMsg_CreateComponent:
	//	CreateComponent(Messaging::MessageCast<C_CreateComponentMsg>(pMessage)->m_Creator);
	//	break;

	case E_ActorMsg_ComponentPrimed:
		if (GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
			PollForPrimed();
		break;

	case E_ActorMsg_ComponentCreated:
		// derived actors can handle this themselves to perform glue code on their components
		break;

	case GameMessaging::E_GameMsg_DebugRenderSpecific:
#ifndef CFG_GOLD
		DebugRender(Messaging::MessageCast<GameMessaging::C_DebugRenderSpecificMsg>(pMessage)->GetValue());
#endif
		break;

	case GameMessaging::E_GameMsg_DebugRenderOn:
		AddSubscriber(GameMessaging::E_GameMsg_DebugRenderSpecific);
		break;

	case GameMessaging::E_GameMsg_DebugRenderOff:
		RemoveSubscriber(GameMessaging::E_GameMsg_DebugRenderSpecific);
		break;

		// Message not handled - return false
	default:
		return false; 
	}

	return true;
}

bool C_Actor::MessageHandler(const Messaging::C_MessageBase* pMessage) 
{ 
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetMemoryTracker());
#endif
	
	ACTOR_PERF_EVENT_BEGIN(pMessage->Type(), this);

	const bool bResult = OnMessage(pMessage);

	ACTOR_PERF_EVENT_END();

	return bResult;
}


const char* C_Actor::GetActorName() const 
{ 
#ifdef ACTOR_NAME_DEBUG
	return m_Name.Chars();
#else
	return "NA in this build";
#endif
}

// const Maths::HMat43 C_Actor::GetCreationMatrix() const 
// {
// 	return /*(m_pActorCreateData) ? m_pActorCreateData->GetCreationMatrix() :*/ Maths::HMat43();
// }

/**
@brief Process the actor message.
Generally speaking, the MessageHandler() method should not be invoked directly
on actor objects. Instead the process message method should be called, which 
will call MessageHandler() on the components as well as the actor itself.
@param pMessage message to process, must not be NULL.
@param component type identifier of component interface to receive the message, 
or -1 for the actor and all components to receive the message.
*/
U32 C_Actor::ProcessMessage(const Messaging::C_MessageBase* pMessage, const S32 component)
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetMemoryTracker());
#endif

	ACTOR_PERF_EVENT_BEGIN(pMessage->Type(), this);

	U32 numHandlers = 0;

	if(component == -1)
	{
		bool msgHandled = false;

		if (!m_ComponentInterfaces.Empty())
		{
			// Iterate through components and pass them the message first:
			LockComponentsDebug();

			if (GetActorList()->GetComponentGenericMessageTypeList().Find(pMessage->Type()))
			{
				// this message type gets sent to all components regardless of what they asked for
				for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
				{
					++numHandlers;
					msgHandled |= it->second->OnMessage(pMessage);
				}
			}
			else
			{
				// send message to interested specific handlers
				for (C_MessageTypeComponentHandlers::ConstIterator it = m_SpecificMsgHandlers.Begin(); it != m_SpecificMsgHandlers.End(); ++it)
				{
					if (Messaging::Contains(it->m_MsgTypeRange, pMessage->Type()))
					{
						++numHandlers;
						msgHandled |= it->m_pComponent->OnMessage(pMessage);
					}
				}
			}

			UnlockComponentsDebug();
		}

		// Call actor message handler
		msgHandled |= OnMessage(pMessage);
		++numHandlers;

		if (!msgHandled) // Component may have handled msg.
		{
#ifndef CFG_SPEED
			LOGERROR(GetDebugLogLevelDesign(this), "'%s' failed to handle message '%s' (type 0x%8x)", GetActorName(), pMessage->GetDebugName(), pMessage->Type());
#endif
		}
	}
	else	// send message exclusively to component
	{
		C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin();

		while (it != m_ComponentInterfaces.End())
		{
			if (it->first == component)
			{
				break;
			}
			++it;
		}

		if (it != m_ComponentInterfaces.End())
		{
			++numHandlers;

			if (!it->second->OnMessage(pMessage))
			{
#ifndef CFG_SPEED
				LOGERROR(GetDebugLogGeneral(), DBSTR"'%s' failed to handle component(%s) message '%s' (0x%8x)", GetActorName(), Component::C_GameComponentFactory::GetInstance()->GetComponentIFName(component).Chars(), pMessage->GetDebugName(), pMessage->Type());
#endif
			}
		}
		else
		{
#ifndef CFG_GOLD
			LOGWARN(GetDebugLogGeneral(), DBSTR"'%s': component interface (%s) not found (msg:%s)", GetActorName(), Component::C_GameComponentFactory::GetInstance()->GetComponentIFName(component).Chars(), pMessage->GetDebugName());
#endif
		}
	}

	ACTOR_PERF_EVENT_END();

	return numHandlers;
}

/**
@brief Sends a message to another actor.
@param dest actor to receive the message.
@param pMessage message to send, must not be NULL.
@param msgQueue user-define message queue index.
@returns result of sending.
*/
E_ActorMessageSendResult C_Actor::SendMessageToActor(const C_ActorHandle dest, const Messaging::C_MessageBase* pMessage,S32 msgQueue) const
{
	return GetActorList()->SendMessageToActor(dest, pMessage, msgQueue);
}

/**
@brief Sends a message to a component of this actor.
@param component type identifier of component interface to receive the message.
@param pMessage message to send, must not be NULL.
@param msgQueue user-defined message queue index.
@returns result of sending.
*/
E_ActorMessageSendResult C_Actor::SendMessageToComponent(Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage, S32 msgQueue) const
{
	return GetActorList()->SendMessageToActorComponent(GetHandle(), component, pMessage, msgQueue);
}

/**
@brief Broadcasts the message to all subscribed actors.
@param pMessage message to send, must not be NULL.
@param mask bitmask to filter subscribers with.
*/
void C_Actor::BroadcastMessage(const Messaging::C_MessageBase *pMessage, U32 mask) const
{
	GetActorList()->BroadcastMessage(pMessage, mask);
}

/**
Actors that have a world-space position should override this method.
@returns world-space position of the actor.
*/
const Maths::C_Vertex C_Actor::GetPosition(void) const 
{
	return GetMatrix().AW();
}

/**
Actors that have a world-space direction should override this method.
@returns world-space direction of the actor.
*/
const Maths::C_Vector C_Actor::GetDirection(void) const
{
	return GetMatrix().AZ();
}

/**
Actors that have a world-space transformation should override this method.
@returns world-space transformation matrix of the actor.
*/
const Maths::HMat43& C_Actor::GetMatrix(void) const 
{ 
	// TODO: use const from maths
	static Maths::HMat43 mat; 
	return mat;
}

/**
@brief Obtains a component interface from the actor.
If the actor has a component that implements the specified interface then the
interface will be returned.
@param interfaceID type identifier for requested interface.
@returns NULL if the interface is not supported.
*/
const I_ActorComponent* C_Actor::QueryInterface(const Component::C_ComponentInterfaceId interfaceID) const
{
//	if (GetLifetimeState() != ActorSys::E_LifetimeState_Killed)
	{
		for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
		{
			if (it->first == interfaceID)
			{
				return it->second;
			}
		}
	}

	return NULL;
}

/**
@brief Called before initialising a component.
@param pCreationData component creation data.
@returns NULL if the component could not be created.
*/
I_ActorComponent* C_Actor::PreInitialiseComponent(const DataRef::C_Ptr<Component::I_ComponentCreator>& pCreationData)
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetMemoryTracker());
#endif

	if (pCreationData.IsNull())
		return NULL;

	Component::C_GameComponentFactory* pComponentFactory = Component::C_GameComponentFactory::GetInstance();
	I_ActorComponent* pComponent = static_cast<I_ActorComponent*>(pComponentFactory->CreateComponent(pCreationData->GetComponentTypeID()));

	if (pComponent == NULL)
	{
		LOGERROR(GetDebugLogLevelDesign(this), "%s: component construction (type %d) failed", GetActorName(), pCreationData->GetComponentTypeID());
		return NULL;
	}

	if (!AddComponent(*pComponent))
	{
		LOGERROR(GetDebugLogLevelDesign(this), "%s: component add (type %d) failed", GetActorName(), pComponent->GetComponentTypeID());
		Component::C_GameComponentFactory::GetInstance()->DestroyComponent(pComponent); // destroy component
		return NULL;
	}

	return pComponent;
}

/**
@brief Called after initialising a component.
@param pComponent component object.
@param initialised true if the component was successfully initialised.
@returns NULL if the component was not initialised.
*/
I_ActorComponent* C_Actor::PostInitialiseComponent(I_ActorComponent* pComponent, const bool initialised)
{
	if (!initialised)
	{
		LOGERROR(GetDebugLogLevelDesign(this), "%s: component initialisation (type %d) failed", GetActorName(), pComponent->GetComponentTypeID());
		RemoveComponent(pComponent->GetInterfaceID());
		return NULL;
	}

	return pComponent;
}

/**
Constructs and initialises a component.
@param pCreationData data to create the component from.
@returns initialised component, or NULL if initialisation failed.
*/
I_ActorComponent* C_Actor::InitialiseComponent(const DataRef::C_Ptr<Component::I_ComponentCreator>& pCreationData)
{
	I_ActorComponent* pComponent = PreInitialiseComponent(pCreationData);

	if (pComponent == NULL)
		return NULL;

	return PostInitialiseComponent(pComponent, pComponent->Initialise(pCreationData));
}

/**
@brief Constructs and initialises a component using Sorcery creation data.
@param pCreationData creation data supplied by Sorcery.
@returns NULL if the component could not be initialised.
*/
I_ActorComponent* C_Actor::InitialiseComponentFromSorcery(const Component::I_ComponentCreator* pCreationData)
{
	const Component::I_ComponentCreator::C_DataRef dataRef = DataRef::SorceryLookup(pCreationData);

	if (dataRef.IsNull())
	{
		LOGERROR(GetDebugLogLevelDesign(this), "%s: component Sorcery data not registered", GetActorName());
		return NULL;
	}

	return InitialiseComponent(dataRef);
}

/**
@brief Creates a primed component.
@param interfaceId interface of primed component.
@returns true if the component is created.
*/
bool C_Actor::CreateComponent(const Component::C_ComponentInterfaceId interfaceId)
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetMemoryTracker());
#endif

	C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin();

	while (it != m_ComponentInterfaces.End())
	{
		if (it->first == interfaceId)
		{
			break;
		}
		++it;
	}

	if (it == m_ComponentInterfaces.End())
		return false;

	if (!it->second->Create())
	{
		LOGERROR(GetDebugLogLevelDesign(this), "%s: component creation (type %d) failed", GetActorName(), it->second->GetComponentTypeID());
		RemoveComponent(interfaceId);
		return false;
	}

	ComponentCreated(it->first);
	return true;
}

/**
@brief called when a component is created for this actor.
@param interfaceId interface id for created component.
*/
void C_Actor::ComponentCreated(const Component::C_ComponentInterfaceId interfaceId)
{
	// send message to self
	{
		const C_ComponentCreatedMsg message(interfaceId);
		SendMessageToActor(GetHandle(), &message);
	}

	// broadcast message
	{
		const C_ActorComponentCreatedMsg message(GetHandle(), interfaceId);
		BroadcastMessage(&message);
	}
}

/**
@brief Creates a component inside the actor.
@param pCreator factory data to create the component with.
@returns true if the component was created.
*/
bool C_Actor::SpawnComponent(const DataRef::C_Ptr<Component::I_ComponentCreator>& pCreator)
{
	I_ActorComponent* pComponent = InitialiseComponent(pCreator);

	if (pComponent == NULL)
		return false;

	if (!pComponent->Prime())
	{
		LOGERROR(GetDebugLogLevelDesign(this), "%s: component priming (type %d) failed", GetActorName(), pComponent->GetComponentTypeID());
		RemoveComponent(pComponent->GetInterfaceID());
		return false;
	}

	// TODO: [Oscar] wait for component resources to finish loading
	//while (pComponent->GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
		// Sleep or something
	BIZ_ASSERT(pComponent->GetLifetimeState() == ActorSys::E_LifetimeState_Primed);

	if (!CreateComponent(pComponent->GetInterfaceID()))
		return false;

	return true;
}

/**
@brief Add a component to the actor.
@param component component to add.
@returns true if the actor doesn't already have this component type.
*/
bool C_Actor::AddComponent(I_ActorComponent& component)
{
#if ACTOR_SYSTEM_MEMORY_TRACKING
	const ActorSys::C_MemoryTracker memoryTracker(GetMemoryTracker());
#endif

	CheckUnlockedComponentsDebug(); // if you hit a break-point here, then we're trying to add a component while iterating over all our components, which is not a safe thing to do

	S32 interfaceID = component.GetInterfaceID();

	if (QueryInterface(interfaceID) == NULL)	// check if component already exists for that interface
	{
		component.SetSubscriberSystem(&GetSubscriberSystem());
		component.m_ActorHandle = GetHandle();
		component.m_pActorList = GetActorList();
		m_ComponentInterfaces.PushBack(C_ComponentInterface(interfaceID, &component));
		
		// add message handlers
		{
			const Messaging::I_MessageTypeList* pMessageTypeList = component.GetActorMessageTypeList();

			if (pMessageTypeList == NULL)
			{
				// add whole range of message
				m_SpecificMsgHandlers.PushBack(C_ActorComponentMessageTypeHandler(Messaging::kFullMessageTypeRange, &component));
			}
			else
			{
				// get specific message types to handle
				Messaging::C_MessageTypeListAccumulator visitor(pMessageTypeList->CalcNumRanges());
				pMessageTypeList->Visit(visitor);

				// record handlers
				m_SpecificMsgHandlers.Reserve(m_SpecificMsgHandlers.Size() + visitor.GetResults().Size());

				for (Messaging::C_MessageTypeRanges::ConstIterator it = visitor.GetResults().Begin(); it != visitor.GetResults().End(); ++it)
				{
					m_SpecificMsgHandlers.PushBack(C_ActorComponentMessageTypeHandler(*it, &component));
				}
			}
		}

		return true;
	}
	else
	{
		LOGERROR(GetDebugLogLevelDesign(this), "%s: failed to add component (type %d) : Component with that interface already exists", GetActorName(), component.GetComponentTypeID());
		return false;
	}
}

/**
@brief Remove a component from an actor.
@param interfaceID type identifier for the component to remove.
@returns true if the component was removed.
*/
bool C_Actor::RemoveComponent(const Component::C_ComponentInterfaceId interfaceID)
{
	CheckUnlockedComponentsDebug(); // if you hit a break-point here, then we're trying to remove a component while iterating over all our components, which is not a safe thing to do

	C_ComponentInterfaces::Iterator it = m_ComponentInterfaces.Begin();

	while (it != m_ComponentInterfaces.End())
	{
		if (it->first == interfaceID)
		{
			break;
		}
		++it;
	}

	if (it != m_ComponentInterfaces.End())
	{
		I_ActorComponent* pComponent = it->second;

		// remove message handlers
		{
			S32 first = 0;

			while (first < m_SpecificMsgHandlers.Size())
			{
				if (m_SpecificMsgHandlers[first].m_pComponent == pComponent)
				{
					break;
				}

				++first;
			}

			if (first < m_SpecificMsgHandlers.Size())
			{
				S32 last = first + 1;

				while (last < m_SpecificMsgHandlers.Size())
				{
					if (m_SpecificMsgHandlers[last].m_pComponent != pComponent)
					{
						break;
					}
					++last;
				}

				const S32 clipCount = last - first;
				const S32 keepCount = m_SpecificMsgHandlers.Size() - last;

				for (S32 keep = 0; keep < keepCount; ++keep)
				{
					m_SpecificMsgHandlers[first + keep] = m_SpecificMsgHandlers[first + keep + clipCount];
				}

				m_SpecificMsgHandlers.Resize(first + keepCount);
			}
		}

		// remove the component interface
		m_ComponentInterfaces.Erase(it);

		// destroy the component
		if (pComponent != NULL)
		{
			Component::C_GameComponentFactory::GetInstance()->DestroyComponent(pComponent);
		}

		return true;
	}

	return false;
}

/**
@brief Serialises actor data for saving.
@param stream receives data.
*/
void C_Actor::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	// send the number of components
	const U32 count = m_ComponentInterfaces.Size();
	stream << count;

	// send component data
	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
	{
#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
		const Sys::C_ContextualStreamPos startPos = stream.GetStreamPos();
#endif

		stream << it->second->GetCreator();

#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
		const Sys::C_ContextualStreamPos midPos = stream.GetStreamPos();
		const Sys::C_ContextualStreamLen creationLen = midPos - startPos;
		stream << creationLen;
#endif

		it->second->SerialiseForSave(stream);

#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
		const Sys::C_ContextualStreamPos endPos = stream.GetStreamPos();
		const Sys::C_ContextualStreamLen componentLen = endPos - midPos;
		stream << componentLen;
#endif
	}

	// send actor data
	stream << m_ActorFlags;
	stream << m_ActorCategory;
}

/**
@brief Deserialises actor data for saving.
@param stream provides data.
*/
void C_Actor::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	// read the number of components
	U32 count = 0;
	stream >> count;

	// read each component
	for (U32 i = 0; i < count; ++i)
	{
#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
		const Sys::C_ContextualStreamPos startPos = stream.GetStreamPos();
#endif

		// read the creation data
		Component::I_ComponentCreator::C_DataRef pCreator;
		stream >> pCreator;

#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
		const Sys::C_ContextualStreamPos midPos = stream.GetStreamPos();
		const Sys::C_ContextualStreamLen calculatedCreationLen = midPos - startPos;

		Sys::C_ContextualStreamLen serialisedCreationLen;
		stream >> serialisedCreationLen;

		if (!Sys::CompareStreamLen(calculatedCreationLen, serialisedCreationLen))
		{
			Btl::String interfaceName = "<unknown>";

			if (!pCreator.IsNull())
			{
				const Component::C_GameComponentFactory* pComponentFactory = Component::C_GameComponentFactory::GetInstance();
				interfaceName = pComponentFactory->GetComponentIFName(pComponentFactory->GetComponentInterface(pCreator->GetComponentTypeID()));
			}

			LOGERROR(GetDebugLogGeneral(), "creation data for component %s of actor '%s' has wrong size", interfaceName.Chars(), GetActorName());
		}
#endif

		// initialise the component
		I_ActorComponent* pComponent = PreInitialiseComponent(pCreator);

		if (pComponent != NULL)
			PostInitialiseComponent(pComponent, pComponent->Initialise(pCreator, stream));

#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
		const Sys::C_ContextualStreamPos endPos = stream.GetStreamPos();
		const Sys::C_ContextualStreamLen calculatedComponentLen = endPos - midPos;

		Sys::C_ContextualStreamLen serialisedComponentLen;
		stream >> serialisedComponentLen;

		if (!Sys::CompareStreamLen(calculatedComponentLen, serialisedComponentLen))
		{
			Btl::String interfaceName = "<unknown>";

			if (!pCreator.IsNull())
			{
				const Component::C_GameComponentFactory* pComponentFactory = Component::C_GameComponentFactory::GetInstance();
				interfaceName = pComponentFactory->GetComponentIFName(pComponentFactory->GetComponentInterface(pCreator->GetComponentTypeID()));
			}

			LOGERROR(Actor::GetDebugLogGeneral(), "serialised data for component %s of actor '%s' has wrong size", interfaceName.Chars(), GetActorName());
		}
#endif
	}

	// read actor data
	stream >> m_ActorFlags;
	stream >> m_ActorCategory;
}

/**
@brief Serialises actor data for synchronising.
@param stream receives data.
*/
void C_Actor::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	/*
	stream << m_ActorFlags;
	stream << m_ActorCategory;
	*/
}

/**
@brief Serialises actor data for synchronising.
@param stream provides data.
*/
void C_Actor::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	/*
	U32 newFlags;
	stream >> newFlags;
	if(newFlags != m_ActorFlags)
	{
		const bool oldUpdate = FlagUpdate();
		const bool newUpdate = ((newFlags & E_ActorFlag_Update) != 0);

		if (oldUpdate && !newUpdate)
			RemoveSubscriber(GameMessaging::E_GameMsg_Update);
		else if(!oldUpdate && newUpdate)
			AddSubscriber(GameMessaging::E_GameMsg_Update);

		m_ActorFlags = newFlags;
	}

	stream >> m_ActorCategory;
	*/
}

/**
Saves component creation date for use by InitialiseComponents.
@param stream stream to receive components.
*/
void C_Actor::SerialiseComponentCreation(Sys::C_ContextualStream& stream) const
{
	// send the number of components
	const U32 count = m_ComponentInterfaces.Size();
	stream << count;

	// send creation data
	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
	{
		stream << it->second->GetCreator();
	}
}

/**
Saves component data for loading wih LoadComponentData.
@param stream stream to receive components.
*/
void C_Actor::SerialiseComponentsForSync(Sys::C_ContextualStream& stream) const
{
	// send the number of components
	const U32 count = m_ComponentInterfaces.Size();
	stream << count;

	// send IID and contents
	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
	{
		stream << it->first;
		it->second->SerialiseForSync(stream);
	}
}

/**
Loads component data saved with SaveComponentData.
@param stream stream to receive components.
*/
void C_Actor::DeserialiseComponentForSync(Sys::C_ContextualStream& stream) const
{
	// read the number of components
	U32 count = 0;
	stream >> count;

	// read each component
	for (U32 i = 0; i < count; ++i)
	{
		// read the IID
		Component::C_ComponentInterfaceId iid = -1;
		stream >> iid;

		// get the corresponding component
		C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin();

		while (it != m_ComponentInterfaces.End())
		{
			if (it->first == iid)
			{
				break;
			}
			++it;
		}

		if (it != m_ComponentInterfaces.End())
		{
			it->second->DeserialiseForSync(stream);
		}
		else
		{
			LOGERROR(GetDebugLogGeneral(), "don't know how to deserialise actor component, get a game programmer to investigate");
		}
	}
}

/**
@brief Requests the specified context.
@param contextId context to request.
@returns NULL if the context is not supported.
*/
const Sys::I_Anything* C_Actor::GetContext(const Sys::C_AnythingUid contextId) const
{
	const Sys::I_ContextProvider* pContextProvider = GetActorList()->GetDefaultContextProvider();
	return (pContextProvider != NULL) ? pContextProvider->GetContext(contextId) : NULL;
}

/**
@brief Gets a const actor pointer from an actor handle.
@param actorHandle handle of actor to get pointer to.
@returns NULL if the actor does not exist in the same actor list.
*/
const C_Actor* C_Actor::GetActorPtr(const C_ActorHandle actorHandle) const
{
	return GetActorList()->GetActorPtr(actorHandle);
}


#ifndef CFG_GOLD

/**
@brief Calls DebugRender() on actor and on all components.
@param debugRenderPtr debug render context.
*/
void C_Actor::DebugRenderSelfAndComponents(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	DebugRender(debugRenderPtr);

	for (C_ComponentInterfaces::ConstIterator it = m_ComponentInterfaces.Begin(); it != m_ComponentInterfaces.End(); ++it)
	{
		it->second->DebugRender(debugRenderPtr);
	}
}

/**
@brief Renders debug for the actor.

Derived actor classes can override this method but should forward the call to
their direct base class as well.
@code
class MyActorClass : public C_MyGameActorClass
{
public:
#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
	{
		m_ThingamyJig->DebugREnder(debugRenderPtr);
		C_MyGameActorClass::DebugRender(debugRenderPtr);
	}
#endif
};
@endcode

@param debugRenderPtr debug render context.
*/
void C_Actor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
}

#endif // ndef CFG_GOLD

#ifndef CFG_SPEED

void C_Actor::DebugDumpMessageHandler(Debug::Logger& log, const char* prefix) const
{
	LOGINFO(log, "%sActor: %s     %s", prefix, GetActorList()->GetActorFactory()->LookupName(GetActorTypeId()).Chars(), GetActorName());
}

#endif

} // namespace Actor


#if defined(ACTOR_SYSTEM_OPTIMIZE_OFF)
#pragma optimize( "", on ) 
#endif
