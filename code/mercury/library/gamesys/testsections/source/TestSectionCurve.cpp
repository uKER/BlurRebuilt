#include "stdafx.h"
#include "TestSections/TestSectionCurve.h"
#include <Render/Sky/SkyDome.h>
#include <Render/Render/GridRender.h>
#include <Misc_Shared/NewCurve/HelloWorld.h>

namespace Framework
{

C_TestSectionCurve::C_TestSectionCurve(const Util::C_String& name) : C_TestSection(name)
{
	m_pSkyDome = new Render::Sky::C_SkyDome();
	m_param = 0;
}

C_TestSectionCurve::~C_TestSectionCurve()
{
	SAFE_DELETE_PTR(m_pSkyDome);
}

void C_TestSectionCurve::OnUpdate()
{
	m_param += 0.01f;
	if (m_param > 1.f)
		m_param = 0;
}

void C_TestSectionCurve::OnRender3DViewWithAA(const S32 viewIndex)
{
	m_pSkyDome->Render();

	Render::DrawGrid();

	Curve::HelloWorld(m_param);
}

}; // namespace Framework
