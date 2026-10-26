#include "stdafx.h"
#include "Render/Render/CubeTextureSaver.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Render
{

Debug::Float	g_debugCubeMapSaverScale	("Graphics.Render.Misc.CubeMapSaver.Scale", 200.0f);

C_CubeTextureSaver::C_CubeTextureSaver(const U32 width) : Display::I_Renderer("CubeTextureSaver"),
	m_Width(width)
{
}

C_CubeTextureSaver::~C_CubeTextureSaver()
{
	DestroyRenderer();
}

bool C_CubeTextureSaver::OnCreateVolatile()
{
	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	for(U32 i=0; i<Display::E_CubeTextureFace_Count; i++)
	{
		if(!Display::Device()->CreateRenderTexture2D(m_Width, m_Width, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, NULL, &m_RenderTexture[i]))
			return false;

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetRenderTexture2DName(&m_RenderTexture[i], "_CubeTextureSaverRT");
		#endif
	}

	Display::C_SurfaceParams surfaceParams;


	if(!Display::Device()->CreateDepthStencilSurface(m_Width, m_Width, Display::E_Usage_DepthStencil, backBuffer.GetDepthStencilFormat(), Display::E_Multisample_None, &surfaceParams, &m_DepthStencilSurface))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetSurfaceName(&m_DepthStencilSurface, "_CubeTextureSaverDS");
	#endif

	return true;
}

void C_CubeTextureSaver::OnDestroyVolatile()
{
	for(U32 i=0; i<Display::E_CubeTextureFace_Count; i++)
	{
		Display::Device()->Destroy(&m_RenderTexture[i]);
	}
	Display::Device()->Destroy(&m_DepthStencilSurface);
}

void C_CubeTextureSaver::Bind(const Display::E_CubeTextureFace face)
{
#ifdef PLATFORM_PS3
	assert(0 && "Not supported yet on PS3...see LloydW about eta!");
#else // PLATFORM_PS3
	assert(face < Display::E_CubeTextureFace_Count);

	Display::Device()->SetColourDepthStencilSurface(&m_RenderTexture[face].GetSurface(), &m_DepthStencilSurface);
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(255, 255, 255, 255));
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);

#endif // PLATFORM_PS3
}

void C_CubeTextureSaver::Unbind(const Display::E_CubeTextureFace face)
{
	assert(face < Display::E_CubeTextureFace_Count);


	Display::Device()->SetView(NULL, false);
}

void C_CubeTextureSaver::DebugRender()
{
	if(!m_DepthStencilSurface.IsCreated())
		return;
	
	F32 s = g_debugCubeMapSaverScale;
	F32 x = 512.0f - (s/2.0f);
	F32 y = 384.0f - (s/2.0f);

	if(m_RenderTexture[Display::E_CubeTextureFace_PosX].IsCreated())
	{
		Display::C_Float2 uv[4] =
		{
			Display::C_Float2(1.0f, 0.0f), Display::C_Float2(1.0f, 1.0f),
			Display::C_Float2(0.0f, 0.0f), Display::C_Float2(0.0f, 1.0f),
		};
		Display::BasicRender()->DrawTextured2DRect(x+s, y, s, s, uv, Maths::RGBA(255, 255, 255, 255), &m_RenderTexture[Display::E_CubeTextureFace_PosX].GetTexture());
	}
	if(m_RenderTexture[Display::E_CubeTextureFace_PosY].IsCreated())
	{
		Display::C_Float2 uv[4] =
		{
			Display::C_Float2(0.0f, 0.0f), Display::C_Float2(1.0f, 0.0f),
			Display::C_Float2(0.0f, 1.0f), Display::C_Float2(1.0f, 1.0f),
		};
		Display::BasicRender()->DrawTextured2DRect(x, y, s, s, uv, Maths::RGBA(255, 255, 255, 255), &m_RenderTexture[Display::E_CubeTextureFace_PosY].GetTexture());
	}
	if(m_RenderTexture[Display::E_CubeTextureFace_PosZ].IsCreated())
	{
		Display::C_Float2 uv[4] =
		{
			Display::C_Float2(0.0f, 0.0f), Display::C_Float2(1.0f, 0.0f),
			Display::C_Float2(0.0f, 1.0f), Display::C_Float2(1.0f, 1.0f),
		};
		Display::BasicRender()->DrawTextured2DRect(x, y+s, s, s, uv, Maths::RGBA(255, 255, 255, 255), &m_RenderTexture[Display::E_CubeTextureFace_PosZ].GetTexture());
	}
	if(m_RenderTexture[Display::E_CubeTextureFace_NegX].IsCreated())
	{
		Display::C_Float2 uv[4] =
		{
			Display::C_Float2(0.0f, 1.0f), Display::C_Float2(0.0f, 0.0f),
			Display::C_Float2(1.0f, 1.0f), Display::C_Float2(1.0f, 0.0f),
		};
		Display::BasicRender()->DrawTextured2DRect(x-s, y, s, s, uv, Maths::RGBA(255, 255, 255, 255), &m_RenderTexture[Display::E_CubeTextureFace_NegX].GetTexture());
	}
	if(m_RenderTexture[Display::E_CubeTextureFace_NegY].IsCreated())
	{
		Display::C_Float2 uv[4] =
		{
			Display::C_Float2(1.0f, 1.0f), Display::C_Float2(0.0f, 1.0f),
			Display::C_Float2(1.0f, 0.0f), Display::C_Float2(0.0f, 0.0f),
		};
		Display::BasicRender()->DrawTextured2DRect(x+s+s, y, s, s, uv, Maths::RGBA(255, 255, 255, 255), &m_RenderTexture[Display::E_CubeTextureFace_NegY].GetTexture());
	}
	if(m_RenderTexture[Display::E_CubeTextureFace_NegZ].IsCreated())
	{
		Display::C_Float2 uv[4] =
		{
			Display::C_Float2(1.0f, 1.0f), Display::C_Float2(0.0f, 1.0f),
			Display::C_Float2(1.0f, 0.0f), Display::C_Float2(0.0f, 0.0f),
		};
		Display::BasicRender()->DrawTextured2DRect(x, y-s, s, s, uv, Maths::RGBA(255, 255, 255, 255), &m_RenderTexture[Display::E_CubeTextureFace_NegZ].GetTexture());
	}
}

bool C_CubeTextureSaver::Save(const Util::C_String& fileName)
{
	for(U32 i=0; i<Display::E_CubeTextureFace_Count; i++)
	{
		if(!SaveFace((Display::E_CubeTextureFace)i, fileName))
			return false;
	}

	return true;
}

bool C_CubeTextureSaver::SaveFace(const Display::E_CubeTextureFace face, const Util::C_String& fileName)
{
	#if 0
	Util::C_String filePath = Display::FormatString(Sys::GetDefaultFileSystem()->GetWorkingDirectory() + "/" + fileName);
	#else
	Util::C_String filePath = Display::FormatString(fileName);
	#endif

	switch (face)
	{
	case Display::E_CubeTextureFace_PosX:
		filePath += "PosX.dds";
		break;
	case Display::E_CubeTextureFace_PosY:
		filePath += "PosY.dds";
		break;
	case Display::E_CubeTextureFace_PosZ:
		filePath += "PosZ.dds";
		break;
	case Display::E_CubeTextureFace_NegX:
		filePath += "NegX.dds";
		break;
	case Display::E_CubeTextureFace_NegY:
		filePath += "NegY.dds";
		break;
	case Display::E_CubeTextureFace_NegZ:
		filePath += "NegZ.dds";
		break;
	}


	return true;
}

} // namespace Render
