#include "stdafx.h"
#include "Render/Render/DepthRender.h"

EXTERN_SHADER(vsDepthRender);
EXTERN_SHADER(psDepthRender);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Render/PS3/vsDepthRender.shader"
#include "Render/Shaders/Render/PS3/psDepthRender.shader"
#endif
#endif

namespace Render
{

C_DepthRenderShaders C_DepthRender::m_Shaders;

C_DepthRender::C_DepthRender() : Display::I_Renderer("DepthRender"),
	m_Width(256),
	m_Height(256)
{
	m_View.SetType(Display::E_ViewType_3D);

	m_ViewSetup.m_ViewSetup3D.m_FarClip = 10.f;
	m_ViewSetup.m_ViewSetup3D.m_PerspDist = 0.f;
	m_ViewSetup.m_ViewSetup3D.m_ViewExtents.SetBounds(-1.f, -1.f, 1.f, 1.f);
	
	LookAt(Maths::HVec3(0.f, 5.f, -5.f), Maths::HVec3(0.f, -1.f, 1.f));

	m_NewViewport.m_W = m_Width;
	m_NewViewport.m_H = m_Height;
}

C_DepthRender::~C_DepthRender()
{
	DestroyRenderer();
}

bool C_DepthRender::OnCreate()
{
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsDepthRender", 0, &m_Shaders.m_VertexShader, REF_SHADER(vsDepthRender)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psDepthRender", 0, &m_Shaders.m_PixelShader, REF_SHADER(psDepthRender)))
		return false;

	return true;
}

bool C_DepthRender::OnCreateVolatile()
{
	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
	
	#if 1
	Display::E_Format format = Display::E_Format_R32F;
	#else
	Display::E_Format format = Display::E_Format_A8R8G8B8;
	#endif
	if (!Display::Device()->CreateRenderTexture2D(m_Width, m_Height, 1, Display::E_Usage_Colour, format, Display::E_Multisample_None, NULL, &m_RenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_DepthRenderRT");
	#endif

	Display::C_SurfaceParams surfaceParams;

	if (!Display::Device()->CreateDepthStencilSurface(m_Width, m_Height, Display::E_Usage_DepthStencil, backBuffer.GetDepthStencilFormat(), Display::E_Multisample_None, &surfaceParams, &m_DepthStencilSurface))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetSurfaceName(&m_DepthStencilSurface, "_DepthRenderDS");
	#endif

	return true;
}

void C_DepthRender::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_RenderTexture);
	Display::Device()->Destroy(&m_DepthStencilSurface);
}

void C_DepthRender::OnDestroy()
{
	Display::Device()->Destroy(&m_Shaders.m_VertexShader);
	Display::Device()->Destroy(&m_Shaders.m_PixelShader);
}

void C_DepthRender::Bind()
{
	m_OldViewport = Display::Device()->GetViewport();

	Display::Device()->SetColourDepthStencilSurface(&m_RenderTexture.GetSurface(), &m_DepthStencilSurface);
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(255, 255, 255, 255));
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = viewSettings3D.GetCameraToWorld();

	Display::Device()->SetupView(&m_View, m_ViewSetup);

	#if 0
	const Maths::HVec3 pos(Maths::HVec3( 0.5f,  0.5f,  0.5f));
	const Maths::HVec3 dir(Maths::HVec3(-0.5f, -0.1f, -0.5f));
	m_View.m_ViewSettings3D.LookAt(pos, dir);
	#endif
	
	Display::Device()->SetView(&m_View);
	Display::Device()->SetViewport(m_NewViewport);

	Display::Device()->SetVertexShader(&m_Shaders.m_VertexShader);
	Display::Device()->SetPixelShader(&m_Shaders.m_PixelShader);
}

void C_DepthRender::Unbind()
{
	Display::Device()->SetView(NULL, false);

	Display::Device()->SetColourDepthStencilSurface();
	Display::Device()->SetViewport(m_OldViewport);
}

void C_DepthRender::LookAt(const Maths::HVec3& pos, const Maths::HVec3& dir)
{
	Maths::HVec3 tox,toy,toz;
	Maths::HMat33 look;
	toz = dir;
	toz.Normalise();
	tox = Maths::HVec3(0.f,1.f,0.f)^toz;
	tox.Normalise();
	toy = toz^tox;
	toy.Normalise();
	look.AX(tox);
	look.AY(toy);
	look.AZ(toz);
	m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = Maths::HMat43(look);
	m_ViewSetup.m_ViewSetup3D.m_CameraToWorld.AW(pos);
}

} // namespace Render
