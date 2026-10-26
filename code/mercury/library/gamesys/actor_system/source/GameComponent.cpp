#include "ActorSystemPch.h"

#include <actor_system/Actor.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/GameComponent.h>
#include <actor_system/GameComponentFactory.h>
#include <misc_shared/Context/ContextualStream.h>
#include <misc_shared/DataRef/Factory.h>
#include <misc_shared/Messaging/GameMessaging.h>

XT_ABSTRACT_TYPE(Component::I_ComponentCreator, "ComponentCreator")
	XT_CALLBACK(Component::I_ComponentCreator::XtCallback)
	XT_FIELD(m_SorceryUid, "SerialisableUid")
XT_END_TYPE


namespace Component
{

/**
@class I_ComponentCreator
@ingroup actor_group_component

Implementations should override GetComponentTypeID() to return the user-
defined component type identifier.
*/

I_ComponentCreator::I_ComponentCreator() : m_SorceryUid(0)
{
	// intentionally blank
}

I_ComponentCreator::~I_ComponentCreator()
{
	// intentionally blank
}

/**
@param stream stream to write info.
*/
void I_ComponentCreator::Serialise(Sys::C_ContextualStream& stream) const
{
	// intentionally blank
}

/**
@param stream stream to read from.
*/
void I_ComponentCreator::Deserialise(Sys::C_ContextualStream& stream)
{
	// intentionally blank
}

void I_ComponentCreator::XtCallback(XtEventType eventType, XtRecord* pRecord, I_ComponentCreator* pData, XtCallbackContext* pContext)
{
	if (pData->GetSorceryUid() == 0)
		LOGERROR(Actor::GetDebugLogLevelDesign(), "An actor component has no database ID, the Sorcery database and the ActorStudio level should both be updated to latest and exported");

	DataRef::SorceryCallback<Component::I_ComponentCreator, Component::I_ComponentCreator>(eventType, pRecord, pData, pContext);
}

/**
Creates an object of the specified type.
@param typeId type of creation data to create.
@param ppData if non-NULL, receives a writable pointer.
@param pTypeChecker final type of object to create.
@returns reference to created data.
*/
I_ComponentCreator::C_DataRef I_ComponentCreator::FactoryCreate(const C_ComponentTypeId typeId, I_ComponentCreator** ppData, const C_TypeChecker* pTypeChecker)
{
#ifndef CFG_SPEED
	{
		const C_TypeChecker* pExpectedTypeChecker = C_ComponentCreatorFactory::GetTypeChecker(typeId);

		// If this assert fires then the type of the object you're trying to create 
		// is different from the type of the object registered to this type id...
		// Likely explanations:
		//	- you've got some duplicate type ids.
		//	- you're creation data type is returning the wrong value from ClassType().
		//	- you've registered a base class and tried to create a dervied class or vice versa.
		BIZ_ASSERT((pExpectedTypeChecker == NULL) || (pTypeChecker == NULL) || (pExpectedTypeChecker == pTypeChecker));
	}
#endif
	return DataRef::FactoryCreate<C_ComponentCreatorFactory>(typeId, ppData);
}

#ifndef CFG_GOLD

/**
@brief Renders debug info for the creation data.
@param debugRenderPtr context to render into.
@param actorMatrix world transform matrix of containing actor.
*/
void I_ComponentCreator::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43& actorMatrix) const 
{
	// intentionally blank
}

#endif





/**
@class C_ComponentCreatorFactoryHelper
@ingroup actor_group_factory

Automates some of the work of dynamically allocating and filling out component
creation data using the factory:
@code
class C_MyComponentCreator : public Component::I_ComponentCreator
{
public:
	// this is needed for the factory creation method
	static Component::C_ComponentTypeId ClassType() {return E_ComponentType_MyComponent;}
};

void MyComposite::CreateMyComponent()
{
	Component::C_ComponentCreatorFactoryHelper<C_MyComponentCreator> creationData;

	creationData->m_WorldMatrix.SetIdentity();
	creationData->m_NumThings = 17;
	creationData->m_Radius = 5.0f;

	CreateComponent(creationData.Get());
}
@endcode

*/




/**
@class I_GameComponent
@ingroup actor_group_component

Components are identified by a unique interface identifier for general
purposes, and by a unique concrete type identifier for creation and
destruction. Both of these are user-defined values which should be supplied
by implementing GetInterfaceID() and GetComponentTypeID() respectively.

Component creation is a dual-phase process, with the constructor being called
first and then the Create() method called subsequently.

Most component types will override the MessageHandler() method.
*/

I_GameComponent::I_GameComponent() : 
	Messaging::C_MessageHandler(), 
	m_pCreator(), 
	m_LifetimeState(ActorSys::E_LifetimeState_Constructed)
{
	// intentionally blank
}

I_GameComponent::~I_GameComponent() 
{
	// intentionally blank
}

/**
@brief Called to move a component from the Constructed state.
@param pCreationData data to use for the component.
@returns false if the component could not be initialised.
*/
bool I_GameComponent::Initialise(const I_ComponentCreator::C_DataRef& pCreationData)
{
	if (GetLifetimeState() != ActorSys::E_LifetimeState_Constructed)
		return false;

	if (pCreationData.IsNull())
		return false;

	m_pCreator = pCreationData;

	if (InitialiseImpl())
	{
		SetLifetimeState(ActorSys::E_LifetimeState_Initialised);
		return true;
	}
	else
	{
		SetLifetimeState(ActorSys::E_LifetimeState_Failed);
		return false;
	}
}

/**
@brief Called to move a component from the Constructed state.
@param pCreationData data to use for the component.
@param stream data to be deserialised.
@returns false if the component could not be initialised.
*/
bool I_GameComponent::Initialise(const I_ComponentCreator::C_DataRef& pCreationData, Sys::C_ContextualStream& stream)
{
	if (GetLifetimeState() != ActorSys::E_LifetimeState_Constructed)
		return false;

	if (pCreationData.IsNull())
		return false;

	m_pCreator = pCreationData;

	DeserialiseForSave(stream);
	SetLifetimeState(ActorSys::E_LifetimeState_Initialised);
	return true;
}

/**
@brief Called to move a component from the Initialised state.
@returns false if the component could not be primed.
*/
bool I_GameComponent::Prime()
{
	if (GetLifetimeState() != ActorSys::E_LifetimeState_Initialised)
		return false;

	SetLifetimeState(ActorSys::E_LifetimeState_Priming);

	if (PrimeImpl())
		SetLifetimeState(ActorSys::E_LifetimeState_Primed);

	return true;
}

/**
@brief Called to move a component from the Primed state.
@returns true if the component is created successfully.
*/
bool I_GameComponent::Create()
{
	if (GetLifetimeState() != ActorSys::E_LifetimeState_Primed)
		return false;

	if (CreateImpl())
	{
		SetLifetimeState(ActorSys::E_LifetimeState_Created);
		return true;
	}
	else
	{
		SetLifetimeState(ActorSys::E_LifetimeState_Failed);
		return false;
	}
}

void I_GameComponent::Kill()
{
	KillImpl();
	SetLifetimeState(ActorSys::E_LifetimeState_Killed);
}

/**
If a derived class overrides PrimeImpl then it should call this method once
all requested resources have loaded.
*/
void I_GameComponent::SetPrimed()
{
	BIZ_ASSERT(GetLifetimeState() == ActorSys::E_LifetimeState_Priming);
	SetLifetimeState(ActorSys::E_LifetimeState_Primed);
}

/**
Derived classes can override this method to perform initial setup.
@returns false if component could not be initialised.
*/
bool I_GameComponent::InitialiseImpl()
{
	return true;
}

/**
Derived classes can override this method to request resources. If the override
returns false then the derived class should call SetPrimed at the appropriate
time.
@returns false if waiting for resources to load.
*/
bool I_GameComponent::PrimeImpl()
{
	return true;
}

/**
Derived classes can override this method to perform actions that place the
component in the game world, such as registering with tech systems or 
subscribing to messages.
@returns false if creation failed.
*/
bool I_GameComponent::CreateImpl()
{
	return true;
}

void I_GameComponent::KillImpl()
{
	return;
}

/**
Derived classes can override this method to perform actions that require the
object containing the component, or other components within that object, to
have been created.
*/
void I_GameComponent::InternalDependenciesCreatedImpl()
{
}

/**
Derived classes can override this method to perform actions that require other
objects related to the containing object to have been created.
*/
void I_GameComponent::ExternalDependenciesCreatedImpl()
{
}

/**
@brief Serialises data for saving.
@param stream stream to receive data.
*/
void I_GameComponent::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	// intentionally blank
}

/**
@brief Deserialises data for saving.
@param stream stream to provide data.
*/
void I_GameComponent::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	// intentionally blank
}

/**
@brief Serialises data for synchronising.
@param stream stream to receive data.
*/
void I_GameComponent::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	// intentionally blank
}

/**
@brief Deserialises data for synchronising.
@param stream stream to provide data.
*/
void I_GameComponent::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	// intentionally blank
}

#ifndef CFG_GOLD

/**
@brief Render debug info for the component.
@param debugRenderPtr context to render into.
*/
void I_GameComponent::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr)  const
{
	// intentionally blank
}

#endif

} // namespace Component


void SerialiseStorage(const DataRef::I_Storage<Component::I_ComponentCreator>& storage, Sys::C_ContextualStream& stream)
{
	switch (storage.GetStorageType())
	{
	case DataRef::kFactoryStorageType:	DataRef::FactorySerialise<Component::C_ComponentCreatorFactory>(storage, stream);	break;
	case DataRef::kSorceryStorageType:	DataRef::SorcerySerialise(storage, stream);											break;

	default:
		BIZ_ASSERT_FAIL("don't know how to serialise this type");
		LOGERROR(Actor::GetDebugLogGeneral(), "Failed to serialise DataRef for I_ComponentCreator... ask a programmer to investigate");
		break;
	}
}

void DeserialisePtr(const DataRef::C_StorageTypeId storageType, DataRef::C_Ptr<Component::I_ComponentCreator>& ptr, Sys::C_ContextualStream& stream)
{
	switch (storageType)
	{
	case DataRef::kFactoryStorageType:	DataRef::FactoryDeserialise<Component::C_ComponentCreatorFactory>(ptr, stream);	break;
	case DataRef::kSorceryStorageType:	DataRef::SorceryDeserialise(ptr, stream);										break;

	default:
		BIZ_ASSERT_FAIL("don't know how to deserialise this type");
		ptr.SetNull();
		LOGERROR(Actor::GetDebugLogGeneral(), "Failed to deserialise DataRef for I_ComponentCreator... ask a programmer to investigate");
		break;
	}
}
