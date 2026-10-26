#include "stdafx.h"
#include "Effect/PostProcess/DoFBase.h"

namespace PostProcess
{

Debug::Bool		g_debugDoFOverridePlaneInFocus	("Graphics.Effect.PostProcess.DepthOfField.OverridePlaneInFocus",	false);
Debug::Float	g_debugDoFPlaneInFocus			("Graphics.Effect.PostProcess.DepthOfField.PlaneInFocus",			5.0f,	Debug::Range(0.0f, 50.0f));
Debug::Bool		g_debugDoFOverrideAperture		("Graphics.Effect.PostProcess.DepthOfField.OverrideAperture",		false);
Debug::Float	g_debugDoFAperture				("Graphics.Effect.PostProcess.DepthOfField.Aperture",				0.0f,	Debug::Range(0.0f, 4.0f));
Debug::Float	g_debugDoFMinCoC				("Graphics.Effect.PostProcess.DepthOfField.MinCoC",					0.001f,	Debug::Range(0.0f, 0.1f));
Debug::Float	g_debugDoFFocalLength			("Graphics.Effect.PostProcess.DepthOfField.FocalLength",			0.035f,	Debug::Range(0.0f, 0.1f));
Debug::Bool		g_debugDepthOfField				("Graphics.Effect.PostProcess.DepthOfField.Enable",					true);

C_DoFBase::C_DoFBase()
{
}

C_DoFBase::~C_DoFBase()
{
	m_ViewportSettings.Clear();
}

F32 C_DoFBase::GetPlaneInFocus(const S32 iViewIndex) const
{
	if(iViewIndex >= 0 && iViewIndex < m_ViewportSettings.Size() && !g_debugDoFOverridePlaneInFocus)
		return m_ViewportSettings[iViewIndex].m_fPlaneInFocus;
	else
		return g_debugDoFPlaneInFocus;
}

F32 C_DoFBase::GetAperture(const S32 iViewIndex) const
{
	if(iViewIndex >= 0 && iViewIndex < m_ViewportSettings.Size() && !g_debugDoFOverrideAperture)
		return m_ViewportSettings[iViewIndex].m_fAperture;
	else
		return g_debugDoFAperture;
}

void C_DoFBase::SetDepthOfField(const S32 iViewIndex, const F32 fPlaneInFocus, const F32 fAperture)
{
	if (iViewIndex < 0)
		return;
	if (iViewIndex >= m_ViewportSettings.Size())
		m_ViewportSettings.Resize(iViewIndex + 1);
	
	m_ViewportSettings[iViewIndex].m_fPlaneInFocus = fPlaneInFocus;
	m_ViewportSettings[iViewIndex].m_fAperture = fAperture;
}

}; // namespace PostProcess
