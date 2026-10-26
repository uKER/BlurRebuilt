#include "stdafx.h"
#include "TestSectionTemplate.h"

C_TestSectionTemplate::C_TestSectionTemplate(const Util::C_String& name) : Framework::C_TestSection(name)
{
}

C_TestSectionTemplate::~C_TestSectionTemplate()
{
}

void C_TestSectionTemplate::OnRender2DView()
{
	Display::FontRender()->Draw("Hello!", 1024.f/2.f, 768.f/2.f, 50.f, 50.f, Maths::RGBA(255, 255, 255, 255), Display::E_FontRenderAlign_Ct);
}
