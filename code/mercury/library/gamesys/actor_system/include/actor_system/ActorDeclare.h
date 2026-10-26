#ifndef ACTORDECLARE_H
#define ACTORDECLARE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/Handle.h>

#include <Btl/Allocators/pool.h>
#include <Btl/Containers/list.h>
#include <Btl/Containers/map.h>
#include <Btl/Utility/string.h>

#define MAX_NO_ACTORS		6000

#ifdef CFG_SPEED
 #define ACTOR_SYSTEM_MEMORY_TRACKING 0
#else
 #define ACTOR_SYSTEM_MEMORY_TRACKING 1
#endif

#ifdef CFG_SPEED
 #define DEBUG_ACTOR_SERIALISE_FOR_SAVE 0
#else
 #define DEBUG_ACTOR_SERIALISE_FOR_SAVE 1
#endif


namespace Util
{
	class C_StaggerManager;

} // namespace Util


namespace ActorSys
{
	/// @brief Lifetime of actors and components.
	/// @ingroup actor_group_main
	/// Actor and component objects go through several states between 
	/// construction and destruction. A newly Constructed object must be
	/// Initialised using creation data, which may happen in different ways.
	/// Any resources that are needed must be loaded, and the object may be 
	/// Priming for a number of frames before this happens. Once the object
	/// is Primed it can finally be Created. Any of these phases could fail,
	/// in which case the object would be Failed. Finally, an object may be 
	/// Killed before destruction.
	enum E_LifetimeState
	{
		E_LifetimeState_Constructed = 0,	///< newly created
		E_LifetimeState_Initialised,		///< initialised with data
		E_LifetimeState_Priming,			///< resources have been requested
		E_LifetimeState_Primed,				///< resources have loaded
		E_LifetimeState_Created,			///< fully alive, able to receive messages, etc.
		E_LifetimeState_Killed,				///< marked for destruction
		E_LifetimeState_Failed,				///< some process has failed, should be killed
		E_LifetimeState_Count
	};

#if ACTOR_SYSTEM_MEMORY_TRACKING
	class C_MemoryTracker;	///< wrapper for memory tracking
#endif // ACTOR_SYSTEM_MEMORY_TRACKING

} // namespace ActorSys


namespace Component
{
	class C_ComponentCreationFactory;
	class C_GameComponentFactory;
	class I_GameComponent;
	class I_ComponentCreator;

	/// @brief Unique component interface identifier.
	/// @ingroup actor_group_component
	/// User-defined identifier for component interfaces. There can be any number
	/// of concrete component types that implement the same interface.
	/// The special value of -1 may be used to signify no specific interface, for 
	/// example when filtering components by interface.
	typedef S32 C_ComponentInterfaceId;

	/// @brief Unique component type identifier.
	/// @ingroup actor_group_component
	/// User-defined identifier for concrete component types. There should only be
	/// one component type for each identifier.
	typedef S32 C_ComponentTypeId;

	/// @brief Callback for component creation data in sorcery database.
	/// @param pCreator sorcery data receiving event.
	typedef void (*F_ComponentCreatorSorceryCallback)(const Component::I_ComponentCreator* pCreator);

} // namespace Component


namespace Actor
{
	struct C_ActorCreationConfig;

	class C_Actor;
	class C_ActorCreationData;
	class C_ActorCreationListComposite;
	class C_ActorCreationListMultiple;
	class C_ActorCreationListSingle;
	class C_ActorCreationStatus;
	class C_ActorFactory;
	class C_ActorHandleListItem;
	class C_ActorList;
	class C_ActorMessageStoreBase;
	class C_DeferredActorMessageStore;
	class C_DeferredActorMessageQueue;
	class C_DeferredActorMessages;
	class C_DelayedActorMessageStore;
	class C_DelayedActorMessageQueue;
	class C_DelayedActorMessages;
	class C_GroupHandleList;
	class I_ActorComponent;
	class I_ActorCreationData;
	class I_ActorCreationList;
	class I_ActorHandleSerialisation;

	/// system flags
	enum E_ActorFlags
	{
		E_ActorFlag_Update			= 0x00000001,	///< Actor gets updated
		E_ActorFlag_Asleep			= 0x00000002,	///< Actor is asleep
		E_ActorFlag_ActorPriming	= 0x00000004,	///< Actor returned false from PrimeImpl, awaiting SetPrimed call
		E_ActorFlag_Delete			= 0x00000008,	///< Actor is to be deleted by the system
		E_ActorFlag_Spawned			= 0x00000010,	///< Actor has been spawned and must delete it's creation data
		E_ActorFlag_ForceDWord		= 0xffffffff
	};

	/// @brief Provides C_ActorTypeId for system actors.
	/// Users should not define C_ActorTypeId values that clash with these.
	enum E_ActorTypeId_System
	{
		E_ActorTypeId_Null = 0,		///< NULL actor identifier, no actor object should ever return this type
		E_ActorTypeId_Actor,		///< base Actor identifier used by C_Actor
		E_ActorTypeId_System_Count
	};

	/// @brief Return values for actor messaging.
	/// @ingroup actor_group_messages
	enum E_ActorMessageSendResult
	{
		E_ActorMessageSendResult_OK = 0,		///< the message will be sent to the actor
		E_ActorMessageSendResult_NoActor,		///< the actor that the message was intended for does not exist
		E_ActorMessageSendResult_ActorKilled,	///< the actor that the message was intended for is no longer alive
		E_ActorMessageSendResult_Count
	};

	/// @brief Unique class type identifier.
	/// @ingroup actor_group_main
	/// Each actor class has a user-defined type identifier for manual RTTI. Some
	/// values are reserved for system use in E_ActorTypeId_System.
	typedef U32 C_ActorTypeId;

	/// @brief Unique class type name.
	/// @ingroup actor_group_main
	/// In addition to C_ActorTypeId, each actor class also has a unique human-
	/// readable name assigned by the user. 
	typedef Btl::String32 C_ActorTypeName;

	/// @brief Handle type for actors.
	/// @ingroup actor_group_main
	/// Users should not store pointers to Actor objects, but should instead store
	/// C_ActorHandle values. These can be maniulated using the C_ActorList.
	typedef Util::C_Handle<C_Actor> C_ActorHandle;

	typedef Btl::List<C_ActorHandleListItem> C_ActorHandleList; ///< list containing actor handles

	typedef Btl::List< C_DeferredActorMessageStore, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter, Btl::ListElementSingle, Btl::StandardAllocator<Btl::NullType>, Btl::Internal> > C_DeferredActorMessageList; ///< list for storing C_DeferredActorMessageStore objects
	typedef Btl::DynamicPool<C_DeferredActorMessageStore> C_DeferredActorMessagePool; ///< pool for allocating C_DeferredActorMessageStore objects
	typedef Btl::List<C_DelayedActorMessageStore> C_DelayedActorMessageList;	///< list for storing C_DelayedActorMessageStore objects
	typedef Btl::DynamicPool<C_DelayedActorMessageStore> C_DelayedActorMessagePool;	///< pool for allocating C_DelayedActorMessageStore objects

	/// @brief Callback for actor creation data in sorcery database.
	/// @param pCreationData sorcery data receiving event.
	typedef void (*F_ActorCreationDataSorceryCallback)(const C_ActorCreationData* pCreationData);

} // namespace Actor


#endif // ACTORDECLARE_H
