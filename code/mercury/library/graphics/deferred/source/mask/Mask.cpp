#include "stdafx.h"
#include "Deferred/Mask/Mask.h"
#include "Display/Buffer/VertexDeclarations.h"

EXTERN_SHADER(psMask);
EXTERN_SHADER(vsMask);

namespace Deferred
{

bool C_MaskShaders::Create()
{
	RESOURCE_MEMORY_ID("Deferred::C_MaskShaders");
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMask", 0, &m_PixelShader, REF_SHADER(psMask)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsMask", 0, &m_VertexShader, REF_SHADER(vsMask)))
		return false;
	return true;
}

void C_MaskShaders::Destroy()
{
	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);
}

S32 C_Mask::m_RefCount = 0;
C_MaskShaders C_Mask::m_Shaders;

C_Mask::C_Mask() : m_MaskBegin(0.0f), m_MaskEnd(1.0f)
{
	if (m_RefCount == 0)
		m_Shaders.Create();
	++m_RefCount;
}

C_Mask::~C_Mask()
{
	--m_RefCount;
	if (m_RefCount == 0)
		m_Shaders.Destroy();
}

void C_Mask::Begin() const
{

	Display::C_StencilState stencilState;
	stencilState.m_StencilEnable		= Display::E_Bool_True;
	stencilState.m_StencilFunc			= Display::E_CmpFunc_Equal;
	stencilState.m_StencilRef			= 0xFF;
	stencilState.m_StencilMask			= 0xFF;
	stencilState.m_StencilWriteMask		= 0x00;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail 	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass 	= Display::E_StencilOp_Keep;	
	Display::Device()->SetStencilState(stencilState);
}

void C_Mask::End() const
{

	Display::C_StencilState stencilState;
	stencilState.m_StencilEnable		= Display::E_Bool_False;
	stencilState.m_StencilFunc			= Display::E_CmpFunc_Equal;
	stencilState.m_StencilRef			= 0xFF;
	stencilState.m_StencilMask			= 0xFF;
	stencilState.m_StencilWriteMask		= 0xFF;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail 	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass 	= Display::E_StencilOp_Keep;	
	Display::Device()->SetStencilState(stencilState);
#if PLATFORM_PS3
	cellGcmSetScullControl(CELL_GCM_SCULL_SFUNC_EQUAL, 0xFF, 0xFF);
	Display::Device()->SetZcullEnable(true, false);
#endif
}

void C_Mask::MaskAfter(const F32 fBegin) const
{
	PERF_EVENT("Deferred.MaskAfter");

#if defined(PLATFORM_PS3)
	Display::Device()->SetZcullEnable(true, true);
	cellGcmSetStencilMask(0xFF);
	cellGcmSetScullControl(CELL_GCM_SCULL_SFUNC_EQUAL, 0xFF, 0xFF);
	cellGcmSetClearDepthStencil(0x00000000);
	cellGcmSetClearSurface(CELL_GCM_CLEAR_S);
#else
	Display::Device()->Clear(Display::E_Clear_Stencil, Maths::RGBA(0x00000000), 0.0f, 0x00);
#endif

	Display::C_StencilState stencilState;
	stencilState.m_StencilEnable		= Display::E_Bool_True;
	stencilState.m_StencilFunc			= Display::E_CmpFunc_Always;
	stencilState.m_StencilMask			= 0x00;
	stencilState.m_StencilRef			= 0xFF;
	stencilState.m_StencilWriteMask		= 0xFF;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail 	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass 	= Display::E_StencilOp_Replace;	
	Display::Device()->SetStencilState(stencilState);

	DrawDepth(fBegin);

	stencilState.m_StencilEnable = Display::E_Bool_False;
	Display::Device()->SetStencilState(stencilState);
}

void C_Mask::MaskBefore(const F32 fEnd) const
{
	PERF_EVENT("Deferred.MaskBefore");

#if defined(PLATFORM_PS3)
	Display::Device()->SetZcullEnable(true, true);
	cellGcmSetStencilMask(0xFF);
	cellGcmSetScullControl(CELL_GCM_SCULL_SFUNC_EQUAL, 0xFF, 0xFF);
	cellGcmSetClearDepthStencil(0x000000FF);
	cellGcmSetClearSurface(CELL_GCM_CLEAR_S);
#else
	Display::Device()->Clear(Display::E_Clear_Stencil, Maths::RGBA(0x00000000), 0.0f, 0xFF);
#endif

	Display::C_StencilState stencilState;
	stencilState.m_StencilEnable		= Display::E_Bool_True;
	stencilState.m_StencilFunc			= Display::E_CmpFunc_Always;
	stencilState.m_StencilMask			= 0x00;
	stencilState.m_StencilRef			= 0x00;
	stencilState.m_StencilWriteMask		= 0xFF;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail 	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass 	= Display::E_StencilOp_Replace;	
	Display::Device()->SetStencilState(stencilState);

	DrawDepth(fEnd);

	stencilState.m_StencilEnable = Display::E_Bool_False;
	Display::Device()->SetStencilState(stencilState);
}

void C_Mask::MaskBetween(const F32 fBegin, const F32 fEnd) const
{
	PERF_EVENT("Deferred.MaskBetween");

#if defined(PLATFORM_PS3)
	Display::Device()->SetZcullEnable(true, true);
	cellGcmSetStencilMask(0xFF);
	cellGcmSetScullControl(CELL_GCM_SCULL_SFUNC_EQUAL, 0xFF, 0xFF);
	cellGcmSetClearDepthStencil(0x00000000);
	cellGcmSetClearSurface(CELL_GCM_CLEAR_S);
#else
	Display::Device()->Clear(Display::E_Clear_Stencil, Maths::RGBA(0x00000000), 0.0f, 0x00);
#endif

	Display::C_StencilState stencilState;
	stencilState.m_StencilEnable		= Display::E_Bool_True;
	stencilState.m_StencilFunc			= Display::E_CmpFunc_Always;
	stencilState.m_StencilRef			= 0xFF;
	stencilState.m_StencilMask			= 0x00;
	stencilState.m_StencilWriteMask		= 0xFF;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail 	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass 	= Display::E_StencilOp_Replace;	
	Display::Device()->SetStencilState(stencilState);

	DrawDepth(fBegin);

	stencilState.m_StencilMask			= 0xFF;
	stencilState.m_StencilRef			= 0x00;
	Display::Device()->SetStencilState(stencilState);

	DrawDepth(fEnd);

	stencilState.m_StencilEnable = Display::E_Bool_False;
	Display::Device()->SetStencilState(stencilState);
}

void C_Mask::ReloadHiStencil() const
{
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_None);


	Display::C_StencilState stencilState;
	stencilState.m_StencilEnable		= Display::E_Bool_True;
	stencilState.m_StencilFunc			= Display::E_CmpFunc_Always;
	stencilState.m_StencilMask			= 0xFF;
	stencilState.m_StencilRef			= 0x00;
	stencilState.m_StencilWriteMask		= 0x00;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail 	= Display::E_StencilOp_Invert;
	stencilState.m_StencilDepthPass 	= Display::E_StencilOp_Keep;	
	Display::Device()->SetStencilState(stencilState);

	Display::Quad::DrawFullScreenColour(Maths::C_RGBA_F32(0.0f, 0.0f, 0.0f, 0.0f));

	stencilState.m_StencilEnable		= Display::E_Bool_False;
	stencilState.m_StencilWriteMask		= 0xFF;
	stencilState.m_StencilFail			= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthFail 	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass 	= Display::E_StencilOp_Keep;	
	Display::Device()->SetStencilState(stencilState);


	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
}

void C_Mask::DrawDepth(const F32 fZ) const
{
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_None);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	Display::Device()->SetVertexShader(&m_Shaders.m_VertexShader);
	Display::Device()->SetPixelShader(&m_Shaders.m_PixelShader);

	F32 fNearClip = Display::Device()->GetViewSettings3D().GetNearClip();
	F32 fFarClip = Display::Device()->GetViewSettings3D().GetFarClip();
	if (Display::Device()->GetDepthFlip())
	{
		fNearClip = Display::Device()->GetViewSettings3D().GetFarClip();
		fFarClip = Display::Device()->GetViewSettings3D().GetNearClip();
	}

	const F32 fDepth = (fFarClip * fZ - fNearClip * fFarClip) / (fZ * (fFarClip - fNearClip));
	Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleStrip, 4, sizeof(Display::Vertex_XYZ), Display::Vertex_XYZ::m_pDeclaration);
	if (pVertex)
	{
		pVertex->x = -1.0f; pVertex->y = -1.0f; pVertex->z = fDepth; pVertex++;
		pVertex->x = +1.0f; pVertex->y = -1.0f; pVertex->z = fDepth; pVertex++;
		pVertex->x = -1.0f; pVertex->y = +1.0f; pVertex->z = fDepth; pVertex++;
		pVertex->x = +1.0f; pVertex->y = +1.0f; pVertex->z = fDepth; pVertex++;
		Display::Device()->EndVertices();
	}

	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthEnable(Display::E_Bool_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_Default);
}

}; // namespace Deferred
