#include "ActorSystemPch.h"

#include <actor_system/GameComponentFactory.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/GameComponent.h>
#include <actor_system/MemoryTracker.h>
#include <core/BizAssert.h>
#include <misc_shared/uid/TypeTable.h>





#if defined(ACTOR_SYSTEM_OPTIMIZE_OFF)
#pragma optimize( "", off ) 
#endif




namespace ComponentCreatorFactory_Impl
{

struct C_Data
{
	Component::C_ComponentCreatorFactory::F_Create	m_pCreate;
	Component::C_ComponentCreatorFactory::F_Destroy	m_pDestroy;
#ifndef CFG_SPEED
	const C_TypeChecker*							m_pTypeChecker;
#endif
};

typedef UID::C_TypeTableHashPolicyHi20<Component::C_ComponentTypeId, 128> C_HashPolicy;
typedef UID::C_TypeTable<Component::C_ComponentTypeId, C_Data, C_HashPolicy> C_TypeTable;

static C_TypeTable g_TypeTable;

} // namespace ComponentCreatorFactory_Impl




namespace GameComponentFactory_Impl
{

static const Btl::String g_UnregisteredInterfaceName("NAME UNREGISTERED");

} // namespace GameComponentFactory_Impl




namespace Component
{

const char* kMiscComponentMemoryTrackerName = "Misc";

/**
@class C_ComponentCreatorFactory
@ingroup actor_group_factory

Clients should call Register() for each component type that requires dynamic 
creation data.
@code
class C_MyComponentCreator : public Component::I_ComponentCreator
{
public:
	static Component::C_ComponentTypeId ClassType() {return E_ComponentType_MyComponent;}
};

void RegisterComponentCreators()
{
	Actor::C_ActorCreationDataFactory::Register<C_MyComponentCreator>();
}
@endcode
*/

/**
@param pCreationData creation data to destroy.
@returns true if the data was destroyed.
*/
bool C_ComponentCreatorFactory::DefaultDestroyFn(const I_ComponentCreator* pCreationData)
{
	delete pCreationData;
	return true;
}

/**
For DataRef::FactoryXXX functions.
@param typeId type of component to make creation data for.
@returns creation data.
*/
I_ComponentCreator* C_ComponentCreatorFactory::Create(const C_ComponentTypeId typeId)
{
	using namespace ComponentCreatorFactory_Impl;
	const C_TypeTable::ConstIterator it = g_TypeTable.Find(typeId);
	return (it == g_TypeTable.End()) ? NULL : it.GetData().m_pCreate();
}

/**
For DataRef::FactoryXXX functions.
@param pData creation data to destroy.
*/
void C_ComponentCreatorFactory::Destroy(const I_ComponentCreator* pData)
{
	if (pData != NULL)
	{
		using namespace ComponentCreatorFactory_Impl;
		const C_TypeTable::ConstIterator it = g_TypeTable.Find(pData->GetComponentTypeID());

		if (it != g_TypeTable.End())
		{
			it.GetData().m_pDestroy(pData);
		}
	}
}

/**
For DataRef::FactoryXXX functions.
@param data creation data to get component type from.
@returns actor type created by this data.
*/
C_ComponentTypeId C_ComponentCreatorFactory::GetTypeOf(const I_ComponentCreator& data)
{
	return data.GetComponentTypeID();
}

#ifndef CFG_SPEED

/**
@param typeId concrete component type.
@returns type checker for this concrete type if known, NULL otherwise.
*/
const C_TypeChecker* C_ComponentCreatorFactory::GetTypeChecker(const C_ComponentTypeId typeId)
{
	using namespace ComponentCreatorFactory_Impl;
	const C_TypeTable::ConstIterator it = g_TypeTable.Find(typeId);
	return (it == g_TypeTable.End()) ? NULL : it.GetData().m_pTypeChecker;
}

#endif // CFG_SPEED

/**
@brief Adds the details for this component type.
@param typeId component type to register.
@param pCreate creation function.
@param pDestroy destruction function.
@param pTypeChecker final type of object to create.
*/
void C_ComponentCreatorFactory::InsertItem(const C_ComponentTypeId typeId, const F_Create pCreate, const F_Destroy pDestroy, const C_TypeChecker* pTypeChecker)
{
	using namespace ComponentCreatorFactory_Impl;

	C_TypeTable::InsertionResult result = g_TypeTable.Insert(typeId);

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
@class C_GameComponentFactory
@ingroup actor_group_factory

@code
class I_MyInterface : public Component::I_GameComponent
{
	static Component::C_ComponentInterfaceId InterfaceType() {return E_InterfaceType_MyInterface;}
};

class C_MyComponent : public I_MyInterface
{
public:
	static Component::C_ComponentTypeId ClassType() {return E_ComponentType_MyComponent;}
};

void RegisterComponents()
{
	Component::C_GameComponentFactory::Register<C_MyComponent>();
}
@endcode
*/

class C_GameComponentFactory::C_Impl
{
public:
	struct C_ConcreteData
	{
		C_ComponentInterfaceId	m_InterfaceType;
		F_Create				m_pCreate;
		F_Destroy				m_pDestroy;
	};

	typedef UID::C_TypeTableHashPolicyHi20<C_ComponentTypeId, 128> C_ConcreteHashPolicy;
	typedef UID::C_TypeTable<C_ComponentTypeId, C_ConcreteData, C_ConcreteHashPolicy> C_ConcreteTypeTable;

	struct C_InterfaceData
	{
		Btl::String				m_InterfaceName;
	};

	typedef UID::C_TypeTableHashPolicyHi20<C_ComponentInterfaceId, 128> C_InterfaceHashPolicy;
	typedef UID::C_TypeTable<C_ComponentInterfaceId, C_InterfaceData, C_InterfaceHashPolicy> C_InterfaceTypeTable;

	C_ConcreteTypeTable		m_ConcreteTypeTable;
	C_InterfaceTypeTable	m_InterfaceTypeTable;

};

C_GameComponentFactory::C_GameComponentFactory()
	: m_pImpl(NULL)
{
	m_pImpl = new C_Impl;
}

C_GameComponentFactory::~C_GameComponentFactory()
{
	delete m_pImpl;
}

bool C_GameComponentFactory::DefaultDestroyFn(I_GameComponent* pComponent)
{
	delete pComponent;
	return true;
}

/**
@brief Create a component.
@param typeId type of component to create.
@returns NULL if the component could not be created.
*/
I_GameComponent* C_GameComponentFactory::CreateComponent(const C_ComponentTypeId typeId, const char* memoryTrackerParentName)
{
	const C_Impl::C_ConcreteTypeTable::ConstIterator it = m_pImpl->m_ConcreteTypeTable.Find(typeId);

	if (it != m_pImpl->m_ConcreteTypeTable.End())
	{
		return it.GetData().m_pCreate();
	}

	LOGERROR(Actor::GetDebugLogGeneral(), "CreateComponent Failed, Component type %d NOT registered!", typeId);
	return NULL;
}

/**
@brief Destroy a component.
@param pComponent component to destroy.
@returns true if the component was destroyed.
*/
bool C_GameComponentFactory::DestroyComponent(I_GameComponent* pComponent)
{
	BIZ_ASSERT(pComponent);

	const C_ComponentTypeId componentType = pComponent->GetComponentTypeID();
	const C_Impl::C_ConcreteTypeTable::ConstIterator it = m_pImpl->m_ConcreteTypeTable.Find(componentType);

	if (it != m_pImpl->m_ConcreteTypeTable.End())
	{
		it.GetData().m_pDestroy(pComponent);
		return true;
	}

	LOGERROR(Actor::GetDebugLogGeneral(), "DestroyComponent Failed, Component type %d NOT registered!", componentType);
	return false;
}

/**
@param componentType unique type identifier for component class.
@returns interface implemented by this component, or -1 if the interface isn't registered.
*/
C_ComponentInterfaceId C_GameComponentFactory::GetComponentInterface(C_ComponentTypeId componentType) const
{
	const C_Impl::C_ConcreteTypeTable::ConstIterator it = m_pImpl->m_ConcreteTypeTable.Find(componentType);
	return (it != m_pImpl->m_ConcreteTypeTable.End()) ? it.GetData().m_InterfaceType : -1;
}

/**
@brief Register a component type to the factory.
@param componentType unique type identifier for component class.
@param interfaceType unique type identifier for component interface.
@param pCreate factory function to create an instance of this component type.
@param pDestroy factory function to destroy an instance of this component type.
@returns true if the component type was registered successfully.
*/
bool C_GameComponentFactory::RegisterImpl(const C_ComponentTypeId componentType, const C_ComponentInterfaceId interfaceType, F_Create pCreate, F_Destroy pDestroy, const char* memoryTrackerName)
{
	C_Impl::C_ConcreteTypeTable::InsertionResult result = m_pImpl->m_ConcreteTypeTable.Insert(componentType);

	if (result.first)
	{
		result.second.GetData().m_InterfaceType = interfaceType;
		result.second.GetData().m_pCreate = pCreate;
		result.second.GetData().m_pDestroy = pDestroy;
		return true;
	}
	else
	{
		return false;
	}
}

void C_GameComponentFactory::RegisterComponentIFName(const C_ComponentInterfaceId interfaceID, const Btl::String& name)
{
	C_Impl::C_InterfaceTypeTable::InsertionResult result = m_pImpl->m_InterfaceTypeTable.Insert(interfaceID);
	result.second.GetData().m_InterfaceName = name;
}

const Btl::String& C_GameComponentFactory::GetComponentIFName(const C_ComponentInterfaceId interfaceID) const
{
	const C_Impl::C_InterfaceTypeTable::ConstIterator it = m_pImpl->m_InterfaceTypeTable.Find(interfaceID);
	return (it != m_pImpl->m_InterfaceTypeTable.End()) ? it.GetData().m_InterfaceName : GameComponentFactory_Impl::g_UnregisteredInterfaceName;
}

} // namespace Component





#if defined(ACTOR_SYSTEM_OPTIMIZE_OFF)
#pragma optimize( "", on ) 
#endif
