#include <nexus_render/stdafx.h>

#include <nexus_render/EffectConstants.h>
#include <nexus_render/GizmoEffectConstants.h>

using namespace NexusRender;

GizmoEffectConstants::GizmoEffectConstants()
{
	m_mWorldViewProjectionHandle = 0;
	m_colourHandle = 0;
}

GizmoEffectConstants::GizmoEffectConstants(const GizmoEffectConstants& rhs)
{
	m_mWorldViewProjectionHandle = rhs.m_mWorldViewProjectionHandle;
	m_colourHandle = rhs.m_colourHandle;
}

GizmoEffectConstants::~GizmoEffectConstants()
{
}

Tu::Status GizmoEffectConstants::Build() 
{
	return Tu::Status::SUCCESS; 
}

U32 GizmoEffectConstants::GetHash() const
{
	return 0;
}

bool GizmoEffectConstants::operator==(const EffectConstants& effectConstants) const
{
	if (GetEffectType() != effectConstants.GetEffectType())
		return false;

	return true;
}

