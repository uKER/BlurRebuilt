#ifndef ACTORFACTORY_H
#define ACTORFACTORY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <misc_shared/Context/I_Anything.h>
#include <misc_shared/DataRef/Ptr.h>
#include <misc_shared/uid/TypeChecker.h>


namespace Sys
{
	class C_ContextualStream;
}


namespace Actor
{

class I_ActorCreationDataFactoryContext : public Sys::C_Anything<'ACDF'>
{
public:
	virtual U32 GetIndex(const C_ActorCreationData* pData) const = 0;
	virtual DataRef::C_Ptr<C_ActorCreationData> Lookup(const U32 index) const = 0;

	static const U32 kInvalidIndex = 0xffffffff;

};

/// Factory singleton for actor creation data.
class C_ActorCreationDataFactory
{
public:
	typedef C_ActorCreationData*	(*F_Create)(); ///< Allocates memory and invokes the default ctor.
	typedef bool					(*F_Destroy)(const C_ActorCreationData* pData); ///< Invokes the dtor and frees the memory.

	template <class T_ActorCreationData>
	static C_ActorCreationData*		DefaultCreateFn() {return new T_ActorCreationData;} ///< Default implementation of F_Create.
	static bool						DefaultDestroyFn(const C_ActorCreationData* pCreationData); ///< Default implementation of F_Destroy

	/**
	@brief Registers factory methods for this actor type.
	@param typeId actor type.
	@param pCreate creates creation data for this actor type.
	@param pDestroy destroys creation data for this actor type.
	*/
	template <class T_ActorCreationData>
	static void						Register(const C_ActorTypeId typeId, F_Create pCreate = &DefaultCreateFn<T_ActorCreationData>, F_Destroy pDestroy = &DefaultDestroyFn)
	{
#ifndef CFG_GOLD
		InsertItem(typeId, pCreate, pDestroy, ::GetTypeChecker<T_ActorCreationData>());
#else
		InsertItem(typeId, pCreate, pDestroy, NULL);
#endif
	}

public: // stuff to comply with DataRef::FactoryXXX functions.
	typedef C_ActorCreationData		C_DataType;
	typedef C_ActorTypeId			C_TypeId;
	typedef I_ActorCreationDataFactoryContext C_StreamContext;

	static C_ActorCreationData*		Create(const C_ActorTypeId typeId);
	static void						Destroy(const C_ActorCreationData* pData);

	static C_ActorTypeId			GetTypeOf(const C_ActorCreationData& data);

#ifndef CFG_SPEED
	static const C_TypeChecker*		GetTypeChecker(const C_ActorTypeId typeId);
#endif

private:
	C_ActorCreationDataFactory(); // can not instantiate
	~C_ActorCreationDataFactory(); // can not instantiate

	C_ActorCreationDataFactory(const C_ActorCreationDataFactory& copy); // can not copy
	C_ActorCreationDataFactory& operator = (const C_ActorCreationDataFactory& copy); // can not copy

	static void						InsertItem(const C_ActorTypeId actorType, const F_Create pCreate, const F_Destroy pDestroy, const C_TypeChecker* pTypeChecker);

};


#if ACTOR_SYSTEM_MEMORY_TRACKING

extern const char* kMiscActorMemoryTrackerName;

// specialise this before registering the actor type if you want memory tracking on that actor type
template <typename T_Actor>
inline const char* GetMemoryTrackerNameForActorType() {return kMiscActorMemoryTrackerName;}

#endif // ACTOR_SYSTEM_MEMORY_TRACKING


/// Factory object for actors.
class C_ActorFactory
{
public:
	struct C_ActorFactoryItem;

	C_ActorFactory(const U32 maxActorTypes);
	~C_ActorFactory();

	typedef C_Actor*		(*F_Create) (); ///< Allocates an actor object and invokes the default ctor.
	typedef bool			(*F_Destroy) (C_Actor* pActor); ///< Invokes the dtor and frees the object.
	
	template <class T> 
	static C_Actor*			DefaultCreateFn() {return new T();} ///< Default construction method for F_Create.
	static bool				DefaultDestroyFn(C_Actor* pActor); ///< Default destruction method for F_Destroy

	/**
	@brief Register function which you can pass constructor, destructor in.
	@param pCreate function to allocate and construct a T_Actor.
	@param pDestroy function to destruct and free a T_Actor.
	*/
	template <class T_Actor>
	inline void				Register(F_Create pCreate = &DefaultCreateFn<T_Actor>, F_Destroy pDestroy = &DefaultDestroyFn)
	{
		C_ActorTypeName actorTypeName = T_Actor::TypeName().Chars();
#if ACTOR_SYSTEM_MEMORY_TRACKING
		InsertItem(T_Actor::TypeId(), actorTypeName, pCreate, pDestroy, GetMemoryTrackerNameForActorType<T_Actor>());
#else
		InsertItem(T_Actor::TypeId(), actorTypeName, pCreate, pDestroy);
#endif
	}

	C_Actor*				CreateActor(const C_ActorTypeId actorType);
	bool					DestroyActor(C_Actor* pActor);

	const C_ActorTypeId		LookupId(const C_ActorTypeName& actorTypeName) const;
	const C_ActorTypeName	LookupName(const C_ActorTypeId actorTypeId) const;

private:
	class					C_Impl;

	C_Impl*					m_pImpl;

	C_ActorFactory(const C_ActorFactory& copy); // can not copy
	C_ActorFactory& operator = (const C_ActorFactory& copy); // can not copy

	void					InsertItem(const C_ActorTypeId actorType, const C_ActorTypeName typeName, const F_Create pCreate, const F_Destroy pDestroy, const char* memoryTrackerName = NULL);

};

} // namespace Actor


#endif
