#include "SharedActorsPch.h"

#include <shared_actors/Components/creation/RenderComponentsCreate.h>
#include <misc_shared/Context/ContextualStream.h>
#include <shared_actors/Utility/XtHandleSerialisation.h>


namespace Components
{

C_RenderComponentCreateInfo::C_RenderComponentCreateInfo()
	: I_GameComponentCreator()
	, m_WorldMatrix(Maths::Constants<Maths::HMat43>::Identity())
	, m_BoundingSphere(Maths::Constants<Maths::HVec3>::Zero(), Maths::Constants<Maths::HF32>::Zero())
	, m_bStartHidden(false)
{
}

C_RenderComponentCreateInfo::~C_RenderComponentCreateInfo()
{
}

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

C_RenderModelBaseCreate::C_RenderModelBaseCreate()
	: C_RenderComponentCreateInfo()
	, m_HasShadow(false)
	, m_IgnorePortals(false)
	, m_DrawDistance(100.0f)
{
}

C_RenderModelBaseCreate::~C_RenderModelBaseCreate()
{
}

void C_RenderModelBaseCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	C_RenderComponentCreateInfo::Serialise(stream);
	//stream << m_ModelHandle;			
	stream << m_HasShadow;
	stream << m_IgnorePortals;
	stream << m_DrawDistance;
}

void C_RenderModelBaseCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	C_RenderComponentCreateInfo::Deserialise(stream);
	//stream >> m_ModelHandle;			
	stream >> m_HasShadow;
	stream >> m_IgnorePortals;
	stream >> m_DrawDistance;
}

C_ModelSingleElementRenderCreate::C_ModelSingleElementRenderCreate()
	: C_RenderModelBaseCreate()
	, m_ModelIndex(0)
	, m_ModelElementIndex(0)
	, m_RenderOffset(0.0f, 0.0f, 0.0f)
{
}

C_ModelSingleElementRenderCreate::~C_ModelSingleElementRenderCreate()
{
}

void C_ModelSingleElementRenderCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	C_RenderModelBaseCreate::Serialise(stream);
	stream << m_ModelIndex;
	stream << m_ModelElementIndex;
	stream << m_RenderOffset;
}

void C_ModelSingleElementRenderCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	C_RenderModelBaseCreate::Deserialise(stream);
	stream >> m_ModelIndex;
	stream >> m_ModelElementIndex;
	stream >> m_RenderOffset;
}

C_ModelSimpleMultiElementRenderCreate::C_ModelSimpleMultiElementRenderCreate()
	: C_RenderModelBaseCreate()
	, m_ModelIndex(0)
{
}

C_ModelSimpleMultiElementRenderCreate::~C_ModelSimpleMultiElementRenderCreate()
{
}

void C_ModelSimpleMultiElementRenderCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	C_RenderModelBaseCreate::Serialise(stream);
	stream << m_ModelIndex;
}

void C_ModelSimpleMultiElementRenderCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	C_RenderModelBaseCreate::Deserialise(stream);
	stream >> m_ModelIndex;
}


///////////////////////////////////////////////////////////////////////////
//
C_ElementManagerObjectRenderCreate::C_ElementManagerObjectRenderCreate()
	: C_RenderModelBaseCreate()
	, m_ModelIndex(0)
	, m_Pivoted(false)
{
}

C_ElementManagerObjectRenderCreate::~C_ElementManagerObjectRenderCreate()
{
}

void C_ElementManagerObjectRenderCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	C_RenderModelBaseCreate::Serialise(stream);
	stream << m_ModelIndex;
	stream << m_Pivoted;
}

void C_ElementManagerObjectRenderCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	C_RenderModelBaseCreate::Deserialise(stream);
	stream >> m_ModelIndex;
	stream >> m_Pivoted;
}


} // namespace Components
