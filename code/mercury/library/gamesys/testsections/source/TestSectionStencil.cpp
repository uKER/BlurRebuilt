#include "stdafx.h"
#include "TestSections/TestSectionStencil.h"
#include <Render/Sky/SkyDome.h>
#include <Render/Render/GridRender.h>

namespace Framework
{

C_TestSectionStencil::C_TestSectionStencil(const Util::C_String& name) : C_TestSection(name)
{
	m_pSkyDome = new Render::Sky::C_SkyDome();
}

C_TestSectionStencil::~C_TestSectionStencil()
{
	SAFE_DELETE_PTR(m_pSkyDome);
}

void C_TestSectionStencil::OnRender3DViewWithAA(const S32 viewIndex)
{
	m_pSkyDome->Render();

	Render::DrawGrid();
}

void C_TestSectionStencil::OnRender2DView()
{
	Display::Device()->Clear(Display::E_Clear_Stencil);

	Display::C_StencilState stencilState;

	stencilState.m_StencilEnable		= Display::E_Bool_True;
	stencilState.m_StencilFunc			= Display::E_CmpFunc_Equal;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail		= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass		= Display::E_StencilOp_Incr;
	stencilState.m_StencilRef			= 0;
	stencilState.m_StencilMask			= 0xFFFFFFFF;
	stencilState.m_StencilWriteMask		= 0xFFFFFFFF;

	Display::Device()->SetStencilState(stencilState);
	Display::BasicRender()->DrawColoured2DRect(0.0f, 0.0f, 200.0f, 200.0f, Maths::RGBA(255, 0, 0, 255));
	Display::BasicRender()->DrawColoured2DRect(50.0f, 50.0f, 150.0f, 150.0f, Maths::RGBA(255, 0, 255, 255));

	stencilState.m_StencilRef			= 1;
	Display::Device()->SetStencilState(stencilState);
	Display::BasicRender()->DrawColoured2DRect(100.0f, 100.0f, 200.0f, 200.0f, Maths::RGBA(0, 255, 0, 255));

	stencilState.m_StencilRef			= 2;
	Display::Device()->SetStencilState(stencilState);
	Display::BasicRender()->DrawColoured2DRect(150.0f, 150.0f, 200.0f, 200.0f, Maths::RGBA(0, 0, 255, 255));

	stencilState.m_StencilRef			= 3;
	Display::Device()->SetStencilState(stencilState);
	Display::BasicRender()->DrawColoured2DRect(175.0f, 175.0f, 200.0f, 200.0f, Maths::RGBA(255, 255, 255, 255));

	stencilState.m_StencilEnable		= Display::E_Bool_False;
	Display::Device()->SetStencilState(stencilState);
}

}; // namespace Framework
