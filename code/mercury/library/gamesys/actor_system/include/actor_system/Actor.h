#ifndef ACTOR_H
#define ACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <actor_system/HandleItem.h>
#include <actor_system/MemoryTracker.h>
#include <actor_system/SerialisationConfig.h>
#include <btl/containers/vector.h>
#include <btl/utility/pair.h>
#include <misc_shared/context/I_Anything.h>
#include <misc_shared/DataRef/Ptr.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <system/Debug/I_DebugRender.h>


struct C_ActorComponentMessageTypeHandler;

namespace Sys
{
	class C_ContextualStream;
}

// defining DEBUG_COMPONENTS causes the XComponentsDebug methods to do something
#ifndef CFG_SPEED
	#define DEBUG_COMPONENTS
	#define ACTOR_NAME_DEBUG
#endif


namespace Actor
{

/// Actor base class.
class C_Actor : public Util::C_HandleItem<C_Actor>, public Messaging::C_MessageHandler
{
	friend class C_ActorList;

public:
	C_Actor();
	virtual ~C_Actor();

	const DataRef::C_Ptr<C_ActorCreationData>&	GetCreationData() const {return m_pActorCreateData;}
	ActorSys::E_LifetimeState		GetLifetimeState() const {return m_LifetimeState;}

	// Static methods for factory lookup
	static const C_ActorTypeId		TypeId()        {return E_ActorTypeId_Actor;}
	static const C_ActorTypeName	TypeName()		{return "Actor";}

	// Factory related
	C_ActorTypeId					GetActorTypeId() const 	{return m_Type;}
	virtual const char*				GetActorName() const;
	//virtual const Maths::HMat43 	GetCreationMatrix() const;

	void							Kill();

	// Context related
	const Sys::I_Anything*			GetContext(const Sys::C_AnythingUid contextId) const;

	template <class T_Context>
	inline const T_Context*			GetContext() const;

	// query methods
	const bool			            FlagAsleep() const				{return TestActorFlag(E_ActorFlag_Asleep);}
	const bool			            FlagUpdate() const				{return TestActorFlag(E_ActorFlag_Update);}
	const bool			            FlagSpawned() const				{return TestActorFlag(E_ActorFlag_Spawned);}

	virtual const Maths::C_Vertex	GetPosition(void) const;
	virtual const Maths::C_Vector	GetDirection(void) const;
	virtual const Maths::HMat43&	GetMatrix(void) const;

	// Component interfacing
	bool							SpawnComponent(const DataRef::C_Ptr<Component::I_ComponentCreator>& pCreator);
	bool							AddComponent(I_ActorComponent& component);
	bool							RemoveComponent(const Component::C_ComponentInterfaceId interfaceID);
	const I_ActorComponent*			QueryInterface(const Component::C_ComponentInterfaceId interfaceID) const;

	template <class T_ActorComponent>
	inline const T_ActorComponent*	QueryInterface() const;

	template <class T_ActorComponent>
	inline bool						QueryInterface(const T_ActorComponent** ppComponent) const;

	U32								GetNumComponents() const;
	const I_ActorComponent*			GetComponent(const U32 index) const;

	// Message Handling        
	U32								ProcessMessage(const Messaging::C_MessageBase* pMessage, const S32 component = -1);
	virtual bool                    MessageHandler(const Messaging::C_MessageBase* pMessage) OVERRIDE SEALED; // from C_MessageHandler
	virtual bool					OnMessage(const Messaging::C_MessageBase* pMessage); ///< actor message processing
	bool							OnMessageSimple(const Messaging::TMsgType msgType); ///< actor message processing

	// Message Sending
	E_ActorMessageSendResult		SendMessageToActor(const C_ActorHandle dest, const Messaging::C_MessageBase *pMessage, S32 msgQueue = -1) const;
	inline E_ActorMessageSendResult	SendMessageToActor(const C_ActorHandle dest, Messaging::TMsgType msgType, S32 msgQueue = -1) const;
	E_ActorMessageSendResult		SendMessageToComponent(Component::C_ComponentInterfaceId componentIf, const Messaging::C_MessageBase *pMessage, S32 msgQueue = -1) const;
	inline E_ActorMessageSendResult	SendMessageToComponent(Component::C_ComponentInterfaceId componentIf, Messaging::TMsgType msgType, S32 msgQueue = -1) const;
	void							BroadcastMessage(const Messaging::C_MessageBase *pMessage, U32 mask = 0xffffffff) const;	
	inline void						BroadcastMessage(Messaging::TMsgType msgType, U32 mask = 0xffffffff) const ;

	// Categories
	// Actor can be in many categories and can change category during play
	void                            SetCategory(const U32 categoryBits)		{m_ActorCategory |= categoryBits; }
	void                            ClearCategory(const U32 categoryBits)	{m_ActorCategory &= ~categoryBits; }
	const U32                       Category() const						{return m_ActorCategory; }
	const bool						CheckCategory(const U32 categoryBits) const	{ return (m_ActorCategory & categoryBits) == categoryBits;}
	inline const bool				MatchCategory(const U32 matchAnyCategory = 0, const U32 matchAllCategory = 0, const U32 matchNoneCategory = 0) const;

	// system accessor functions
	const C_Actor *					GetActorPtr(const C_ActorHandle actorHandle) const;
	C_ActorList*		    		GetActorList() const {return m_pActorList;}
	virtual void					PrintDebugInfo() const {}

	// Serialisation
	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

	void							SerialiseComponentCreation(Sys::C_ContextualStream& stream) const; ///< write component creation data
	void							SerialiseComponentsForSync(Sys::C_ContextualStream& stream) const; ///< write component data
	void							DeserialiseComponentForSync(Sys::C_ContextualStream& stream) const; ///< read component data

#ifndef CFG_GOLD
	// Debug Methods, Not for final build
	void							DebugRenderSelfAndComponents(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
	virtual void					DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

#ifndef CFG_SPEED
	virtual void					DebugDumpMessageHandler(Debug::Logger& log, const char* prefix) const; // from Messaging::C_MessageHandler
#endif

#if ACTOR_SYSTEM_MEMORY_TRACKING
	const char*						m_MemoryTrackerName;
	Memory::ContextId				m_MemoryTrackerId;
	ActorSys::C_MemoryTracker		GetMemoryTracker() const {return ActorSys::C_MemoryTracker(m_MemoryTrackerId);}
#endif // ACTOR_SYSTEM_MEMORY_TRACKING

public:
	I_ActorComponent*				InitialiseComponent(const DataRef::C_Ptr<Component::I_ComponentCreator>& pCreationData);
protected:
	I_ActorComponent*				InitialiseComponentFromSorcery(const Component::I_ComponentCreator* pCreationData);

	void							SetPrimed(); ///< called by derived classes when all resources have loaded

	//void							SetActiveGroup(const bool yn);

	// E_ActorFlags access
	inline const bool				TestActorFlag(const E_ActorFlags flag) const;
	inline void						SetActorFlag(const E_ActorFlags flag, const bool value);

	// should be overridden by derived classes where appropriate
	virtual bool					InitialiseImpl();	///< Called when the actor moves from the Constructed state.
	virtual bool					PrimeImpl();		///< Called when the actor moves from the Initialised state.
	virtual bool					CreateImpl();		///< Called when the actor moves from the Primed state.
	virtual void					KillImpl();			///< Called if the actor is killed.

	virtual void					InternalDependenciesCreatedImpl();	///< Called when all components have had internal dependencies created.
	virtual void					ExternalDependenciesCreatedImpl();	///< Called when all related actors have been created.

private:
	typedef Btl::Pair<Component::C_ComponentInterfaceId, I_ActorComponent*> C_ComponentInterface;
	typedef Btl::Vector<C_ComponentInterface> C_ComponentInterfaces;
	typedef Btl::Vector<C_ActorComponentMessageTypeHandler> C_MessageTypeComponentHandlers;

	DataRef::C_Ptr<C_ActorCreationData>	m_pActorCreateData;		///< data used to create actor
	ActorSys::E_LifetimeState			m_LifetimeState;		///< current lifetime state
	C_ActorTypeId						m_Type;					///< type identifier for manual RTTI
	C_ActorList*						m_pActorList;			///< Actor list that owns us
	U32									m_ActorFlags;			///< Actor system flags
	U32									m_ActorCategory;		///< user defined bitmask of Actor Category flags
	S32									m_ActorGroup;			///< user-defined group number
	C_ComponentInterfaces				m_ComponentInterfaces;	///< list of component interfaces
	C_MessageTypeComponentHandlers		m_SpecificMsgHandlers;	///< components that handle specific messages
#ifdef DEBUG_COMPONENTS
	mutable U32							m_ComponentsLockCount;	///< number of active locks
#endif
#ifdef ACTOR_NAME_DEBUG
	Util::C_String						m_Name;
#endif

									C_Actor(const C_Actor&);		// can not be copied
	C_Actor&						operator = (const C_Actor&);	// can not be copied

	void							SetLifetimeState(const ActorSys::E_LifetimeState state) {m_LifetimeState = state;}

	I_ActorComponent*				PreInitialiseComponent(const DataRef::C_Ptr<Component::I_ComponentCreator>& pCreationData);
	I_ActorComponent*				PostInitialiseComponent(I_ActorComponent* pComponent, const bool initialised);

	bool							CreateComponent(const Component::C_ComponentInterfaceId interfaceId);
	void							ComponentCreated(const Component::C_ComponentInterfaceId interfaceId);

	void							PollForPrimed();

	// DEBUG_COMPONENTS stuff
	inline void						LockComponentsDebug() const;
	inline void						UnlockComponentsDebug() const;
	inline void						CheckUnlockedComponentsDebug() const;

	// C_ActorList friend interface
	void							SetCreationData(const DataRef::C_Ptr<C_ActorCreationData>& pCreationData);
	bool							Initialise();
	bool							Initialise(Sys::C_ContextualStream& stream);
	bool							Enslave();
	bool							Prime();
	bool							Create();

	void							InternalDependenciesCreated();	///< Called when all related actors have been created.
	void							ExternalDependenciesCreated();	///< Called when all related actors have been created.

};


// downcasting functions
template <class DerivedActor> inline const DerivedActor*	ActorCast_Safe(const C_Actor* pActor);
template <class DerivedActor> inline DerivedActor*			ActorCast_Safe(C_Actor* pActor);
template <class DerivedActor> inline const DerivedActor*	ActorCast_Assert(const C_Actor* pActor);
template <class DerivedActor> inline DerivedActor*			ActorCast_Assert(C_Actor* pActor);
template <class DerivedActor> inline const DerivedActor*	ActorCast_Reckless(const C_Actor* pActor);
template <class DerivedActor> inline DerivedActor*			ActorCast_Reckless(C_Actor* pActor);
template <class DerivedActor> inline const DerivedActor*	ActorCast(const C_Actor* pActor);
template <class DerivedActor> inline DerivedActor*			ActorCast(C_Actor* pActor);

} // namespace Actor


#define ACTOR_INL
#include "Actor.inl"
#undef ACTOR_INL

#endif
