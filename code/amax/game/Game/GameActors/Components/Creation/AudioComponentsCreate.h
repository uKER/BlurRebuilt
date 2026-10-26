#ifndef AUDIOCOMPONENTS_CREATE_H
#define AUDIOCOMPONENTS_CREATE_H

#ifndef NOT_USING_AUDIO_COMPONENETS

#ifdef _MSC_VER
#pragma once
#endif

//#include "GameDef/AudioObjectsDef.h"
//#include "XtAudio.h"
#include <../../mercury/library/audio/include/XtAudio.h>
#include "Game/GameActors/Components/GameComponents.h"
#include <shared_actors/XtActorCreation.h>

namespace Components
{

// notes: dont do casting to get derived type.

// audio entity component create base
class I_AudioEntity_CreateInfo : public I_GameComponentCreator
{
public:
	I_AudioEntity_CreateInfo() {}
	XtHandle<Audio::XtEntityProxy> GetXtEntityDef() const { return m_AudioEntityDef; }
	void						 SetXtEntityDef(XtHandle<Audio::XtEntityProxy> hDef)  { m_AudioEntityDef = hDef; }

//private:
	XtHandle<Audio::XtEntityProxy>			m_AudioEntityDef;
};

// audio entity component create info for component created dynamically by an actor
class C_AudioEntity_SpawnedCreateInfo : public I_AudioEntity_CreateInfo
{
public:
	C_AudioEntity_SpawnedCreateInfo(){}
	static Component::C_ComponentTypeId		ClassType() {return Components::E_AudioEntity_Spawned;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_AudioEntity_Spawned;}
	
	//virtual void							DeclareResources(C_GameResourceList &resourceList);

	virtual void							Serialise(Sys::C_ContextualStream& stream) const;
	virtual void							Deserialise(Sys::C_ContextualStream& stream);
};

// audio entity component create info for component created dynamically by an actor
class C_AudioEntity_AdditionalSpawnedCreateInfo : public I_AudioEntity_CreateInfo
{
public:
	C_AudioEntity_AdditionalSpawnedCreateInfo(){}
	static Component::C_ComponentTypeId		ClassType() {return Components::E_AudioEntity_AdditionalSpawned;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_AudioEntity_AdditionalSpawned;}
	
	//virtual void							DeclareResources(C_GameResourceList &resourceList);

	virtual void							Serialise(Sys::C_ContextualStream& stream) const{}
	virtual void							Deserialise(Sys::C_ContextualStream& stream){}

	XtHandle<Audio::XtEntityProxy>			GetXtSharedEntityDef() const { return m_AudioSharedEntityDef; }
	void									 SetXtSharedEntityDef(XtHandle<Audio::XtEntityProxy> hDef);

	protected:
	XtHandle<Audio::XtEntityProxy>			m_AudioSharedEntityDef;
};

// audio entity component create info for component setup via actor studio
class C_AudioEntity_ActorStudioCreateInfo : public I_AudioEntity_CreateInfo
{
public:
	C_AudioEntity_ActorStudioCreateInfo(){}
	static Component::C_ComponentTypeId		ClassType() {return Components::E_AudioEntity_ActorStudio;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_AudioEntity_ActorStudio;}
	//virtual void							DeclareResources(C_GameResourceList &resourceList);
};

}	// namespace Components


#endif //#ifndef NOT_USING_AUDIO_COMPONENETS
#endif
