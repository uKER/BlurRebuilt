#include "SharedActorsPch.h"

#include <shared_actors/components/creation/TriggerCreate.h>

#include <misc_shared/context/ContextualStream.h>


namespace Components
{

I_VolumeTriggerCreate::I_VolumeTriggerCreate(const U32 flags) :
	Component::I_ComponentCreator(), 
	m_Flags(flags),m_TriggerLayerIndex(0),m_TriggerMask(0xffffffff)
{
	// intentionally blank
}

I_VolumeTriggerCreate::~I_VolumeTriggerCreate()
{
	// intentionally blank
}

void I_VolumeTriggerCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	Component::I_ComponentCreator::Serialise(stream);
	stream << m_Flags;
	stream << m_TriggerLayerIndex;
	stream << m_TriggerMask;
}

void I_VolumeTriggerCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	Component::I_ComponentCreator::Deserialise(stream);
	stream >> m_Flags;
	stream >> m_TriggerLayerIndex;
	stream >> m_TriggerMask;
}




C_BoxTriggerCreate::C_BoxTriggerCreate() : 
	I_VolumeTriggerCreate(), 
	m_Size(Maths::Constants<Maths::HVec3>::Zero()), 
	m_Matrix(Maths::Constants<Maths::HMat43>::Identity())
{
	// intentionally blank
}

C_BoxTriggerCreate::~C_BoxTriggerCreate()
{
	// intentionally blank
}

#ifndef CFG_GOLD
void C_BoxTriggerCreate::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43 &actorMatrix)  const
{
	Maths::C_AxisBox box(Maths::Constants<Maths::HVec3>::Zero(), m_Size);
	debugRenderPtr->Draw3dBox(box, m_Matrix * actorMatrix, Maths::RGBA(255,0,0));
}
#endif

void C_BoxTriggerCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	I_VolumeTriggerCreate::Serialise(stream);
	stream << m_Size;
	stream << m_Matrix;
}

void C_BoxTriggerCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	I_VolumeTriggerCreate::Deserialise(stream);
	stream >> m_Size;
	stream >> m_Matrix;
}




C_SphereTriggerCreate::C_SphereTriggerCreate() :
	I_VolumeTriggerCreate(), 
	m_Sphere(Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero())
{
	// intentionally blank
}

C_SphereTriggerCreate::~C_SphereTriggerCreate()
{
	// intentionally blank
}

#ifndef CFG_GOLD
void C_SphereTriggerCreate::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43 &actorMatrix) const
{
	debugRenderPtr->Draw3dSphere(m_Sphere, actorMatrix, Maths::RGBA(255,255,255));
}
#endif

void C_SphereTriggerCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	I_VolumeTriggerCreate::Serialise(stream);
	stream << m_Sphere;
}

void C_SphereTriggerCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	I_VolumeTriggerCreate::Deserialise(stream);
	stream >> m_Sphere;
}

// Trigger Source Component
void I_TriggerSourceCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_TriggerLayerIndex;
	stream << m_TriggerFlags;
}
void I_TriggerSourceCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_TriggerLayerIndex;
	stream >> m_TriggerFlags;
}
void C_TriggerSourceSphereCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	I_TriggerSourceCreate::Serialise(stream);
	stream << m_Sphere;
}

void C_TriggerSourceSphereCreate::Deserialise(Sys::C_ContextualStream& stream)		
{
	I_TriggerSourceCreate::Deserialise(stream);
	stream >> m_Sphere;

}
void C_TriggerSourceBoxCreate::Serialise(Sys::C_ContextualStream& stream) const	
{
	I_TriggerSourceCreate::Serialise(stream);
	stream << m_Size;
	stream << m_Matrix;

}
void C_TriggerSourceBoxCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	I_TriggerSourceCreate::Deserialise(stream);
	stream >> m_Size;
	stream >> m_Matrix;
}

} // namespace Components
