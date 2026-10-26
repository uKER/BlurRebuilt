#include "StdAfx.h"
#include "Samples/Section/SectionGlobalGraphicsBuffers.h"
#include "Render/Utils/Globals.h"

namespace Samples
{

C_SectionGlobalGraphicsBuffers::C_SectionGlobalGraphicsBuffers(const Util::C_String& name) : C_Section(name)
{
}

C_SectionGlobalGraphicsBuffers::~C_SectionGlobalGraphicsBuffers()
{
}

bool C_SectionGlobalGraphicsBuffers::OnCreateVolatile()
{
	if (!Render::CreateGlobalGraphicsBuffers())
		return false;
	
	return true;
}

void C_SectionGlobalGraphicsBuffers::OnDestroyVolatile()
{
	Render::DestroyGlobalGraphicsBuffers();
}

void C_SectionGlobalGraphicsBuffers::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionGlobalGraphicsBuffers");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::BLUE);
	Display::FontRender()->Draw("HELLO!", 50, 50, 100, 100, Maths::RGBA(255, 255, 255, 255));

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
	Display::Device()->ResolveSurface(backBuffer.GetColourSurface(), &Render::g_ColourBuffer);
	
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::GREY);
	Display::BasicRender()->DrawTextured2DRect(100.f, 100.f, 1024.f - 200.f, 768.f - 200.f, Display::WHITE, &Render::g_ColourBuffer.m_Texture);
}

}; // namespace Samples
