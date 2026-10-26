#include "stdafx.h"

#include <misc_shared/Context/ContextualStream.h>
#include <shared_actors/Utility/XtHandleSerialisation.h>

#include "RenderComponentsCreate.h"

namespace Components
{

#if 0
void C_RenderComponentCreateInfo::Serialise(Sys::C_ContextualStream& stream) const
{
	I_GameComponentCreator::Serialise(stream);
	//stream << m_WorldMatrix;
	stream << m_BoundingSphere;
	stream << m_bStartHidden;
}

void C_RenderComponentCreateInfo::Deserialise(Sys::C_ContextualStream& stream)
{
	I_GameComponentCreator::Deserialise(stream);
	//stream >> m_WorldMatrix;
	stream >> m_BoundingSphere;
	stream >> m_bStartHidden;

}

void C_RenderModelBaseCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	C_RenderComponentCreateInfo::Serialise(stream);
	//stream << m_ModelHandle;			
	stream << m_HasShadow;
}

void C_RenderModelBaseCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	C_RenderComponentCreateInfo::Deserialise(stream);
	//stream >> m_ModelHandle;			
	stream >> m_HasShadow;
}

void C_ModelSingleElementRenderCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	C_RenderModelBaseCreate::Serialise(stream);
	//stream << m_ModelElement;
	stream << m_RenderOffset;
}

void C_ModelSingleElementRenderCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	C_RenderModelBaseCreate::Deserialise(stream);
	//stream >> m_ModelElement;
	stream >> m_RenderOffset;
}
#endif

void C_ModelMultiElementRenderCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	C_RenderModelBaseCreate::Serialise(stream);
	stream << m_Pivoted;
}

void C_ModelMultiElementRenderCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	C_RenderModelBaseCreate::Deserialise(stream);
	stream >> m_Pivoted;
}


} // namespace Components

// Declare Xt Info
/*XT_TYPE(Components::C_PortalBlockerCreate, "PortalBlockerComp")
XT_BASE(Components::I_GameComponentCreator)
XT_FIELD(m_BoundingBox, "BoundingBox")
XT_FIELD(m_bStartClosed, "StartClosed")
XT_END_TYPE

XT_LIBRARY(RenderComponentCreation)
XT_REGISTER_TYPE(Components::C_PortalBlockerCreate)
XT_END_LIBRARY
*/