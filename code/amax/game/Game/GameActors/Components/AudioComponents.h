#ifndef AUDIOCOMPONENTS_H
#define AUDIOCOMPONENTS_H

#ifndef NOT_USING_AUDIO_COMPONENETS

#ifdef _MSC_VER
#pragma once
#endif

#ifdef USER_SHALL
#pragma message("TODO: split up AppInterfaceLayer.h")
#endif
#include "AppInterfaceLayer.h"

#include "Game/GameActors/Components/Creation/AudioComponentsCreate.h"

struct ActiveDynamicEntity
{
	ActiveDynamicEntity() : fTimeCreated(FLT_MAX), bToDestroy(false), fDistance(0.f) { }
	void Clear();

	Actor::C_ActorHandle hActor;
	AudioEntityHandle hEntity;
	F32 fTimeCreated;
	bool bToDestroy;
	int callbackState;		// debug. 0 = never played, 1 =  playing, 2 = was playing but now stopped
	F32 fDistance;			// debug
};

class C_DynamicEntityList
{
public:
	static const int MAX_NUM_DYNOBJ_INSTANCES = 8;

	static C_DynamicEntityList&	Get() { return m_sInstance; }

	C_DynamicEntityList(): m_DynamicObjectCount(0) {}

	ActiveDynamicEntity* Lookup(AudioEntityHandle& hEntity);
	
	ActiveDynamicEntity* GetFreeEntitySlot();
	
	void RemoveFromList(AudioEntityHandle& hEntity);
	
	void SetCallbackState(AudioEntityHandle& hEntity, int state);
	void SetDistance(AudioEntityHandle& hEntity, F32 fDistance);

public:
	S32 m_DynamicObjectCount;
	Btl::Vector<ActiveDynamicEntity, Btl::VectorParameters_Array<MAX_NUM_DYNOBJ_INSTANCES> > mEntities;

private:
	static C_DynamicEntityList m_sInstance;
};

namespace Components
{

class BaseMessageHandler;

// Sam: Important note from MarkC: nothing must change the state of components directly, but must do so via an actor message.
// All public functions must be const.

class I_AudioEntity : public I_GameActorComponent
{
public:


	~I_AudioEntity();

	static Component::C_ComponentInterfaceId		InterfaceType() {return Components::E_IF_AudioEntity;}
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_IF_AudioEntity;} // from Component::I_GameComponent
	virtual bool									OnMessage(const Messaging::C_MessageBase *pMessage);

	virtual const AudioEntityHandle&				GetEntityHnd() const = 0;
	virtual const Audio::EntityDefinitionHandle&	GetEntityDefHnd() const = 0;

	const I_AudioEntity_CreateInfo*					GetCreationData(void) const {return static_cast<const I_AudioEntity_CreateInfo *>(GetCreator().GetData());}
	AudioEntityHandle								CreateEntityInstance();
	bool											ReleaseEntityInstance(bool bReleaseForever = false);

	void											KillImpl();

	BaseMessageHandler*								GetEntityHandler() const { return m_pHandler; }

protected:
	virtual bool									CreateImpl();


	void											SetHandler(BaseMessageHandler* pHandler) { m_pHandler = pHandler; }

	virtual bool									PrimeImpl();

	Audio::EntityDefinitionHandle					m_hEntityDefinition;
	AudioEntityHandle								m_hEntityInst;
	BaseMessageHandler*								m_pHandler;
	XtHandle<Audio::XtEntityProxy>					m_hXtEntityDef;
	
};

// an audio entity component created dynamically by an actor
class C_AudioEntity_Spawned : public I_AudioEntity
{
public:
	C_AudioEntity_Spawned(){ m_pHandler = 0; 	/*totalEntityNum++;*/}
	~C_AudioEntity_Spawned();

	static Component::C_ComponentTypeId				ClassType() {return Components::E_AudioEntity_Spawned;}
	virtual Component::C_ComponentTypeId			GetComponentTypeID() const {return Components::E_AudioEntity_Spawned;} // from Component::I_GameComponent
	virtual Component::C_ComponentInterfaceId		GetInterfaceID() const {return Components::E_AudioEntity_Spawned;} // from Component::I_GameComponent
	const C_AudioEntity_SpawnedCreateInfo			*GetCreationData(void) const {return static_cast<const C_AudioEntity_SpawnedCreateInfo *>(GetCreator().GetData());}

	virtual const AudioEntityHandle&				GetEntityHnd() const { return m_hEntityInst; }
	virtual const Audio::EntityDefinitionHandle&	GetEntityDefHnd() const { return m_hEntityDefinition; }

protected:
//	virtual BaseMessageHandler*						GetEntityHandler() { return m_pHandler; }
};

// an audio entity component created dynamically by an actor
class C_AudioEntity_AdditionalSpawned : public I_AudioEntity
{
public:
	C_AudioEntity_AdditionalSpawned(){ m_pHandler = 0; 	/*totalEntityNum++;*/}
	~C_AudioEntity_AdditionalSpawned();

	static Component::C_ComponentTypeId				ClassType() {return Components::E_AudioEntity_AdditionalSpawned;}
	virtual Component::C_ComponentTypeId			GetComponentTypeID() const {return Components::E_AudioEntity_AdditionalSpawned;} // from Component::I_GameComponent
virtual Component::C_ComponentInterfaceId			GetInterfaceID() const {return Components::E_AudioEntity_AdditionalSpawned;} // from Component::I_GameComponent
	const C_AudioEntity_AdditionalSpawnedCreateInfo			*GetCreationData(void) const {return static_cast<const C_AudioEntity_AdditionalSpawnedCreateInfo *>(GetCreator().GetData());}

	virtual const AudioEntityHandle&				GetEntityHnd() const { return m_hEntityInst; }
	virtual const Audio::EntityDefinitionHandle&	GetEntityDefHnd() const { return m_hEntityDefinition; }

	const AudioEntityHandle&						GetSharedEntityHnd() const { return m_hSharedEntityInst; }
	const Audio::EntityDefinitionHandle&			GetSharedEntityDefHnd() const { return m_hSharedEntityDefinition; }

	AudioEntityHandle								CreateEntityInstance();

	bool											CreateImpl();
	bool											PrimeImpl();
	bool											ReleaseEntityInstance(bool bReleaseForever);

	void											SetHandler(BaseMessageHandler* pHandler);

protected:
	//virtual BaseMessageHandler*						GetEntityHandler() { return m_pHandler; }
	Audio::EntityDefinitionHandle					m_hSharedEntityDefinition;
	AudioEntityHandle								m_hSharedEntityInst;
//	BaseMessageHandler*								m_pSharedHandler;
	XtHandle<Audio::XtEntityProxy>					m_hXtSharedEntityDef;

};

// an audio entity component placed via actor studio 
class C_AudioEntity_ActorStudio : public I_AudioEntity
{
public:
	C_AudioEntity_ActorStudio() { m_pHandler = 0; }
	~C_AudioEntity_ActorStudio();

	static Component::C_ComponentTypeId				ClassType() {return Components::E_AudioEntity_ActorStudio;}
	virtual Component::C_ComponentTypeId			GetComponentTypeID() const {return Components::E_AudioEntity_ActorStudio;} // from Component::I_GameComponent
	const C_AudioEntity_ActorStudioCreateInfo		*GetCreationData(void) const {return static_cast<const C_AudioEntity_ActorStudioCreateInfo *>(GetCreator().GetData());}
	
protected:
	virtual const AudioEntityHandle&				GetEntityHnd() const { return m_hEntityInst; }
	virtual const Audio::EntityDefinitionHandle&	GetEntityDefHnd() const { return m_hEntityDefinition; }
	//virtual BaseMessageHandler*						GetEntityHandler() { return m_pHandler; }
};

} // namespace Components


#endif //#ifndef NOT_USING_AUDIO_COMPONENETS
#endif