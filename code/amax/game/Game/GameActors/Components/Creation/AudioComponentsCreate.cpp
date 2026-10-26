#include "stdafx.h"

#ifndef NOT_USING_AUDIO_COMPONENETS

#include "Game/GameActors/Components/Creation/AudioComponentsCreate.h"
//#include "GameDef/XtHandleSerialisation.h"

// XT annotation
XT_TYPE(Components::C_AudioEntity_ActorStudioCreateInfo, "AudioComponentComp")
XT_BASE(Components::I_GameComponentCreator)
XT_FIELD(m_AudioEntityDef, "AudioEntity")
XT_END_TYPE

XT_LIBRARY(AudioComponentCreation)
XT_REGISTER_TYPE(Components::C_AudioEntity_ActorStudioCreateInfo)
XT_END_LIBRARY

namespace Components
{

void C_AudioEntity_SpawnedCreateInfo::Serialise(Sys::C_ContextualStream& stream) const
{
	I_GameComponentCreator::Serialise(stream);
	//[Cian] needs to go in for Amax
	//stream << m_AudioEntityDef;
}

void C_AudioEntity_SpawnedCreateInfo::Deserialise(Sys::C_ContextualStream& stream)
{
	I_GameComponentCreator::Deserialise(stream);
	//[Cian] needs to go in for Amax
	//stream >> m_AudioEntityDef;
}

void C_AudioEntity_AdditionalSpawnedCreateInfo::SetXtSharedEntityDef(XtHandle<Audio::XtEntityProxy> hDef)
{ 
	m_AudioSharedEntityDef = hDef; 
}

// Resources declaration
// void C_AudioObjectCreateInfo::DeclareResources(C_GameResourceList &resourceList)
// {
// 	if(m_AudioObjectDef)
// 		m_AudioObjectDef->AddToResourceList(resourceList);
// }


} //namespace Components

#endif //#ifndef NOT_USING_AUDIO_COMPONENETS