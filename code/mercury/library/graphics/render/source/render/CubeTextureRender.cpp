#include "stdafx.h"
#include "Render/Render/CubeTextureRender.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Render
{

Debug::Float	g_debugCubeMapRenderScale	("Graphics.Render.Misc.CubeMapRender.Scale", 200.0f);
Debug::Bool		g_debugCubeMapClearColour	("Graphics.Render.Misc.CubeMapRender.ClearColour", false);

C_CubeTextureRender::C_CubeTextureRender(const U32 _width, Display::E_Multisample multiSample) : 
Display::I_Renderer("CubeTextureRender"),
m_Width(_width),
m_MultiSample (multiSample)
{
}

C_CubeTextureRender::~C_CubeTextureRender()
{
	DestroyRenderer();
}

const Display::C_TextureCube* C_CubeTextureRender::GetTexture()
{
	return &m_RenderTexture.GetTexture();
}

bool C_CubeTextureRender::OnCreateVolatile()
{
	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	const Display::E_Format format = backBuffer.GetColourFormat();

	Display::C_SurfaceParams surfaceParams;

	if(!Display::Device()->CreateRenderTextureCube(m_Width, 1, Display::E_Usage_Colour, format, m_MultiSample, &surfaceParams, &m_RenderTexture))
		return false;
/*
#ifdef PLATFORM_XENON
	if(m_RenderTexture.m_Texture.Get())
	{
		// CUBEMAP SRGB
		m_RenderTexture.m_Texture.Get()->Format.SignX = 3;
		m_RenderTexture.m_Texture.Get()->Format.SignY = 3;
		m_RenderTexture.m_Texture.Get()->Format.SignZ = 3;
	}
#endif*/


	if(!Display::Device()->CreateDepthStencilSurface(m_Width, m_Width, Display::E_Usage_DepthStencil, backBuffer.GetDepthStencilFormat(), m_MultiSample, &surfaceParams, &m_DepthStencilSurface))
		return false;

	return true;
}

void C_CubeTextureRender::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_RenderTexture);
	Display::Device()->Destroy(&m_DepthStencilSurface);
}

void C_CubeTextureRender::Bind(const Display::E_CubeTextureFace face)
{
	assert(face < Display::E_CubeTextureFace_Count);

	Display::Device()->SetColourDepthStencilSurface(&m_RenderTexture.GetSurface(face), &m_DepthStencilSurface);
	
	//In XBox only clear the first time
		if (!g_debugCubeMapClearColour)
			Display::Device()->Clear(Display::E_Clear_Depth, Maths::RGBA(255, 0, 0, 255));
		else
			Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(255, 0, 0, 255));

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
}

void C_CubeTextureRender::Unbind(const Display::E_CubeTextureFace face)
{
	assert(face < Display::E_CubeTextureFace_Count);


	Display::Device()->SetView(NULL, false);
}


void C_CubeTextureRender::DebugRender()
{
	if(!m_DepthStencilSurface.IsCreated())
		return;
	if(!m_RenderTexture.IsCreated())
		return;

	F32 s = g_debugCubeMapRenderScale;
	F32 x = 512.0f - (s/2.0f);
	F32 y = 384.0f - (s/2.0f);

}
} // namespace Render
