#include "ActorSystemPch.h"

#include <actor_system/ActorCreation.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/ActorFactory.h>
#include <actor_system/GameComponent.h>
#include <misc_shared/Context/ContextualStream.h>
#include <misc_shared/DataRef/Factory.h>


XT_ABSTRACT_TYPE(Actor::C_ActorCreationData, "ActorCreationData")
	XT_CALLBACK(Actor::C_ActorCreationData::XtCallback)
	XT_FIELD(m_SorceryUid, "SerialisableUid")
XT_END_TYPE


namespace Actor
{

/**
@class I_ActorCreationData
@ingroup actor_group_main
*/

/**
@class C_ActorCreationData
@ingroup actor_group_main

Creation data for actors should derive from this class and implement 
I_ActorCreationData::GetType() to specify the type of actor that should be
created. The GetName() and GetCreationMatrix() methods should be overridden
where appropriate.
*/

C_ActorCreationData::C_ActorCreationData() :
	I_ActorCreationData(), 
	m_SorceryUid(0), 
	m_ActorHandle()
{
	// intentionally blank
}

C_ActorCreationData::~C_ActorCreationData()
{
	// check if creation data is getting destroyed before it's actor is - that would be bad!
	// Note this can happen when updating level creation data in sorcery & potentially the gameplay editor
	// TODO: we need to be able to get to the actor without going through the gameplay
#if 0
	C_GameWorld* const pGameWorld = C_GameWorld::GetInstance();
	if (pGameWorld != NULL)
	{
		C_ActorList* const pActorList = pGameWorld->GetActorList();
		if (pActorList != NULL)
		{
			BIZ_ASSERT(!pActorList->ActorExists(m_ActorHandle));
		}
	}
#endif
}

const char* C_ActorCreationData::GetName() const
{
	return "NoName";
}

const Maths::HMat43 C_ActorCreationData::GetCreationMatrix() const 
{
	return Maths::HMat43();
}

/**
@brief Serialises data for dynamically allocated data.
@param stream receives data.
*/
void C_ActorCreationData::Serialise(Sys::C_ContextualStream& stream) const
{
	// intentionally blank
}

/**
@brief Serialises data for dynamically allocated data.
@param stream provides data.
*/
void C_ActorCreationData::Deserialise(Sys::C_ContextualStream& stream)
{
	// intentionally blank
}

void C_ActorCreationData::XtCallback(XtEventType eventType, XtRecord* pRecord, C_ActorCreationData* pData, XtCallbackContext* pContext)
{
	// this handles the patching-up and serialisation of DataRef::C_Ptr<C_ActorCreationData>
	DataRef::SorceryCallback<Actor::C_ActorCreationData, Actor::C_ActorCreationData>(eventType, pRecord, pData, pContext);
	DataRef::SorceryCallback<Component::I_ComponentCreator, Actor::C_ActorCreationData>(eventType, pRecord, pData, pContext);
}

/**
Derived classes can override this method if they embed component creation data
inside the actor creation data.
@code
void MyActorCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	MyParentClass::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_RenderComponentCreator, &m_pRenderCreator);
}
@endcode
@param pCallbackFn function to call for each component creation object.
*/
void C_ActorCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	// intentionally blank
}

/**
Creates an object of the specified type.
@param actorType type of creation data to create.
@param ppData if non-NULL, receives a writable pointer.
@param pTypeChecker final type of object to create.
@returns reference to created data.
*/
C_ActorCreationData::C_DataRef C_ActorCreationData::FactoryCreate(const C_ActorTypeId actorType, C_ActorCreationData** ppData, const C_TypeChecker* pTypeChecker)
{
#ifndef CFG_SPEED
	{
		const C_TypeChecker* pExpectedTypeChecker = C_ActorCreationDataFactory::GetTypeChecker(actorType);

		// If this assert fires then the type of the object you're trying to create 
		// is different from the type of the object registered to this type id...
		// Likely explanations:
		//	- you've got some duplicate type ids.
		//	- you're creation data type is returning the wrong value from TypeId().
		//	- you've registered a base class and tried to create a dervied class or vice versa.
		BIZ_ASSERT((pExpectedTypeChecker == NULL) || (pTypeChecker == NULL) || (pExpectedTypeChecker == pTypeChecker));
	}
#endif
	return DataRef::FactoryCreate<C_ActorCreationDataFactory>(actorType, ppData);
}

/**
@brief Helper function for DoSorceryCallback.
@param pCallbackFn function to call.
@param data component creation data.
@param ppData if non-NULL receives sorcery-powered data reference.
*/
void C_ActorCreationData::DoComponentSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn, Component::I_ComponentCreator& data, DataRef::C_Ptr<Component::I_ComponentCreator>* ppData)
{
	if (data.GetSorceryUid() == 0)
		LOGERROR(GetDebugLogLevelDesign(), "An actor component has no database ID, the Sorcery database and the ActorStudio level should both be updated to latest and exported");

	pCallbackFn(&data);

	if (ppData != NULL)
		*ppData = DataRef::SorceryLookup(&data);
}

/**
@brief Links the creation data to the actor instance.
This is usually only called by the actor list that creates the actor and
should only be overridden and/or called directly if you're absolutely sure
that you know what you're doing!
@param handle handle to actor created from this data.
*/
void C_ActorCreationData::LinkActorHandle(const C_ActorHandle& handle) const
{
	m_ActorHandle = handle;
}





/**
@class C_ActorCreationDataFactoryHelper.
@ingroup actor_group_factory

Automates some of the work of dynamically allocating and filling out actor
creation data using the factory:
@code
class C_MyActorCreationData : public C_ActorCreationData
{
public:
	// this is needed for the factory creation method
	static C_ActorTypeId ClassType() {return E_ActorType_MyActor;}
};

Actor::C_ActorHandle MakeMyActor(Actor::C_ActorList& actorList)
{
	Actor::C_ActorCreationDataFactoryHelper<C_MyActorCreationData> creationData;

	creationData->m_WorldMatrix.SetIdentity();
	creationData->m_NumThings = 17;
	creationData->m_Radius = 5.0f;

	return actorList.Spawn(creationData.Get());
}
@endcode
*/

} // namespace Actor


void SerialiseStorage(const DataRef::I_Storage<Actor::C_ActorCreationData>& storage, Sys::C_ContextualStream& stream)
{
	switch (storage.GetStorageType())
	{
	case DataRef::kFactoryStorageType:	DataRef::FactorySerialise<Actor::C_ActorCreationDataFactory>(storage, stream);	break;
	case DataRef::kSorceryStorageType:	DataRef::SorcerySerialise(storage, stream);										break;

	default:
		BIZ_ASSERT_FAIL("don't know how to serialise this type");
		LOGERROR(Actor::GetDebugLogGeneral(), "Failed to serialise DataRef for C_ActorCreationData... ask a programmer to investigate");
		break;
	}
}

void DeserialisePtr(const DataRef::C_StorageTypeId storageType, DataRef::C_Ptr<Actor::C_ActorCreationData>& ptr, Sys::C_ContextualStream& stream)
{
	switch (storageType)
	{
	case DataRef::kFactoryStorageType:	DataRef::FactoryDeserialise<Actor::C_ActorCreationDataFactory>(ptr, stream);	break;
	case DataRef::kSorceryStorageType:	DataRef::SorceryDeserialise(ptr, stream);										break;

	default:
		BIZ_ASSERT_FAIL("don't know how to deserialise this type");
		ptr.SetNull();
		LOGERROR(Actor::GetDebugLogGeneral(), "Failed to deserialise DataRef for C_ActorCreationData... ask a programmer to investigate");
		break;
	}
}
