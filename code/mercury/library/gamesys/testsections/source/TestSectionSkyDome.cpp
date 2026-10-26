#include "stdafx.h"
#include "TestSections/TestSectionSkyDome.h"
#include <Render/Sky/SkyDome.h>
#include <Render/Render/GridRender.h>

namespace Framework
{

C_TestSectionSkyDome::C_TestSectionSkyDome(const Util::C_String& name) : C_TestSection(name)
{
	m_pSkyDome = new Render::Sky::C_SkyDome();
}

C_TestSectionSkyDome::~C_TestSectionSkyDome()
{
	SAFE_DELETE_PTR(m_pSkyDome);
}

void C_TestSectionSkyDome::OnRender3DViewWithAA(const S32 viewIndex)
{
	m_pSkyDome->Render();

	Render::DrawGrid();
}

}; // namespace Framework
