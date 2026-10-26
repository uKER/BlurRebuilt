#include "ActorSystemPch.h"

#include <actor_system/ActorSys.h>
#include <actor_system/ActorFactory.h>
#include <actor_system/ActorCreation.h>
#include <actor_system/ActorCreationList.h>
#include <actor_system/ActorHandleListItem.h>
#include <actor_system/ActorMessaging.h>
#include <actor_system/ActorPerf.h>
#include <actor_system/ActorSerialisation.h>
#include <actor_system/GameComponentFactory.h>
#include <actor_system/StaggerManager.h>
#include <btl/containers/algorithm.h>
#include <misc_shared/DataRef/Factory.h>
#include <misc_shared/Messaging/MessageStats.h>
#include <misc_shared/Messaging/MessageSubscriberSystem.h>
#include <misc_shared/Messaging/MessageTypeList.h>

#if defined(ACTOR_SYSTEM_OPTIMIZE_OFF)
#pragma optimize( "", off ) 
#endif


enum E_DebugDumpLevel
{
	E_DebugDumpLevel_Count, 
	E_DebugDumpLevel_Name, 
	E_DebugDumpLevel_Detail,
};



Debug::Bool gActorDebugLogActorListSize("ActorDebug.LogActorlistSize", false);

#ifndef CFG_GOLD
Debug::Function g_DebugActorListDumpActorCounts("ActorDebug.DumpActorCounts", Actor::C_ActorList::DebugDumpActors, (void*)E_DebugDumpLevel_Count);
Debug::Function g_DebugActorListDumpActorNames("ActorDebug.DumpActorNames", Actor::C_ActorList::DebugDumpActors, (void*)E_DebugDumpLevel_Name);
Debug::Function g_DebugActorListDumpActorDetails("ActorDebug.DumpActorDetails", Actor::C_ActorList::DebugDumpActors, (void*)E_DebugDumpLevel_Detail);

Debug::Function g_DebugActorListDumpComponentCounts("ActorDebug.DumpComponentCounts", Actor::C_ActorList::DebugDumpComponents, (void*)E_DebugDumpLevel_Count);
Debug::Function g_DebugActorListDumpComponentNames("ActorDebug.DumpComponentNames", Actor::C_ActorList::DebugDumpComponents, (void*)E_DebugDumpLevel_Name);
Debug::Function g_DebugActorListDumpComponentDetails("ActorDebug.DumpComponentDetails", Actor::C_ActorList::DebugDumpComponents, (void*)E_DebugDumpLevel_Detail);
#endif

#ifndef CFG_SPEED
Debug::Function g_DebugActorListDumpSubscriberCounts("ActorDebug.Messaging.DumpSubscriberCounts", Actor::C_ActorList::DebugDumpSubscribers, (void*)E_DebugDumpLevel_Count);
Debug::Function g_DebugActorListDumpSubscriberNames("ActorDebug.Messaging.DumpSubscriberNames", Actor::C_ActorList::DebugDumpSubscribers, (void*)E_DebugDumpLevel_Name);

Debug::Bool g_DebugActorListMessageStatsEnable("ActorDebug.Messaging.Stats.Enable", false);
Debug::Bool g_DebugActorListMessageStatsReset("ActorDebug.Messaging.Stats.Reset", false);
Debug::Float g_DebugActorListMessageStatsX("ActorDebug.Messaging.Stats.X", 100.0f, Debug::Range(-1000.0f, 1000.0f));
Debug::Float g_DebugActorListMessageStatsY("ActorDebug.Messaging.Stats.Y", 100.0f, Debug::Range(-1000.0f, 1000.0f));
Debug::Int g_DebugActorListMessageStatsMaxItems("ActorDebug.Messaging.Stats.MaxItems", 0, Debug::Range(0, 1000));
Debug::Enum<Messaging::E_MessageStatsType> g_DebugActorListMessageStatsSortBy("ActorDebug.Messaging.Stats.SortBy", Messaging::E_MessageStatsType_Average);
Debug::Enum<Messaging::E_MessageStatsValue> g_DebugActorListMessageStatsShowValue("ActorDebug.Messaging.Stats.ShowValue", Messaging::E_MessageStatsValue_Broadcast);
#endif




namespace ActorList_Impl
{

#ifndef CFG_GOLD

class C_ActorDebugRenderVisitor : public Actor::C_ActorList::I_ConstActorListVisitor
{
public:
	explicit C_ActorDebugRenderVisitor(DbUtil::I_DebugRenderPtr pDebugRender) : m_pDebugRender(pDebugRender) {}

	// from Util::I_HandleVisitor
	virtual bool VisitHandleItem(const Actor::C_Actor* pItem)
	{
		pItem->DebugRenderSelfAndComponents(m_pDebugRender);
		return true;
	}

private:
	DbUtil::I_DebugRenderPtr	m_pDebugRender;

};

#endif // CFG_GOLD

} // namespace ActorList_Impl




namespace Actor
{

template <class T_DataType, class T_Factory, class T_BaseClass>
class C_FactoryStreamContext : public T_BaseClass
{
public:
	typedef DataRef::C_Ptr<T_DataType> C_DataRef;

	C_FactoryStreamContext()
		: T_BaseClass()
		, m_Data()
		, m_Map()
	{
	}

	void Insert(const C_DataRef& dataRef)
	{
		if (!dataRef.IsNull() && (dataRef.GetStorageType() == DataRef::kFactoryStorageType))
		{
			const U32 index = m_Data.Size();
			m_Data.PushBack(dataRef);
			m_Map[dataRef.GetData()] = index;
		}
	}

	void SerialiseCount(Sys::C_ContextualStream& stream) const
	{
		stream.WriteAs<U32>(m_Data.Size());
	}

	void SerialiseTypes(Sys::C_ContextualStream& stream) const
	{
		for (typename C_Data::ConstIterator it = m_Data.Begin(); it != m_Data.End(); ++it)
		{
			stream << T_Factory::GetTypeOf(*it->GetData());
		}
	}

	void SerialiseData(Sys::C_ContextualStream& stream) const
	{
		for (typename C_Data::ConstIterator it = m_Data.Begin(); it != m_Data.End(); ++it)
		{
#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
			const Sys::C_ContextualStreamPos startPos = stream.GetStreamPos();
#endif

			stream << *it->GetData();

#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
			const Sys::C_ContextualStreamPos endPos = stream.GetStreamPos();
			const Sys::C_ContextualStreamLen creationLen = endPos - startPos;
			stream << creationLen;
#endif
		}
	}

	void DeserialiseCount(Sys::C_ContextualStream& stream)
	{
		U32 count = 0;
		stream >> count;
		m_Data.Resize(count);
	}

	void DeserialiseTypes(Sys::C_ContextualStream& stream)
	{
		const U32 count = static_cast<U32>(m_Data.Size());

		for (U32 i = 0; i < count; ++i)
		{
			typename T_Factory::C_TypeId typeId = 0;
			stream >> typeId;

			typename T_Factory::C_DataType* pData = NULL;
			C_DataRef dataRef = DataRef::FactoryCreate<T_Factory>(typeId, &pData);
			BIZ_ASSERT(pData != NULL);

			m_Data[i] = dataRef;
			m_Map[dataRef.GetData()] = i;
		}
	}

	void DeserialiseData(Sys::C_ContextualStream& stream)
	{
		const U32 count = static_cast<U32>(m_Data.Size());

		for (U32 i = 0; i < count; ++i)
		{
#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
			const Sys::C_ContextualStreamPos startPos = stream.GetStreamPos();
#endif

			stream >> *const_cast<T_DataType*>(m_Data[i].GetData());

#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
			const Sys::C_ContextualStreamPos endPos = stream.GetStreamPos();
			const Sys::C_ContextualStreamLen calculatedCreationLen = endPos - startPos;

			Sys::C_ContextualStreamLen serialisedCreationLen;
			stream >> serialisedCreationLen;

			BIZ_ASSERT(Sys::CompareStreamLen(calculatedCreationLen, serialisedCreationLen));
#endif
		}
	}

	virtual U32 GetIndex(const T_DataType* pData) const
	{
		const typename C_Map::ConstIterator it = m_Map.Find(pData);
		return (it != m_Map.End()) ? it->GetValue() : T_BaseClass::kInvalidIndex;
	}

	virtual C_DataRef Lookup(const U32 index) const
	{
		return (index < static_cast<U32>(m_Data.Size())) ? m_Data[index] : C_DataRef();
	}

private:
	typedef Btl::Vector<C_DataRef> C_Data;
	typedef Btl::Map<const T_DataType*, U32> C_Map;

	C_Data	m_Data;
	C_Map	m_Map;

};

typedef C_FactoryStreamContext<Component::I_ComponentCreator, Component::C_ComponentCreatorFactory, Component::I_ComponentCreatorFactoryContext> C_ComponentCreatorFactoryContext;
typedef C_FactoryStreamContext<C_ActorCreationData, C_ActorCreationDataFactory, I_ActorCreationDataFactoryContext> C_ActorCreationDataFactoryContext;


/// Helper for (de)serialisation
class C_ActorListHandleSerialisation : public I_ActorHandleSerialisation
{
public:
	explicit C_ActorListHandleSerialisation(const U32 maxHandles) {m_Handles.Reserve(maxHandles);}

	U32 GetCount() const {return static_cast<U32>(m_Handles.Size());}
	const C_ActorHandle& GetHandle(const U32 index) const {return m_Handles[index];}

	/// Adds a handle if there's room.
	/// @param handle handle to add.
	/// @returns true if the handle was added.
	bool PushHandle(const C_ActorHandle& handle)
	{
		if (m_Handles.Size() < m_Handles.Capacity())
		{
			m_Handles.PushBack(handle);
			return true;
		}
		return false;
	}

	// from I_ActorHandleSerialisation
	virtual U32 ActorHandleToU32(const C_ActorHandle& value)
	{
		for (S32 i = 0; i < m_Handles.Size(); ++i)
			if (value == m_Handles[i])
				return i;

		return static_cast<U32>(m_Handles.Capacity());
	}

	// from I_ActorHandleSerialisation
	virtual C_ActorHandle U32ToActorHandle(const U32 value)
	{
		if (value < static_cast<U32>(m_Handles.Size()))
			return m_Handles[value];

		return C_ActorHandle();
	}

private:
	typedef Btl::Vector<C_ActorHandle> C_Handles;
	C_Handles m_Handles;

};

typedef Btl::Vector<const Actor::C_Actor*> C_ValidActors;

class C_ActorListValidActorsVisitor : public C_ActorList::I_ConstActorListVisitor
{
public:
	C_ActorListValidActorsVisitor(C_ValidActors& dest)
		: C_ActorList::I_ConstActorListVisitor()
		, m_ValidActors(dest)
	{
	}

	// from Util::I_HandleVisitor
	virtual bool VisitHandleItem(const C_Actor* pItem)
	{
		switch (pItem->GetLifetimeState())
		{
		case ActorSys::E_LifetimeState_Priming:
		case ActorSys::E_LifetimeState_Primed:
		case ActorSys::E_LifetimeState_Created:
			m_ValidActors.PushBack(pItem);
			break;

		default:
			break;
		}

		return true;
	}

private:
	C_ValidActors&	m_ValidActors;

};




/**
@class C_ActorList.
@ingroup actor_group_main

The major responsibilities of C_ActorList are:
- managing the lifetimes of actors.
- sending messages to actors.
- organising actors into groups.

Handles are used to refer to actors. ActorExists() can be used to test if a
handle refers to a valid actor. For reading data from an actor GetActorPtr()
can be used to get a pointer. Actors should not be written to but should 
instead be sent messages informing them of required changes.

Messages can be sent to a specific actor, or even to a single component within
an actor. Messages can also be sent to whole groups of actors, and can even be
broadcast to every actor. The message subscription system is used to limit
which actors receive broadcasts, but this does not apply to groups.

Messages can be sent to message queues, which the actor list must empty at 
appropriate intervals. These queues are user-defined according to the 
noMsgQueues parameter passed to the constructor. If you're using this then
you must process these message queues by calling ProcessMessageQueue() when
you want the messages to be dispatched to actors.

Messages can also be delayed, either for a number of seconds or for a number
of frames. Delayed messages are dispatched to actors during the call to
UpdateSystem() if the delay has expired.
*/

#ifndef CFG_GOLD
C_ActorList* C_ActorList::s_pLastActorList = NULL;
#endif

/**
@param maxNoActors maximum number of actors that can coexist.
@param noMsgQueues number of message queues.
@param pFactory factory object, must not be NULL.
@param pDefaultContextProvider default context for created actors.
*/
C_ActorList::C_ActorList(const U32 maxNoActors, const U32 noMsgQueues, C_ActorFactory* pFactory, const Sys::I_ContextProvider* pDefaultContextProvider)
	: m_pHandleManager(new Util::C_HandleManager<C_Actor>(maxNoActors))
	, m_pActorFactory(pFactory)
	, m_pDefaultContextProvider(pDefaultContextProvider)
	, m_pKilledActorsList(new Btl::Vector<C_ActorHandle>)
	, m_pStaggerManager(new Util::C_StaggerManager())
	, m_pGenericComponentMessageTypeList(new Messaging::C_MessageTypeListVariable())
	, m_pMessageSubscriberSystem(new Messaging::C_MessageSubscriberSystem())
	, m_pDeferredMessages(new C_DeferredActorMessages(noMsgQueues))
	, m_pDelayedMessages(new C_DelayedActorMessages())
#ifndef CFG_SPEED
	, m_pMessageStats(NULL)
#endif
{
#ifndef CFG_GOLD
	s_pLastActorList = this;
#endif

#ifdef DEFER_ACTOR_MESSAGES
	BIZ_ASSERT(noMsgQueues > 0);
	m_CurrentMessageQueue = 0;
	m_IsFlushingCurrentQueue = false;
#endif

	// reserve actor lists
	m_pKilledActorsList->Reserve(32);
}

C_ActorList::~C_ActorList()
{
	Reset();
	BIZ_ASSERT(m_pHandleManager->Count() == 0);

	delete m_pStaggerManager;
	delete m_pMessageSubscriberSystem;
	delete m_pDeferredMessages;
	delete m_pDelayedMessages;
	delete m_pKilledActorsList;
	delete m_pHandleManager;
	delete m_pGenericComponentMessageTypeList;
#ifndef CFG_SPEED
	delete m_pMessageStats;
#endif

#ifndef CFG_GOLD
	if (s_pLastActorList == this)
	{
		s_pLastActorList = NULL;
	}
#endif
}

struct C_ActorListVisitor_KillActor : public C_ActorList::I_ActorListVisitor
{
	virtual bool VisitHandleItem(C_Actor* pItem)
	{
		pItem->Kill();
		return true;
	}
};

struct C_ActorListVisitor_DeleteActor : public C_ActorList::I_ActorListVisitor
{
	C_ActorList* m_pActorList;

	explicit C_ActorListVisitor_DeleteActor(C_ActorList* pActorList) : m_pActorList(pActorList) {}

	virtual bool VisitHandleItem(C_Actor* pItem)
	{
		m_pActorList->DeleteActor(pItem);
		return true;
	}
};

/**
@brief Eeset the actor list.
Destroys all actors.
*/
void C_ActorList::Reset(void)
{
	// Kill Actors
	C_ActorListVisitor_KillActor killActor;
	VisitActors(killActor);
	m_pKilledActorsList->Clear();

	// Delete Actors
	C_ActorListVisitor_DeleteActor deleteActor(this);
	VisitActors(deleteActor);

	m_pStaggerManager->Reset();
}

/**
@brief Construct an actor.
Creates a new actor in the Construct state.
@param actorType type of actor to construct.
@returns handle to constructed actor.
*/
C_ActorHandle C_ActorList::ConstructActor(const C_ActorTypeId actorType)
{
	C_Actor* pActor = GetActorFactory()->CreateActor(actorType);

	if (pActor != NULL)
	{
		pActor->m_pActorList = this;
		pActor->m_Type = actorType;
		pActor->SetSubscriberSystem(&GetSubscriberSystem());
		return m_pHandleManager->AllocateHandle(pActor);
	}

	return C_ActorHandle();
}

/**
@brief Binds the actor to the creation data and vice versa.
@param handle created actor.
@param pActorCreate creation data.
@returns false if the actor couldn't be bound.
*/
bool C_ActorList::BindActorCreationData(C_Actor* pActor, const DataRef::C_Ptr<C_ActorCreationData>& pActorCreate)
{
	if (pActor->GetActorTypeId() != pActorCreate->GetType())
		return false;

	// setup creation data
	pActorCreate->LinkActorHandle(pActor->GetHandle());

	// setup actor
	pActor->SetCreationData(pActorCreate);
#ifdef ACTOR_NAME_DEBUG
	{
		const char* name = pActorCreate->GetName();
		if (name != NULL)
			pActor->m_Name = Util::C_String(name);
	}
#endif

	// done
	return true;
}

/**
@brief Initialises an actor.
Takes an actor in the Construct state, associates it with creation data and
initialises it.
@param handle actor to initialise.
@param pActorCreate creation data to use.
@returns true if successful.
*/
bool C_ActorList::InitialiseActor(const C_ActorHandle& handle, const DataRef::C_Ptr<C_ActorCreationData>& pActorCreate)
{
	if (pActorCreate.IsNull())
		return false;

	C_Actor* pActor = GetActorRWPtr(handle);
	if (pActor == NULL)
		return false;

	if (!BindActorCreationData(pActor, pActorCreate))
		return false;

	// initialise actor
	return pActor->Initialise();
}

/**
@brief Initialises an slaved actor.
Client actors can be enslaved to the server instead of being initialised
locally, so the server will provide components and deserialise data.
@param handle actor to initialise.
@param pActorCreate creation data to use.
@returns true if successful.
*/
bool C_ActorList::EnslaveActor(const C_ActorHandle& handle, const DataRef::C_Ptr<C_ActorCreationData>& pActorCreate)
{
	if (pActorCreate.IsNull())
		return false;

	C_Actor* pActor = GetActorRWPtr(handle);
	if (pActor == NULL)
		return false;

	if (!BindActorCreationData(pActor, pActorCreate))
		return false;

	// initialise actor
	return pActor->Enslave();
}

/**
@brief Primes an actor.
Takes an actor in the Initialised state and attempts to prime it. If this
succeeds then the actor may be either Priming or Primed.
@param handle actor to prime.
@returns true if successful.
*/
bool C_ActorList::PrimeActor(const C_ActorHandle& handle)
{
	C_Actor* pActor = GetActorRWPtr(handle);
	if (pActor == NULL)
		return false;

	return pActor->Prime();
}

/**
@brief Creates an actor.
Takes an actor in the Primed state and completes it creation.
@param handle actor to create.
@returns true if the actor was created successfully.
*/
bool C_ActorList::CreateActor(const C_ActorHandle& handle)
{
	C_Actor* pActor = GetActorRWPtr(handle);
	if (pActor == NULL)
		return false;

	if (pActor->Create())
	{
		const C_ActorCreatedMsg message(pActor->GetHandle());
		BroadcastMessage(&message);
		return true;
	}

	return false;
}

#ifdef PROFILE_ACTOR_CREATION
#include "Tracerecording.h"
#include "Xbdm.h"
#endif
/**
@brief Constructs, initialises and primes actors.
@param list creation data for actors.
@param config configuration for creation.
@param io_pStatus status for staggered creation, may be NULL.
@returns false if errors aborted the process.
*/
bool C_ActorList::CreatePreLoad(const I_ActorCreationList& list, const C_ActorCreationConfig& config, C_ActorCreationStatus* io_pStatus)
{
	const U32 numCreators = list.CalcNumCreators();
	
	C_ActorCreationStatus status;

	if (io_pStatus != NULL)
	{
		status = *io_pStatus;
	}
	
	// begin
	status.Begin(C_ActorCreationStatus::E_Phase_Construct);

// profiling capture
#ifdef PROFILE_ACTOR_CREATION
	Util::C_String fname;
	static int count = 0;

	fname.Printf("GAME:\\Caps\\ActorPreload%d_%d.cap",count++, numCreators);
	//fname.Printf("GAME:\\Caps\\ActorPreload%d_%d.pix2",count++, numCreators);

	DmStartProfiling(fname.Chars(),0);
	//XTraceStartRecording(fname.Chars());
// ------------
#endif
	// construct actors
	if (status.GetPhase() == C_ActorCreationStatus::E_Phase_Construct)
	{
		PERF_EVENT("Actor.Construct");

		U32 startIndex, endIndex;
		status.Update(numCreators, startIndex, endIndex);

		for (U32 i = startIndex; i < endIndex; ++i)
		{
			const C_ActorCreationData::C_DataRef creator = list.GetCreator(i);

			if (!creator.IsNull())
			{
				const C_ActorHandle actorHandle = ConstructActor(creator->GetType());

				if (!ActorExists(actorHandle))
				{
					LOGERROR(GetDebugLogLevelDesign(), DBSTR "Failed to construct actor %s of type %d", creator->GetName(), creator->GetType());
					
					if (config.m_OnFail == C_ActorCreationConfig::E_OnFail_Abort)
						return false;
				}
				else
				{
					C_Actor* pActor = GetActorRWPtr(actorHandle);
					BIZ_ASSERT(pActor != NULL);
					if (!BindActorCreationData(pActor, creator))
					{
						BIZ_ASSERT(false); // should never happen!
					}
				}
			}
		}
	}

	// initialise actors
	if (status.GetPhase() == C_ActorCreationStatus::E_Phase_Initialise)
	{
		PERF_EVENT("Actor.Initialise");

		U32 startIndex, endIndex;
		status.Update(numCreators, startIndex, endIndex);

		for (U32 i = startIndex; i < endIndex; ++i)
		{
			const C_ActorCreationData::C_DataRef creator = list.GetCreator(i);

			if (!creator.IsNull())
			{
				const C_ActorHandle actorHandle = creator->GetActorHandle();
				C_Actor* pActor = GetActorRWPtr(actorHandle);

				if ((pActor != NULL) && !pActor->Initialise())
				{
					LOGERROR(GetDebugLogLevelDesign(pActor), DBSTR "Failed to initialise actor %s of type %s", creator->GetName(), m_pActorFactory->LookupName(creator->GetType()).Chars());

					SendMessageToActor(actorHandle, Actor::E_ActorMsg_Kill);

					if (config.m_OnFail == C_ActorCreationConfig::E_OnFail_Abort)
						return false;
				}
			}
		}
	}

	// prime actors
	if (status.GetPhase() == C_ActorCreationStatus::E_Phase_Prime)
	{
		PERF_EVENT("Actor.Prime");

		U32 startIndex, endIndex;
		status.Update(numCreators, startIndex, endIndex);

		for (U32 i = startIndex; i < endIndex; ++i)
		{
			const C_ActorCreationData::C_DataRef creator = list.GetCreator(i);

			if (!creator.IsNull())
			{
				const C_ActorHandle actorHandle = creator->GetActorHandle();

				if (ActorExists(actorHandle))
				{
					PrimeActor(actorHandle);
				}
			}
		}
	}

	if (io_pStatus != NULL)
	{
		*io_pStatus = status;
	}
#ifdef PROFILE_ACTOR_CREATION
	// ------------
	//LOGERROR(Actor::GetDebugLogGeneral(),"%d Actors: Construct %dms, Init %dms, Prime %dms, Total %dms",numCreators, constructCount,initCount,primeCount,totalCount);
	DmStopProfiling();
	//XTraceStopRecording();
	// ------------
#endif

	return true;
}

/**
@brief Checks if any of the actors are waiting for data to load.
@param list partially-created actors.
@returns true if any actor is priming.
*/
bool C_ActorList::IsWaitingForLoad(const I_ActorCreationList& list) const
{
	const U32 numCreators = list.CalcNumCreators();

	for (U32 i = 0; i < numCreators; ++i)
	{
		const C_ActorCreationData* pCreationData = list.GetCreator(i).GetData();

		if (pCreationData != NULL)
		{
			const C_Actor* pActor = GetActorPtr(pCreationData->GetActorHandle());

			if ((pActor != NULL) && (pActor->GetLifetimeState() == ActorSys::E_LifetimeState_Priming))
				return true;
		}
	}

	return false;
}

/**
@brief Completes the creation of primed actors.
@param list primed actors.
@param config configuration for creation.
@param io_pStatus status for staggered creation, may be NULL.
@returns false if errors aborted the process.
*/
bool C_ActorList::CreatePostLoad(const I_ActorCreationList& list, const C_ActorCreationConfig& config, C_ActorCreationStatus* io_pStatus)
{
	const U32 numCreators = list.CalcNumCreators();

	C_ActorCreationStatus status;

	if (io_pStatus != NULL)
	{
		status = *io_pStatus;
	}


#ifdef PROFILE_ACTOR_CREATION
	Util::C_String fname;
	static int count = 0;


	fname.Printf("GAME:\\Caps\\ActorPostLoad%d_%d.cap",count++, numCreators);
	//fname.Printf("GAME:\\Caps\\ActorPostLoad%d_%d.pix2",count++, numCreators);

	DmStartProfiling(fname.Chars(),20 * 4096 * 1024);
	//XTraceStartRecording(fname.Chars());
#endif

	// begin
	status.Begin(C_ActorCreationStatus::E_Phase_Create);

	// create actors
	if (status.GetPhase() == C_ActorCreationStatus::E_Phase_Create)
	{
		PERF_EVENT("Actor.Create");

		U32 startIndex, endIndex;
		status.Update(numCreators, startIndex, endIndex);

		for (U32 i = startIndex; i < endIndex; ++i)
		{
			const C_ActorCreationData::C_DataRef creator = list.GetCreator(i);

			if (!creator.IsNull())
			{
				const C_ActorHandle actorHandle = creator->GetActorHandle();

				if (ActorExists(actorHandle) && !CreateActor(actorHandle))
				{
					LOGERROR(GetDebugLogLevelDesign(GetActorPtr(actorHandle)), DBSTR "Failed to create actor %s of type %s", creator->GetName(), m_pActorFactory->LookupName(creator->GetType()).Chars());
					
					SendMessageToActor(actorHandle, Actor::E_ActorMsg_Kill);
					
					if (config.m_OnFail == C_ActorCreationConfig::E_OnFail_Abort)
						return false;
				}
			}
		}
	}

	if (status.GetPhase() == C_ActorCreationStatus::E_Phase_InternalDependencies)
	{
		PERF_EVENT("Actor.InternalDependencies");

		U32 startIndex, endIndex;
		status.Update(numCreators, startIndex, endIndex);

		for (U32 i = startIndex; i < endIndex; ++i)
		{
			const C_ActorCreationData::C_DataRef creator = list.GetCreator(i);

			if (!creator.IsNull())
			{
				C_Actor* pActor = GetActorRWPtr(creator->GetActorHandle());

				if ((pActor != NULL) && (pActor->GetLifetimeState() == ActorSys::E_LifetimeState_Created))
				{
					pActor->InternalDependenciesCreated();
				}
			}
		}
	}

	if (status.GetPhase() == C_ActorCreationStatus::E_Phase_ExternalDependencies)
	{
		PERF_EVENT("Actor.ExternalDependencies");

		U32 startIndex, endIndex;
		status.Update(numCreators, startIndex, endIndex);

		for (U32 i = startIndex; i < endIndex; ++i)
		{
			const C_ActorCreationData::C_DataRef creator = list.GetCreator(i);

			if (!creator.IsNull())
			{
				C_Actor* pActor = GetActorRWPtr(creator->GetActorHandle());

				if ((pActor != NULL) && (pActor->GetLifetimeState() == ActorSys::E_LifetimeState_Created))
				{
					pActor->ExternalDependenciesCreated();
				}
			}
		}
	}

	if (io_pStatus != NULL)
	{
		*io_pStatus = status;
	}

#ifdef PROFILE_ACTOR_CREATION
	// ------------
	//LOGERROR(Actor::GetDebugLogGeneral(),"%d Actors: Create Count %dms", numCreators, GetTickCount() - msCount);
	//XTraceStopRecording();
	DmStopProfiling();
	// ------------
#endif
	return true;
}

/**
@brief Spawn an actor.
Constructs and creates the actor in one go, use for mid level spawning, eg. Debris.
@param pActorCreate factory creation data.
@returns handle to spawned actor.
*/
C_ActorHandle C_ActorList::Spawn(const DataRef::C_Ptr<C_ActorCreationData>& pActorCreate)
{
	BIZ_ASSERT(!pActorCreate.IsNull());

	C_ActorCreationListSingle creationList(pActorCreate);

	C_ActorCreationConfig config;
	config.m_OnFail = C_ActorCreationConfig::E_OnFail_Abort;

	if (!CreatePreLoad(creationList, config))
	{
		LOGERROR(GetDebugLogLevelDesign(GetActorPtr(pActorCreate->GetActorHandle())), DBSTR "Failed to spawn actor, pre-load aborted");
	}
	else if (IsWaitingForLoad(creationList))
	{
		LOGERROR(GetDebugLogLevelDesign(GetActorPtr(pActorCreate->GetActorHandle())), DBSTR "Failed to spawn actor, resources were not loaded in time");
		SendMessageToActor(pActorCreate->GetActorHandle(), E_ActorMsg_Kill);
	}
	else if (!CreatePostLoad(creationList, config))
	{
		LOGERROR(GetDebugLogLevelDesign(GetActorPtr(pActorCreate->GetActorHandle())), DBSTR "Failed to spawn actor, post-load aborted");
	}
	else
	{
		return pActorCreate->GetActorHandle();  
	}

	return C_ActorHandle();  
}


void C_ActorList::MarkForDeletion(C_Actor& actor)
{
	BIZ_ASSERT(actor.GetLifetimeState() == ActorSys::E_LifetimeState_Killed);
	actor.SetActorFlag(Actor::E_ActorFlag_Delete, true);
}

/**
@brief Delete an actor and remove it from the actor list.
@param pActor actor to delete.
@returns true if the actor was deleted.
*/
bool C_ActorList::DeleteActor(C_Actor* pActor)
{
	if (gActorDebugLogActorListSize)
		LOGINFO(GetDebugLogGeneral(), "<<<< ACTOR DELETE: handle index %d, actorlist size %d", pActor->GetHandle().GetIndex(), m_pHandleManager->Count());

	m_pHandleManager->FreeHandle(pActor);
	return GetActorFactory()->DestroyActor(pActor);	
}

/**
@brief Tests if this handle refers to a valid actor.
@param handle handle to test.
@returns true if this handle refers to a valid actor.
*/
bool C_ActorList::ActorExists(C_ActorHandle handle) const 
{
	return m_pHandleManager->IsValidHandle(handle);
}

/**
@brief Gets the actor referred to by this handle.
@param handle handle to use.
@returns actor referred to by handle, or NULL if the handle is not valid.
*/
C_Actor* C_ActorList::GetActorRWPtr(C_ActorHandle handle)
{
	return m_pHandleManager->GetItemPtr(handle);
}

/**
@brief Visits actors in the list.
@param visitor object to visit the actors in the list.
@returns false if the visitor aborted.
*/
bool C_ActorList::VisitActors(I_ActorListVisitor& visitor)
{
	return m_pHandleManager->Visit(visitor);
}

/**
@brief Visits actors in the list.
@param visitor object to visit the actors in the list.
@returns false if the visitor aborted.
*/
bool C_ActorList::VisitActors(I_ConstActorListVisitor& visitor) const
{
	return m_pHandleManager->Visit(visitor);
}

/**
@brief Gets a component by interface.
@param actor actor to get the component interface from.
@param interfaceID type identifier for the interface.
@returns NULL if the interface is not supported.
*/
const I_ActorComponent* C_ActorList::QueryInterface(C_ActorHandle actor, Component::C_ComponentInterfaceId interfaceID) const
{
	const C_Actor* pActor = m_pHandleManager->GetItemPtr(actor);
	return (pActor == NULL) ? NULL : pActor->QueryInterface(interfaceID);
}

/**
@brief Adds the actor to the list of killed actors to be destroyed.
@param pActor actor that has been killed.
*/
void C_ActorList::AddKilledActor(C_Actor* pActor) 
{
	m_pKilledActorsList->PushBack(pActor->GetHandle());
}

/**
@brief Update the actor system.
Sends delayed messages, deletes dead actors.
@param timeStep number of seconds passed since last update.
@param frameNo frame number.
*/
void C_ActorList::UpdateSystem(const Maths::C_TimeS& timeStep, const U32 frameNo)
{
	UpdateSystemNonGlobal(timeStep, frameNo);
	UpdateSystemGlobal();
}

void C_ActorList::UpdateSystemGlobal()
{
	// remove dead actors
	if(m_pKilledActorsList->Size())
	{
		//Btl::Vector<C_Actor*> stillToKill; // list for actors not ready to kill yet (still have collision)

		for(S32 killedActorNo = 0; killedActorNo < m_pKilledActorsList->Size(); killedActorNo++)
		{
			C_Actor* pActor = GetActorRWPtr((*m_pKilledActorsList)[killedActorNo]);

//			if(pActor->TestActorFlag(E_ActorFlag_Delete))
			if(pActor)
				DeleteActor(pActor);
//			else
//				stillToKill.PushBack(pActor);	// not ready to kill yet
		}

		// clear the kill list
		m_pKilledActorsList->Clear();

		// if there are any actors still to kill put them back in the kill list
		// TODO: think of a more elegant way of doing this
// 		if(stillToKill.Size())
// 		{
// 			for(S32 toKillActorNo = 0; toKillActorNo < stillToKill.Size(); toKillActorNo++)
// 				m_pKilledActorsList->PushBack(stillToKill[toKillActorNo]);
// 		}
	}

#ifndef CFG_SPEED
	if (g_DebugActorListMessageStatsEnable)
	{
		if (m_pMessageStats == NULL)
		{
			m_pMessageStats = new Messaging::C_MessageStats();
			m_pMessageSubscriberSystem->SetStats(m_pMessageStats);
		}

		m_pMessageStats->Update();

		if (g_DebugActorListMessageStatsReset)
		{
			m_pMessageStats->Reset();
			g_DebugActorListMessageStatsReset.SetValue(false);
		}
	}
	else
	{
		if (m_pMessageStats != NULL)
		{
			m_pMessageSubscriberSystem->SetStats(NULL);
			delete m_pMessageStats;
			m_pMessageStats = NULL;
		}
	}
#endif

#if ACTOR_SYSTEM_PERF_EVENTS
	C_MessagePerfEvents::GlobalUpdate();
#endif
}

void C_ActorList::UpdateSystemNonGlobal(const Maths::C_TimeS& timeStep, const U32 frameNo)
{
	PERF_EVENT("UpdateSystemNonGlobal");

	m_CurrentFrameNo = frameNo;
	// send delayed messages
	m_pDelayedMessages->Update(static_cast<F32>(timeStep.Get()), this);
}

/**
@brief Sends a message to a specific component of the actor.
@param dest actor containing the component.
@param component interface type identifier for the component to receive the message.
@param pMessage message to send, must not be NULL.
@param msgQueue user-define message queue index.
@returns result of sending.
*/
E_ActorMessageSendResult C_ActorList::SendMessageToActorComponent(const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage, S32 msgQueue)
{
	C_Actor* pDest = m_pHandleManager->GetItemPtr(dest);

	BIZ_ASSERT(pMessage);

	// if actor doesn't exist or has been killed return 0
	if(pDest == NULL)
	{
		return E_ActorMessageSendResult_NoActor;
	}

	// MGB: need to send dead objects a 'RemoveCollision' msg:
	//	if(pDest->FlagKilled()) 
	//		return E_ActorMessageSendResult_ActorKilled;

#ifdef DEFER_ACTOR_MESSAGES
	S32 msgQueueIndex = (msgQueue < 0) ? m_CurrentMessageQueue : msgQueue;
	m_pDeferredMessages->PushMessage(static_cast<U32>(msgQueueIndex), dest, component, pMessage);
	return E_ActorMessageSendResult_OK;
#else
	if(msgQueue == -1)
	{
		return SendMessageNow(dest, component, pMessage);
	}
	else
	{
		m_pDeferredMessages->PushMessage(static_cast<U32>(msgQueue), dest, component, pMessage);
		return E_ActorMessageSendResult_OK;
	}
#endif
}

/**
@brief Broadcasts the message to all subscribed actors.
@param pMessage message to send, must not be NULL.
@param mask bitmask to filter subscribers with.
*/
void C_ActorList::BroadcastMessage(const Messaging::C_MessageBase* pMessage, U32 mask)
{
#ifdef DEFER_ACTOR_MESSAGES
	m_pDeferredMessages->BroadcastMessage(static_cast<U32>(m_CurrentMessageQueue), mask, pMessage);
	FlushCurrentMessageQueue();
#else
	BroadcastMessageNow(mask, pMessage);
#endif
}

/**
@brief Delays sending the message for a number of frames.
@param numFrames number of frames to delay message for.
@param dest actor to receive the message.
@param component component interface type identifier within actor.
@param pMessage message to send.
@param msgQueue user-defined message queue index.
*/
void C_ActorList::DelayMessageToActorComponentByFrames(const U32 numFrames, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage, const S32 msgQueue)
{
	const U32 messageQueue = (msgQueue < 0) ? 0 : static_cast<U32>(msgQueue);
	m_pDelayedMessages->DelayMessageByFrames(numFrames, messageQueue, dest, component, pMessage);
}

/**
@brief Delays sending the message for a number of frames.
@param numFrames number of frames to delay message for.
@param pMessage message to send.
@param mask bitmask to filter the message with.
*/
void C_ActorList::DelayMessageBroadcastByFrames(const U32 numFrames, const Messaging::C_MessageBase* pMessage, const U32 mask)
{
	m_pDelayedMessages->DelayBroadcastByFrames(numFrames, mask, pMessage);
}

/**
@brief Delays sending the message for a number of seconds.
@param numSeconds number of seconds to delay message for.
@param dest actor to receive the message.
@param component component interface type identifier within actor.
@param pMessage message to send.
@param msgQueue user-defined message queue index.
*/
void C_ActorList::DelayMessageToActorComponentBySeconds(const F32 numSeconds, const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage, const S32 msgQueue)
{
	const U32 messageQueue = (msgQueue < 0) ? 0 : static_cast<U32>(msgQueue);
	m_pDelayedMessages->DelayMessageBySeconds(numSeconds, messageQueue, dest, component, pMessage);
}

/**
@brief Delays sending the message for a number of seconds.
@param numSeconds number of seconds to delay message for.
@param pMessage message to send.
@param mask bitmask to filter the message with.
*/
void C_ActorList::DelayMessageBroadcastBySeconds(const F32 numSeconds, const Messaging::C_MessageBase* pMessage, const U32 mask)
{
	m_pDelayedMessages->DelayBroadcastBySeconds(numSeconds, mask, pMessage);
}

/**
@brief Dispatch the message to the actor.
@param dest actor to receive the message.
@param component component interface type identifier, -1 to send to actor.
@param pMessage message to send.
@returns result of sending.
*/
E_ActorMessageSendResult C_ActorList::SendMessageNow(const C_ActorHandle dest, const Component::C_ComponentInterfaceId component, const Messaging::C_MessageBase* pMessage)
{
	C_Actor* pDest = m_pHandleManager->GetItemPtr(dest);

	// if actor doesn't exist or has been killed return 0
	if(pDest == NULL)
	{
		return E_ActorMessageSendResult_NoActor;
	}

	// MGB: need to send dead objects a 'RemoveCollision' msg:
	//	if(pDest->FlagKilled()) 
	//		return E_ActorMessageSendResult_ActorKilled;

	const U32 numHandlers = pDest->ProcessMessage( pMessage, component );

#ifndef CFG_SPEED
	if (m_pMessageStats != NULL)
	{
		m_pMessageStats->AccumulateSend(pMessage->Type(), numHandlers);
	}
#endif

	return E_ActorMessageSendResult_OK;
}

/**
@brief Dispatch the broadcast to all subscribed actors.
@param mask bitmask to filter subscribers by.
@param pMessage message to send.
*/
void C_ActorList::BroadcastMessageNow(const U32 mask, const Messaging::C_MessageBase* pMessage)
{
	m_pMessageSubscriberSystem->BroadcastMessage(pMessage, mask);
}

/**
@brief Dispatches all messages in this queue.
@param msgQueue user-defined message queue index.
*/
void C_ActorList::ProcessMessageQueue(const S32 msgQueue)
{
	BIZ_ASSERT(msgQueue >= 0);
	m_pDeferredMessages->ProcessMessages(static_cast<U32>(msgQueue), this);
}

/**
@brief Serialises the actors for saving.
@param stream stream to write into.
@returns true if the actors are serialised.
*/
bool C_ActorList::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	if (stream.HasErrors())
		return false;

	// gather valid actors
	C_ValidActors validActors;
	validActors.Reserve(m_pHandleManager->Count());
	{
		C_ActorListValidActorsVisitor visitor(validActors);
		VisitActors(visitor);
	}

	// serialise the number of actors
	const U32 numActors = static_cast<U32>(validActors.Size());
	stream << numActors;

	// serialise the actor handle table
	C_ActorListHandleSerialisation actorHandles(numActors);
	C_ComponentCreatorFactoryContext componentCreatorFactoryContext;
	C_ActorCreationDataFactoryContext actorCreationDataFactoryContext;

	for (C_ValidActors::ConstIterator it = validActors.Begin(); it != validActors.End(); ++it)
	{
		const Actor::C_Actor* pItem = *it;

		// write the actor type id... will be invalidated if the id value changes between builds, remember to update version number!
		stream << pItem->GetActorTypeId();

		// add the handle to the serialisation table
		if (!actorHandles.PushHandle(pItem->GetHandle()))
		{
			LOGERROR(GetDebugLogGeneral(), "too many actors for serialisation of handles");
			return false;
		}

		// add the creation data
		actorCreationDataFactoryContext.Insert(pItem->GetCreationData());

		const U32 numComponents = pItem->GetNumComponents();
		for (U32 i = 0; i < numComponents; ++i)
		{
			componentCreatorFactoryContext.Insert(pItem->GetComponent(i)->GetCreator());
		}
	}

	// write creation data counts and types
	componentCreatorFactoryContext.SerialiseCount(stream);
	componentCreatorFactoryContext.SerialiseTypes(stream);

	actorCreationDataFactoryContext.SerialiseCount(stream);
	actorCreationDataFactoryContext.SerialiseTypes(stream);

	// add the contexts
	if (!stream.AddContextItem(I_ActorHandleSerialisation::ClassUid(), &actorHandles) || 
		!stream.AddContextItem(I_ActorCreationDataFactoryContext::ClassUid(), &actorCreationDataFactoryContext) || 
		!stream.AddContextItem(Component::I_ComponentCreatorFactoryContext::ClassUid(), &componentCreatorFactoryContext))
	{
		BIZ_ASSERT(false); // calling code shouldn't be providing these contexts!
		return false;
	}

	// serialise the factory creation data
	componentCreatorFactoryContext.SerialiseData(stream);
	actorCreationDataFactoryContext.SerialiseData(stream);

	// serialise the actor data
	for (C_ValidActors::ConstIterator it = validActors.Begin(); it != validActors.End(); ++it)
	{
		const Actor::C_Actor* pItem = *it;

		stream << pItem->GetCreationData();

#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
		const Sys::C_ContextualStreamPos startPos = stream.GetStreamPos();
#endif

		pItem->SerialiseForSave(stream);

#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
		const Sys::C_ContextualStreamPos endPos = stream.GetStreamPos();
		const Sys::C_ContextualStreamLen actorLen = endPos - startPos;
		stream << actorLen;
#endif
	}

	// serialise the message queues
	stream << *m_pDeferredMessages;
	stream << *m_pDelayedMessages;

	// remove the contexts
	stream.RemoveContextItem(I_ActorHandleSerialisation::ClassUid());
	stream.RemoveContextItem(I_ActorCreationDataFactoryContext::ClassUid());
	stream.RemoveContextItem(Component::I_ComponentCreatorFactoryContext::ClassUid());

	// done
	return !stream.HasErrors();
}

/**
@param creationList receives primed/priming actors.
@param stream stream to read from.
@param config configuration data.
@returns true if the actors are deserialised.
*/
bool C_ActorList::DeserialiseForLoad(C_ActorCreationListMultiple& creationList, Sys::C_ContextualStream& stream)
{
	if (stream.HasErrors())
		return false;

	// read the number of actors
	U32 numActors = 0;
	stream >> numActors;

	// deserialise the actor handle table and construct the actors
	C_ActorListHandleSerialisation actorHandles(numActors);
	for (U32 i = 0; i < numActors; ++i)
	{
		// read the actor type
		C_ActorTypeId actorType = 0;
		stream >> actorType;

		// construct the actor
		C_ActorHandle actorHandle = ConstructActor(actorType);

		if (actorHandle == C_ActorHandle())
		{
			// we don't know how to make this actor, probably invalid data
			LOGERROR(GetDebugLogGeneral(), "don't know how to make deserialised actor");
			return false;
		}
	
		// record the handle
		if (!actorHandles.PushHandle(actorHandle))
		{
			// epic fail... should never happen
			BIZ_ASSERT(false);
			return false;
		}
	}

	// deserialise the creation data types and construct them
	C_ComponentCreatorFactoryContext componentCreatorFactoryContext;
	componentCreatorFactoryContext.DeserialiseCount(stream);
	componentCreatorFactoryContext.DeserialiseTypes(stream);
	
	C_ActorCreationDataFactoryContext actorCreationDataFactoryContext;
	actorCreationDataFactoryContext.DeserialiseCount(stream);
	actorCreationDataFactoryContext.DeserialiseTypes(stream);

	// add the contexts
	if (!stream.AddContextItem(I_ActorHandleSerialisation::ClassUid(), &actorHandles) || 
		!stream.AddContextItem(I_ActorCreationDataFactoryContext::ClassUid(), &actorCreationDataFactoryContext) || 
		!stream.AddContextItem(Component::I_ComponentCreatorFactoryContext::ClassUid(), &componentCreatorFactoryContext))
	{
		BIZ_ASSERT(false); // calling code shouldn't be providing these contexts!
		return false;
	}

	// deserialise actor creation data
	componentCreatorFactoryContext.DeserialiseData(stream);
	actorCreationDataFactoryContext.DeserialiseData(stream);

	// deserialise the actor data
	creationList.ReserveActorCreationListItems(numActors);

	for (U32 i = 0; i < numActors; ++i)
	{
		// read the creation data
		DataRef::C_Ptr<C_ActorCreationData> pCreationData;
		stream >> pCreationData;

		if (pCreationData.IsNull())
		{
			LOGERROR(GetDebugLogGeneral(), "failed to deserialise actor creation data");
			return false;
		}

#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
		const Sys::C_ContextualStreamPos startPos = stream.GetStreamPos();
#endif

		// initialise the actor
		C_Actor* pActor = GetActorRWPtr(actorHandles.GetHandle(i));

		if (!BindActorCreationData(pActor, pCreationData))
		{
			LOGERROR(GetDebugLogGeneral(), "failed to bind actor creation data for deserialisation");
			return false;
		}

		if (!pActor->Initialise(stream))
		{
			LOGERROR(GetDebugLogGeneral(), "failed to initialise actor components for deserialisation");
			return false;
		}

#if DEBUG_ACTOR_SERIALISE_FOR_SAVE
		const Sys::C_ContextualStreamPos endPos = stream.GetStreamPos();
		const Sys::C_ContextualStreamLen calculatedActorLen = endPos - startPos;

		Sys::C_ContextualStreamLen serialisedActorLen;
		stream >> serialisedActorLen;

		if (!Sys::CompareStreamLen(calculatedActorLen, serialisedActorLen))
		{
			LOGERROR(GetDebugLogGeneral(), "serialised data for actor '%s' has wrong size", pActor->GetActorName());
			return false;
		}
#endif

		creationList.AddActorCreationListItem(pCreationData);
	}

	// deserialise the message queues
	stream >> *m_pDeferredMessages;
	stream >> *m_pDelayedMessages;

	// remove the contexts
	stream.RemoveContextItem(I_ActorHandleSerialisation::ClassUid());
	stream.RemoveContextItem(I_ActorCreationDataFactoryContext::ClassUid());
	stream.RemoveContextItem(Component::I_ComponentCreatorFactoryContext::ClassUid());

	// prime the actors
	for (U32 i = 0; i < numActors; ++i)
	{
		PrimeActor(actorHandles.GetHandle(i));
	}

	// done
	return !stream.HasErrors();
}

void C_ActorList::AddComponentGenericMessageType(const Messaging::TMsgType msgType)
{
	m_pGenericComponentMessageTypeList->Insert(msgType);
}

void C_ActorList::AddComponentGenericMessageTypes(const Messaging::I_MessageTypeList& list)
{
	m_pGenericComponentMessageTypeList->Insert(list);
}

void C_ActorList::DebugRender(DbUtil::I_DebugRenderPtr debugRender) const
{
#ifndef CFG_GOLD
	{
		ActorList_Impl::C_ActorDebugRenderVisitor visitor(debugRender);
		VisitActors(visitor);
	}
#endif

#ifndef CFG_SPEED
	if (g_DebugActorListMessageStatsEnable && (m_pMessageStats != NULL))
	{
		const Maths::HVec2 origin(g_DebugActorListMessageStatsX.Value(), g_DebugActorListMessageStatsY.Value());
		const Messaging::E_MessageStatsType sortType = g_DebugActorListMessageStatsSortBy;
		const Messaging::E_MessageStatsValue showValue = g_DebugActorListMessageStatsShowValue;
		const U32 maxItems = g_DebugActorListMessageStatsMaxItems;

		m_pMessageStats->DebugRender(debugRender, origin, sortType, showValue, maxItems);
	}
#endif
}

#ifdef DEFER_ACTOR_MESSAGES

/**
@brief Flushes the 'current' message queue, dispatching all pending messages.
Should be called to synchronise message sending.
*/
void C_ActorList::FlushCurrentMessageQueue()
{
	if(!m_IsFlushingCurrentQueue)
	{
		m_IsFlushingCurrentQueue = true;
		ProcessMessageQueue(m_CurrentMessageQueue);
		m_IsFlushingCurrentQueue = false;
	}
}

/**
@brief Sets the 'current' message queue.
When all actor messages are being deferred, the special queue value of -1 is 
interpreted as the 'current' message queue and the message will be pushed 
onto this queue and dispatched later.
@param msgQueue index of new current queue.
*/
void C_ActorList::SetCurrentMessageQueue(S32 msgQueue)
{
	BIZ_ASSERT(msgQueue >= 0);
	BIZ_ASSERT(msgQueue < static_cast<S32>(m_pDeferredMessages->GetNumQueues()));

	if(m_CurrentMessageQueue != msgQueue)
	{
		FlushCurrentMessageQueue();
		m_CurrentMessageQueue = msgQueue;
	}
}

#endif // def DEFER_ACTOR_MESSAGES


#ifndef CFG_SPEED

class C_GatherActorInfoVisitor : public C_ActorList::I_ConstActorListVisitor
{
public:
	C_ActorInfos&	m_Dest;

	explicit C_GatherActorInfoVisitor(C_ActorInfos& dest) : m_Dest(dest) {}

	virtual bool VisitHandleItem(const C_Actor* pItem)
	{
		const S32 index = m_Dest.Size();
		m_Dest.Resize(index + 1);
		C_ActorInfo& dest = m_Dest[index];

		dest.m_Handle = pItem->GetHandle();
		dest.m_TypeId = pItem->GetActorTypeId();
		strncpy_s(dest.m_Name, sizeof(dest.m_Name), pItem->GetActorName(), Maths::Min(sizeof(dest.m_Name) - 1,strlen(pItem->GetActorName())));

		return true;
	}
};

void C_ActorList::GatherActorInfo(C_ActorInfos& dest) const
{
	dest.Clear();
	dest.Reserve(m_pHandleManager->Count());

	C_GatherActorInfoVisitor visitor(dest);
	VisitActors(visitor);
}

#endif // CFG_SPEED


#ifndef CFG_GOLD

class C_DebugActorListDumpVisitor : public C_ActorList::I_ConstActorListVisitor
{
public:
	typedef Btl::Vector<const C_Actor*> C_Actors;
	typedef Btl::Map<C_ActorTypeId, C_Actors> C_Data;

	C_Data	m_Data;
	U32		m_Count;

	C_DebugActorListDumpVisitor() : m_Count(0) {}

	virtual bool VisitHandleItem(const C_Actor* pItem)
	{
		m_Data[pItem->GetActorTypeId()].PushBack(pItem);
		++m_Count;
		return true;
	}
};

void C_ActorList::DebugDumpActors(void* eLevel)
{
	const int debugDumpLevel = (int)(INTPTR)eLevel;	// [Blur2PC][x64] level carried in the pointer

	C_DebugActorListDumpVisitor visitor;

	if (s_pLastActorList != NULL)
	{
		s_pLastActorList->VisitActors(visitor);
	}

	Debug::Logger& log = GetDebugLogGeneral();

	LOGINFO(log, "DumpActors: %d total actors", visitor.m_Count);

	for (C_DebugActorListDumpVisitor::C_Data::ConstIterator i = visitor.m_Data.Begin(); i != visitor.m_Data.End(); ++i)
	{
		LOGINFO(log, "    %10d %s actors", i->GetValue().Size(), s_pLastActorList->m_pActorFactory->LookupName(i->GetKey()).Chars());

		if (debugDumpLevel != E_DebugDumpLevel_Count)
		{
			for (C_DebugActorListDumpVisitor::C_Actors::ConstIterator j = i->GetValue().Begin(); j != i->GetValue().End(); ++j)
			{
				LOGINFO(log, "                %s", (*j)->GetActorName());

				if (debugDumpLevel != E_DebugDumpLevel_Name)
				{
					LOGINFO(log, "                    Components:");

					for (C_Actor::C_ComponentInterfaces::ConstIterator k = (*j)->m_ComponentInterfaces.Begin(); k != (*j)->m_ComponentInterfaces.End(); ++k)
					{
						LOGINFO(log, "                        0x%8x    %s", k->second->GetComponentTypeID(), Component::C_GameComponentFactory::GetInstance()->GetComponentIFName(k->first).Chars());
					}

#ifndef CFG_SPEED
					LOGINFO(log, "                    Subscribers:");
					(*j)->DebugDumpSubscribers(log, "                        ");
#endif
				}
			}
		}
	}
}

void C_ActorList::DebugDumpComponents(void* eLevel)
{
	const int debugDumpLevel = (int)(INTPTR)eLevel;	// [Blur2PC][x64] level carried in the pointer

	typedef Btl::Vector<const Actor::I_ActorComponent*> C_Components;
	typedef Btl::Map<Component::C_ComponentTypeId, C_Components> C_ComponentData;

	C_ComponentData data;
	U32 count = 0;
	{
		C_DebugActorListDumpVisitor visitor;

		if (s_pLastActorList != NULL)
		{
			s_pLastActorList->VisitActors(visitor);
		}


		for (C_DebugActorListDumpVisitor::C_Data::ConstIterator i = visitor.m_Data.Begin(); i != visitor.m_Data.End(); ++i)
		{
			for (C_DebugActorListDumpVisitor::C_Actors::ConstIterator j = i->GetValue().Begin(); j != i->GetValue().End(); ++j)
			{
				for (C_Actor::C_ComponentInterfaces::ConstIterator k = (*j)->m_ComponentInterfaces.Begin(); k != (*j)->m_ComponentInterfaces.End(); ++k)
				{
					++count;
					data[k->second->GetComponentTypeID()].PushBack(k->second);
				}
			}
		}
	}

	Debug::Logger& log = GetDebugLogGeneral();

	LOGINFO(log, "DumpComponents: %d total components", count);

	for (C_ComponentData::ConstIterator i = data.Begin(); i != data.End(); ++i)
	{
		LOGINFO(log, "    %10d  0x%8x    %s", i->GetValue().Size(), i->GetKey(), Component::C_GameComponentFactory::GetInstance()->GetComponentIFName((*i->GetValue().First())->GetInterfaceID()).Chars());

		if (debugDumpLevel != E_DebugDumpLevel_Count)
		{
			for (C_Components::ConstIterator j = i->GetValue().Begin(); j != i->GetValue().End(); ++j)
			{
				LOGINFO(log, "                  %s    %s", s_pLastActorList->GetActorFactory()->LookupName((*j)->GetActor()->GetActorTypeId()).Chars(), (*j)->GetActor()->GetActorName());

#ifndef CFG_SPEED
				if (debugDumpLevel != E_DebugDumpLevel_Name)
				{
					(*j)->DebugDumpSubscribers(log, "                      ");
				}
#endif
			}
		}
	}
}

void C_ActorList::DebugDumpSubscribers(void* eLevel)
{
	const int debugDumpLevel = (int)(INTPTR)eLevel;	// [Blur2PC][x64] level carried in the pointer

	LOGINFO(GetDebugLogGeneral(), "Dumping ActorList message subscribers...");

#ifndef CFG_SPEED
	if (s_pLastActorList != NULL)
	{
		s_pLastActorList->m_pMessageSubscriberSystem->DumpSubscribers(debugDumpLevel != E_DebugDumpLevel_Count);
	}
#endif
}

#endif // ndef CFG_GOLD

} // namespace Actor


#if defined(ACTOR_SYSTEM_OPTIMIZE_OFF)
#pragma optimize( "", on ) 
#endif
