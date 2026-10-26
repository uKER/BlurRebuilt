
#include "stdafx.h"
#include <misc_shared/Context/ContextualStream.h>
#include <shared_actors/Utility/XtHandleSerialisation.h>
#include "Game/ActorComponents/Creation/PhysicsBodyComponentCreate.h"


namespace Components
{

/*
void C_PlatformPhysicsBodyCreateInfo::Serialise(Sys::C_ContextualStream& stream) const
{
	C_PhysicsBodyCreateInfo::Serialise(stream);

	stream << m_PlatformId;
}


void C_PlatformPhysicsBodyCreateInfo::Deserialise(Sys::C_ContextualStream& stream)
{
	C_PhysicsBodyCreateInfo::Deserialise(stream);

	stream >> m_PlatformId;
}

*/

void C_ModelPhysicsBodyCreateInfo::Serialise(Sys::C_ContextualStream& stream) const
{
	C_PhysicsBodyCreateInfo::Serialise(stream);

	stream << m_ModelIndex;
	stream << m_ModelElementIndex;
	stream << m_Mass;
}


void C_ModelPhysicsBodyCreateInfo::Deserialise(Sys::C_ContextualStream& stream)
{
	C_PhysicsBodyCreateInfo::Deserialise(stream);

	stream >> m_ModelIndex;
	stream >> m_ModelElementIndex;
	stream >> m_Mass;
}


#if 0
const XtArray<C_DynObjectElementDef>& C_DynamicObjectPhysicsBodyCreateInfo::ElementDefs(void) const
{
	if (m_DynObjDef)
		return m_DynObjDef->m_Elements;
	else if (m_VehicleActorDef)
		return m_VehicleActorDef->m_Elements;

	static XtArray<C_DynObjectElementDef> emptyArrayInstance;
	return emptyArrayInstance;
}


const XtHandle<ResourceUtils::C_GameModel>& C_DynamicObjectPhysicsBodyCreateInfo::Model(void) const
{
	if (m_DynObjDef)
		return m_DynObjDef->m_Model;
	else if (m_VehicleActorDef)
		return m_VehicleActorDef->m_Model;

	static XtHandle<ResourceUtils::C_GameModel> empty;
	return empty;
}


void C_DynamicObjectPhysicsBodyCreateInfo::Serialise(Sys::C_ContextualStream& stream) const
{
	C_PhysicsBodyCreateInfo::Serialise(stream);

	stream << m_DynObjDef;
	stream << m_VehicleActorDef;

	const S32 count = m_ElementDescs.Size();
	stream << count;
	for (S32 e=0 ; e<count ; e++)
		stream << m_ElementDescs[e];
}


void C_DynamicObjectPhysicsBodyCreateInfo::Deserialise(Sys::C_ContextualStream& stream)
{
	C_PhysicsBodyCreateInfo::Deserialise(stream);

	stream >> m_DynObjDef;
	stream >> m_VehicleActorDef;

	assert(m_ElementDescs.Empty());
	S32 count;
	stream >> count;
	for (S32 e=0 ; e<count ; e++)
	{
		S32 val;
		stream >> val;
		m_ElementDescs.PushBack(val);
	}
}
#endif

} //namespace Components
