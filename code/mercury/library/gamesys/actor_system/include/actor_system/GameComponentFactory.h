#ifndef GAMECOMPONENTFACTORY_H
#define GAMECOMPONENTFACTORY_H

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


namespace Component
{

class I_ComponentCreatorFactoryContext : public Sys::C_Anything<'CCDF'>
{
public:
	virtual U32 GetIndex(const I_ComponentCreator* pData) const = 0;
	virtual DataRef::C_Ptr<I_ComponentCreator> Lookup(const U32 index) const = 0;

	static const U32 kInvalidIndex = 0xffffffff;

};

/// Factory singleton for component creation data.
class C_ComponentCreatorFactory
{
public:
	typedef I_ComponentCreator*	(*F_Create) (); ///< Allocates memory and invokes the default ctor.
	typedef bool				(*F_Destroy) (const I_ComponentCreator* pData); ///< Invokes the dtor and frees the memory.
	
	template <class T_ComponentCreator>
	static I_ComponentCreator*	DefaultCreateFn() {return new T_ComponentCreator;} ///< Default implementation of F_Create.
	static bool					DefaultDestroyFn(const I_ComponentCreator* pCreationData); ///< Default implementation of F_Destroy

	/**
	@brief Registers factory methods for this component type.
	@param pCreate creates creation data for this component type.
	@param pDestroy destroys creation data for this component type.
	*/
	template <class T_ComponentCreator>
	static void					Register(F_Create pCreate = &DefaultCreateFn<T_ComponentCreator>, F_Destroy pDestroy = &DefaultDestroyFn)
	{
#ifndef CFG_SPEED
		InsertItem(T_ComponentCreator::ClassType(), pCreate, pDestroy, ::GetTypeChecker<T_ComponentCreator>());
#else
		InsertItem(T_ComponentCreator::ClassType(), pCreate, pDestroy, NULL);
#endif
	}

public: // stuff to comply with DataRef::FactoryXXX functions.
	typedef I_ComponentCreator	C_DataType;
	typedef C_ComponentTypeId	C_TypeId;
	typedef I_ComponentCreatorFactoryContext C_StreamContext;

	static I_ComponentCreator*	Create(const C_ComponentTypeId typeId);
	static void					Destroy(const I_ComponentCreator* pData);

	static C_ComponentTypeId	GetTypeOf(const I_ComponentCreator& data);

#ifndef CFG_SPEED
	static const C_TypeChecker*	GetTypeChecker(const C_ComponentTypeId typeId);
#endif

private:
	C_ComponentCreatorFactory(); // can not instantiate
	~C_ComponentCreatorFactory(); // can not instantiate

	C_ComponentCreatorFactory(const C_ComponentCreatorFactory& copy); // can not copy
	C_ComponentCreatorFactory& operator = (const C_ComponentCreatorFactory& copy); // can not copy

	static void					InsertItem(const C_ComponentTypeId typeId, const F_Create pCreate, const F_Destroy pDestroy, const C_TypeChecker* pTypeChecker);

};


/// Factory singleton for components.
class C_GameComponentFactory
{
public:
	/// @returns Singleton instance.
	static C_GameComponentFactory*	GetInstance()
	{
		static C_GameComponentFactory instance;
		return &instance;
	}

	I_GameComponent*				CreateComponent(const C_ComponentTypeId typeId, const char* memoryTrackerParentName = NULL);
	bool							DestroyComponent(I_GameComponent* pComponent);

	C_ComponentInterfaceId			GetComponentInterface(const C_ComponentTypeId componentType) const;

	typedef I_GameComponent*		(*F_Create) (); ///< Allocates memory and calls the default constructor.
	typedef bool					(*F_Destroy) (I_GameComponent* pComponent); ///< Calls destructor and frees memory.

	template <class T_ComponentType>
	static I_GameComponent*			DefaultCreateFn() {return new T_ComponentType;} ///< Default implementation of F_Create.
	static bool						DefaultDestroyFn(I_GameComponent* pComponent); ///< Default implementation of F_Destroy.

	/**
	@brief Registers a component class with the factory.
	@param pCreate function to create a component.
	@param pDestroy function to destroy a component.
	@returns true if the component type was registered successfully.
	*/
	template <class T_ComponentType>
	bool							Register(F_Create pCreate = &DefaultCreateFn<T_ComponentType>, F_Destroy pDestroy = &DefaultDestroyFn)
	{
		return RegisterImpl(T_ComponentType::ClassType(), T_ComponentType::InterfaceType(), pCreate, pDestroy);
	}

	void							RegisterComponentIFName(const C_ComponentInterfaceId interfaceID, const Btl::String& name);
	const Btl::String&				GetComponentIFName(const C_ComponentInterfaceId interfaceID) const;

private:
	class							C_Impl;

	C_Impl*							m_pImpl;

	C_GameComponentFactory(); // singleton only
	~C_GameComponentFactory(); // singleton only

	C_GameComponentFactory(const C_GameComponentFactory&); // can not copy
	C_GameComponentFactory& operator = (const C_GameComponentFactory&); // can not copy

	bool							RegisterImpl(const C_ComponentTypeId componentType, const C_ComponentInterfaceId interfaceType, F_Create pCreate, F_Destroy pDestroy, const char* memoryTrackerName = NULL);

};

} // namespace Component


#endif // GAMECOMPONENTFACTORY_H
