#include "stdafx.h"
#include "Render/Render/TextureRender.h"

Debug::Bool		g_debugTextureRenderClearColour	("Graphics.Render.Misc.TextureRender.ClearColour", false);

namespace Render
{
C_TextureRender::C_TextureRender(const U32 width, const U32 height, const Display::E_Multisample multisample, const U32 numMips) : Display::I_Renderer("TextureRender"),
	m_Width(width),
	m_Height(height),
	m_MultiSample(multisample),
	m_NumMips(numMips)
{
}

C_TextureRender::~C_TextureRender()
{
	DestroyRenderer();
}

#if PLATFORM_PC
void C_TextureRender::SetFormat(const U32 width, const U32 height, const Display::E_Multisample multisample)
{
	m_Width = width;
	m_Height = height;
	m_MultiSample = multisample;
	m_NumMips = 1;
}
#endif

bool C_TextureRender::OnCreateVolatile()
{
	MEMORY_TRACKER_PUSH("graphics.renderers.texturerender");

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

		const Display::E_Format format = backBuffer.GetColourFormat();
	
	if (!Display::Device()->CreateRenderTexture2D(m_Width, m_Height, m_NumMips, Display::E_Usage_Colour, format, m_MultiSample, NULL, &m_RenderTexture))
	{
		MEMORY_TRACKER_POP("graphics.renderers.texturerender");
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_TextureRenderRT");
	#endif
/*
#ifdef PLATFORM_XENON
	if(m_RenderTexture.GetTexture().Get())
	{
		// CUBEMAP SRGB
		m_RenderTexture.GetTexture().Get()->Format.SignX = 3;
		m_RenderTexture.GetTexture().Get()->Format.SignY = 3;
		m_RenderTexture.GetTexture().Get()->Format.SignZ = 3;
	}
#endif*/

	Display::C_SurfaceParams surfaceParams;

	if (!Display::Device()->CreateDepthStencilSurface(m_Width, m_Height, Display::E_Usage_DepthStencil, backBuffer.GetDepthStencilFormat(), m_MultiSample, &surfaceParams, &m_DepthStencilSurface))
	{
		MEMORY_TRACKER_POP("graphics.renderers.texturerender");
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetSurfaceName(&m_DepthStencilSurface, "_TextureRenderDS");
	#endif

	MEMORY_TRACKER_POP("graphics.renderers.texturerender");

	return true;
}

void C_TextureRender::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_RenderTexture);
	Display::Device()->Destroy(&m_DepthStencilSurface);
}

void C_TextureRender::Bind() const
{
#ifdef PLATFORM_PS3
	Display::Device()->SetZcullEnable(true, true);
#endif
	Display::Device()->SetColourDepthStencilSurface(&m_RenderTexture.GetSurface(), &m_DepthStencilSurface);
	if (!g_debugTextureRenderClearColour)
		Display::Device()->Clear(Display::E_Clear_Depth);
	else
		Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0,0,0,255));
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);

	/*const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = viewSettings3D.GetCameraToWorld();

	Display::Device()->SetupView(&m_View, m_ViewSetup, m_Width, m_Height);
	Display::Device()->SetView(&m_View);*/
}

void C_TextureRender::Unbind() const
{
	#if PLATFORM_PS3
	Display::Device()->SetZcullEnable(true, true);
	#elif PLATFORM_PC
	// [Blur2PC] retail Blur PC's unbind (Blur.exe 1.2 0x0068ca20) is only this: copies a multisampled render
	// target into the texture (0x005adff0); it does not reset the view
	m_RenderTexture.Resolve();
	return;
	#endif

	Display::Device()->SetView(NULL, false);
}

void C_TextureRender::DebugRender() const
{
	if (!m_RenderTexture.IsCreated())
		return;

	Display::BasicRender()->DrawTextured2DRect(1024.f - 300.f - 20.f, 20.f, 300.f, 300.f, Maths::RGBA(255, 255, 255, 255), &m_RenderTexture.GetTexture());
}

} // namespace Render
