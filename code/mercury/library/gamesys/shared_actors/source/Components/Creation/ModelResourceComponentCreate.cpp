#include "SharedActorsPch.h"

#include <shared_actors/Components/Creation/ModelResourceComponentCreate.h>
#include <misc_shared/Context/ContextualStream.h>
#include <shared_actors/Utility/XtHandleSerialisation.h>
#include <resource_utils/GameModel.h>

namespace Components
{

// File Model Resource
void C_FileModelResourceCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	I_GameComponentCreator::Serialise(stream);
	stream << m_ModelHandle;
}

void C_FileModelResourceCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	I_GameComponentCreator::Deserialise(stream);
	stream >> m_ModelHandle;
}

Util::C_String C_FileModelResourceCreate::GetFileName(void) const 
{
	return m_ModelHandle->GetFileName();
}

bool C_FileModelResourceCreate::IsValid(void) const
{
	return m_ModelHandle != NULL;
}


void C_FileNameModelResourceCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	I_GameComponentCreator::Serialise(stream);
	stream << m_Filename;
}

void C_FileNameModelResourceCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	I_GameComponentCreator::Deserialise(stream);
	stream >> m_Filename;
}

// Level Model Resource
void C_LevelModelResourceCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	I_GameComponentCreator::Serialise(stream);
	//stream << m_LevelModelUid;
}

void C_LevelModelResourceCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	I_GameComponentCreator::Deserialise(stream);
	//stream >> m_LevelModelUid;
}


}//namespace Components
