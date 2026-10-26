#include "ActorSystemPch.h"

#include <actor_system/ActorFactory.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorSys.h>
#include <core/BizAssert.h>
#include <misc_shared/DataRef/Factory.h>
#include <misc_shared/uid/TypeTable.h>





#if defined(ACTOR_SYSTEM_OPTIMIZE_OFF)
#pragma optimize( "", off ) 
#endif




namespace ActorCreationDataFactory_Impl
{

struct C_Data
{
	Actor::C_ActorCreationDataFactory::F_Create		m_pCreate;
	Actor::C_ActorCreationDataFactory::F_Destroy	m_pDestroy;
#ifndef CFG_SPEED
	const C_TypeChecker*							m_pTypeChecker;
#endif
};

typedef UID::C_TypeTableHashPolicyHi20<Actor::C_ActorTypeId, 128> C_HashPolicy;
typedef UID::C_TypeTable<Actor::C_ActorTypeId, C_Data, C_HashPolicy> C_TypeTable;

static C_TypeTable g_TypeTable;

} // namespace ActorCreationDataFactory_Impl




namespace Actor
{

#if ACTOR_SYSTEM_MEMORY_TRACKING

const char* kMiscActorMemoryTrackerName = "Misc";

#endif // ACTOR_SYSTEM_MEMORY_TRACKING




/**
@class C_ActorCreationDataFactory
@ingroup actor_group_factory

Clients should call Register() for each actor type that requires dynamic 
creation data.
@code
Actor::C_ActorCreationDataFactory::Register<C_MyActor>(E_ActorTypes_MyActor);
@endcode

Creation data can then be created dynamically using DataRef::FactoryCreate().
@code
DataRef::C_Ptr<Actor::C_ActorCreationData> ptr = DataRef::FactoryCreate<Actor::C_ActorCreationDataFactory>(E_ActorTypes_MyActor);
@endcode
*/

/**
@param pCreationData creation data to destroy.
@returns true if the data was destroyed.
*/
bool C_ActorCreationDataFactory::DefaultDestroyFn(const C_ActorCreationData* pCreationData)
{
	delete pCreationData;
	return true;
}

/**
For DataRef::FactoryXXX functions.
@param typeId type of actor to make creation data for.
@returns creation data.
*/
C_ActorCreationData* C_ActorCreationDataFactory::Create(const C_ActorTypeId typeId)
{
	using namespace ActorCreationDataFactory_Impl;
	const C_TypeTable::ConstIterator it = g_TypeTable.Find(typeId);
	return (it == g_TypeTable.End()) ? NULL : it.GetData().m_pCreate();
}

/**
For DataRef::FactoryXXX functions.
@param pData creation data to destroy.
*/
void C_ActorCreationDataFactory::Destroy(const C_ActorCreationData* pData)
{
	if (pData != NULL)
	{
		using namespace ActorCreationDataFactory_Impl;
		const C_TypeTable::ConstIterator it = g_TypeTable.Find(pData->GetType());

		if (it != g_TypeTable.End())
		{
			it.GetData().m_pDestroy(pData);
		}
	}
}

/**
For DataRef::FactoryXXX functions.
@param data creation data to get actor type from.
@returns actor type created by this data.
*/
C_ActorTypeId C_ActorCreationDataFactory::GetTypeOf(const C_ActorCreationData& data)
{
	return data.GetType();
}

#ifndef CFG_SPEED

/**
@param typeId concrete actor type.
@returns type checker for this actor type or NULL if not known.
*/
const C_TypeChecker* C_ActorCreationDataFactory::GetTypeChecker(const C_ActorTypeId typeId)
{
	using namespace ActorCreationDataFactory_Impl;
	const C_TypeTable::ConstIterator it = g_TypeTable.Find(typeId);
	return (it == g_TypeTable.End()) ? NULL : it.GetData().m_pTypeChecker;
}

#endif // CFG_SPEED

/**
@brief Adds the details for this actor type.
@param actorType actor type to register.
@param pCreate creation function.
@param pDestroy destruction function.
@param pTypeChecker final type of object to create.
*/
void C_ActorCreationDataFactory::InsertItem(const C_ActorTypeId actorType, const F_Create pCreate, const F_Destroy pDestroy, const C_TypeChecker* pTypeChecker)
{
	using namespace ActorCreationDataFactory_Impl;
	
	const C_TypeTable::InsertionResult result = g_TypeTable.Insert(actorType);

	if (result.first)
	{
		result.second.GetData().m_pCreate = pCreate;
		result.second.GetData().m_pDestroy = pDestroy;
#ifndef CFG_SPEED
		result.second.GetData().m_pTypeChecker = pTypeChecker;
#endif
	}
}




/**
@class C_ActorFactory
@ingroup actor_group_factory

Users should register each actor class using the templated Register() method, 
which makes use of the manual RTTI system to associate factory functions with 
the actor type.
@code
m_ActorFactory.Register<C_CharacterActor>();
m_ActorFactory.Register<C_CameraActor>();
m_ActorFactory.Register<C_RoyalShakespearCompanyActor>();
@endcode

Registered actor types can then be instantiated with CreateActor() and killed
with DestroyActor().
*/

class C_ActorFactory::C_Impl
{
public:
	struct C_Data
	{
		C_ActorTypeName m_ActorTypeName;		///< actor type name
		F_Create		m_pCreate;				///< creates actors
		F_Destroy		m_pDestroy;				///< destroys actors
#if ACTOR_SYSTEM_MEMORY_TRACKING
		const char*		m_MemoryTrackerName;	///< for memory tracking
#endif
	};

	typedef UID::C_TypeTableHashPolicyHi20<Actor::C_ActorTypeId, 128> C_HashPolicy;
	typedef UID::C_TypeTable<Actor::C_ActorTypeId, C_Data, C_HashPolicy> C_TypeTable;

	C_TypeTable m_TypeTable;

};

/**
@param maxActorTypes maximum number of actor types that can be registered.
*/
C_ActorFactory::C_ActorFactory(const U32 maxActorTypes)
	: m_pImpl(NULL)
{
	// TODO: either deprecate maxActorTypes or change type table to use a static pool
	
	m_pImpl = new C_Impl;
}

C_ActorFactory::~C_ActorFactory()
{
	delete m_pImpl;
}

/**
@param pActor actor to destroy.
@returns true if the actor was destroyed.
*/
bool C_ActorFactory::DefaultDestroyFn(C_Actor* pActor)
{
	delete pActor;
	return true;
}

/**
@brief Creates an actor object.
@param actorType type of actor to create.
@returns NULL if the type is not recognised.
*/
C_Actor* C_ActorFactory::CreateActor(const C_ActorTypeId actorType)
{
	const C_Impl::C_TypeTable::ConstIterator it = m_pImpl->m_TypeTable.Find(actorType);

	if (it != m_pImpl->m_TypeTable.End())
	{
		BIZ_ASSERT(it.GetUID() == actorType);
#if ACTOR_SYSTEM_MEMORY_TRACKING
		BIZ_ASSERT(it.GetData().m_MemoryTrackerName != NULL);
		char buffer [256];
		sprintf_s(buffer, sizeof(buffer), "actors.%s", it.GetData().m_MemoryTrackerName);
		const Memory::ContextId memoryTrackerId = Memory::TrackNodes::Instance().GetId(buffer);
		const ActorSys::C_MemoryTracker memoryTracker(memoryTrackerId);
#endif // ACTOR_SYSTEM_MEMORY_TRACKING

		C_Actor* pActor = it.GetData().m_pCreate();

#if ACTOR_SYSTEM_MEMORY_TRACKING
		pActor->m_MemoryTrackerName = it.GetData().m_MemoryTrackerName;
		pActor->m_MemoryTrackerId = memoryTrackerId;
#endif // ACTOR_SYSTEM_MEMORY_TRACKING

		return pActor;
	}

	LOGERROR(GetDebugLogGeneral(), DBSTR " actor create type %d unregistered", actorType);
	return NULL;
}

/**
@brief Destroys an actor object.
@param pActor actor to destroy.
@returns true if the actor was destroyed.
*/
bool C_ActorFactory::DestroyActor(C_Actor* pActor)
{
	BIZ_ASSERT(pActor != NULL);
	const C_Impl::C_TypeTable::ConstIterator it = m_pImpl->m_TypeTable.Find(pActor->GetActorTypeId());
	return (it != m_pImpl->m_TypeTable.End()) ? it.GetData().m_pDestroy(pActor) : false;
}

/**
@brief Finds the type identifier for this actor type.
@param actorTypeName name of the actor type.
@returns E_ActorTypeId_Null if this type name has not been registered.
*/
const C_ActorTypeId C_ActorFactory::LookupId(const C_ActorTypeName& actorTypeName) const
{
	const C_Impl::C_TypeTable::ConstIterator end = m_pImpl->m_TypeTable.End();

	for (C_Impl::C_TypeTable::ConstIterator it = m_pImpl->m_TypeTable.Begin(); it != end; ++it)
	{
		if (it.GetData().m_ActorTypeName == actorTypeName)
		{
			return it.GetUID();
		}
	}

	return E_ActorTypeId_Null;
}

/**
@brief Finds the type name for this actor type.
@param actorTypeId unique identifier of the actor type.
@returns "NULL" if this type identifier has not been registerd.
*/
const C_ActorTypeName C_ActorFactory::LookupName(const C_ActorTypeId actorTypeId) const
{
	const C_Impl::C_TypeTable::ConstIterator it = m_pImpl->m_TypeTable.Find(actorTypeId);
	return (it != m_pImpl->m_TypeTable.End()) ? it.GetData().m_ActorTypeName : C_ActorTypeName("NULL");
}

/**
@brief Adds an actor type to the factory.
@param actorType type identifier.
@param typeName type name.
@param pCreate function to create actors.
@param pDestroy function to destroy actors.
@param memoryTrackerName only used if ACTOR_SYSTEM_MEMORY_TRACKING is non-zero.
*/
void C_ActorFactory::InsertItem(const C_ActorTypeId actorType, const C_ActorTypeName typeName, const F_Create pCreate, const F_Destroy pDestroy, const char* memoryTrackerName)
{
	C_Impl::C_TypeTable::InsertionResult result = m_pImpl->m_TypeTable.Insert(actorType);
	BIZ_ASSERT(result.second.GetUID() == actorType);
	if (result.first)
	{
		result.second.GetData().m_ActorTypeName = typeName;
		result.second.GetData().m_pCreate = pCreate;
		result.second.GetData().m_pDestroy = pDestroy;
#if ACTOR_SYSTEM_MEMORY_TRACKING
		result.second.GetData().m_MemoryTrackerName = memoryTrackerName;
#endif // ACTOR_SYSTEM_MEMORY_TRACKING
	}
	else
	{
		LOGERROR(GetDebugLogGeneral(), DBSTR " Actor type already registered!");
	}
}

} // namespace Actor





#if defined(ACTOR_SYSTEM_OPTIMIZE_OFF)
#pragma optimize( "", on ) 
#endif
