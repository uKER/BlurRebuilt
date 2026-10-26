#include "stdafx.h"
#include "TestSections/TestSectionPostprocess.h"

C_TestSectionPostprocess::C_TestSectionPostprocess(const Util::C_String& name) : Framework::C_TestSectionHDR(name)
{
}

C_TestSectionPostprocess::~C_TestSectionPostprocess()
{
}

void C_TestSectionPostprocess::OnRenderHdr3DViewWithAA(const S32 viewIndex)
{
	m_SkyDome.Render();
}

void C_TestSectionPostprocess::OnRenderHdr3DViewWithoutAA(const S32 viewIndex)
{
}
