#include "stdafx.h"
#include "Display/Render/ResourceRender.h"
#include "Display/Render/BasicRender.h"
#include "Display/Render/FontRender.h"
#include "Display/Device/Device.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

Debug::Int		g_debugDisplayResourceRenderTransparency(	"Graphics.Display.Render.ResourceRender.Transparency",	220, Debug::Range(0, 255));
Debug::Int		g_debugDisplayResourceRenderType(			"Graphics.Display.Render.ResourceRender.Type",			Display::E_ResourceRender_Texture, Debug::Range(Display::E_ResourceRender_Head + 1, Display::E_ResourceRender_Tail - 1));
Debug::Float	g_debugDisplayResourceRenderX(				"Graphics.Display.Render.ResourceRender.X",				50.0f, Debug::Range(0.0f, 1024.0f));
Debug::Float	g_debugDisplayResourceRenderY(				"Graphics.Display.Render.ResourceRender.Y",				50.0f, Debug::Range(0.0f, 1024.0f));
Debug::Float	g_debugDisplayResourceRenderSize(			"Graphics.Display.Render.ResourceRender.Size",			16.0f, Debug::Range(0.0f, 32.0f));

namespace Display
{

C_ResourceRender::C_ResourceRender()
{
	BIZ_ASSERT(g_pResourceRender == NULL);
	g_pResourceRender = this;
}

C_ResourceRender::~C_ResourceRender()
{
	BIZ_ASSERT(g_pResourceRender == this);
	g_pResourceRender = NULL;
}

void C_ResourceRender::Draw()
{
	BasicRender()->DrawColoured2DRect(20.f, 20.f, 984.f, 728.f, Maths::RGBA(0, 0, 0, g_debugDisplayResourceRenderTransparency));
	
	switch(g_debugDisplayResourceRenderType)
	{
	case E_ResourceRender_Texture:
		DrawTexture();
		break;
	case E_ResourceRender_Surface:
		DrawSurface();
		break;
	case E_ResourceRender_VertexDeclaration:
		DrawVertexDeclaration();
		break;
	case E_ResourceRender_VertexBuffer:
		DrawVertexBuffer();
		break;
	case E_ResourceRender_IndexBuffer:
		DrawIndexBuffer();
		break;
	case E_ResourceRender_VertexShader:
		DrawVertexShader();
		break;
	case E_ResourceRender_PixelShader:
		DrawPixelShader();
		break;
	default:
		BIZ_ASSERT_FAIL("Unsupported resource render type");
		break;
	}
}

void C_ResourceRender::DrawTexture()
{
#if TRACK_DISPLAY2_RESOURCES
	F32 x = g_debugDisplayResourceRenderX;
	F32 y = g_debugDisplayResourceRenderY;
	F32 s = g_debugDisplayResourceRenderSize;

	char str[1024];
	
	sprintf_s(str, sizeof(str), "Textures %i",
		Display2::Device()->GetTextureCount());
	FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
	y += s*2.2f;

	U32 i = 0;
	Display2::C_ResourceListIt it = Display2::Device()->GetTextureBegin();
	while(it != Display2::Device()->GetTextureEnd())
	{
		Display2::C_Texture* pTexture = (Display2::C_Texture*)it->GetKey();
		Display2::C_ResourceInfo* pResourceInfo = it->GetValue();

		sprintf_s(str, sizeof(str), "%i : %ix%ix%i : %i : %s : %i : %s : '%s'",
			i,
			pTexture->GetWidth(),
			pTexture->GetHeight(),
			pTexture->GetDepth(),
			pTexture->GetLevels(),
			Display2::GetTextureTypeAsString(pTexture->GetType()).Chars(),
			pTexture->GetSize(),
			Display2::GetTextureFormatAsString(pTexture->GetFormat()).Chars(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
		FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.1f;

		i++;
		it++;
	}
#endif
}

void C_ResourceRender::DrawSurface()
{
#if TRACK_DISPLAY2_RESOURCES
	F32 x = g_debugDisplayResourceRenderX;
	F32 y = g_debugDisplayResourceRenderY;
	F32 s = g_debugDisplayResourceRenderSize;
	
	char str[1024];
	
	sprintf_s(str, sizeof(str), "Surfaces %i",
		Display2::Device()->GetSurfaceCount());
	FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
	y += s*2.2f;

	U32 i = 0;
	Display2::C_ResourceListIt it = Display2::Device()->GetSurfaceBegin();
	while(it != Display2::Device()->GetSurfaceEnd())
	{
		Display2::C_Surface* pSurface = (Display2::C_Surface*)it->GetKey();
		Display2::C_ResourceInfo* pResourceInfo = it->GetValue();

		sprintf_s(str, sizeof(str), "%i : %ix%i : %i : %s : %s : '%s'",
			i,
			pSurface->GetWidth(),
			pSurface->GetHeight(),
			pSurface->GetSize(),
			Display2::GetSurfaceFormatAsString(pSurface->GetFormat()).Chars(),
			Display2::GetMultisampleAsString((Display2::E_Multisample)pSurface->GetMultisample()).Chars(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
		FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.1f;

		i++;
		it++;
	}
#endif
}

void C_ResourceRender::DrawVertexDeclaration()
{
}

void C_ResourceRender::DrawVertexBuffer()
{
#if TRACK_DISPLAY2_RESOURCES
	F32 x = g_debugDisplayResourceRenderX;
	F32 y = g_debugDisplayResourceRenderY;
	F32 s = g_debugDisplayResourceRenderSize;
	
	char str[1024];
	
	sprintf_s(str, sizeof(str), "Vertex Buffers %i",
		Display2::Device()->GetVertexBufferCount());
	FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
	y += s*2.2f;

	U32 i = 0;
	Display2::C_ResourceListIt it = Display2::Device()->GetVertexBufferBegin();
	while(it != Display2::Device()->GetVertexBufferEnd())
	{
		Display2::C_VertexBuffer* pVertexBuffer = (Display2::C_VertexBuffer*)it->GetKey();

		sprintf_s(str, sizeof(str), "%i : size %i flags %i address 0x%.8x",
			i,
			pVertexBuffer->GetSize(),
			pVertexBuffer->GetFlags(),
			pVertexBuffer->GetAddress());
		FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.1f;

		i++;
		it++;
	}
#endif
}

void C_ResourceRender::DrawIndexBuffer()
{
#if TRACK_DISPLAY2_RESOURCES
	F32 x = g_debugDisplayResourceRenderX;
	F32 y = g_debugDisplayResourceRenderY;
	F32 s = g_debugDisplayResourceRenderSize;
	
	char str[1024];
	
	sprintf_s(str, sizeof(str), "Index Buffers %i",
		Display2::Device()->GetIndexBufferCount());
	FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
	y += s*2.2f;

	U32 i = 0;
	Display2::C_ResourceListIt it = Display2::Device()->GetIndexBufferBegin();
	while(it != Display2::Device()->GetIndexBufferEnd())
	{
		Display2::C_IndexBuffer* pIndexBuffer = (Display2::C_IndexBuffer*)it->GetKey();

		sprintf_s(str, sizeof(str), "%i : size %i flags %i format %i address 0x%.8x",
			i,
			pIndexBuffer->GetSize(),
			pIndexBuffer->GetFlags(),
			pIndexBuffer->GetFormat(),
			pIndexBuffer->GetAddress());
		FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.1f;

		i++;
		it++;
	}
#endif
}

void C_ResourceRender::DrawVertexShader()
{
#if TRACK_DISPLAY2_RESOURCES
	F32 x = g_debugDisplayResourceRenderX;
	F32 y = g_debugDisplayResourceRenderY;
	F32 s = g_debugDisplayResourceRenderSize;
	
	char str[1024];
	
	sprintf_s(str, sizeof(str), "Vertex Shaders %i",
		Display2::Device()->GetVertexShaderCount());
	FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
	y += s*2.2f;

	U32 i = 0;
	Display2::C_ResourceListIt it = Display2::Device()->GetVertexShaderBegin();
	while(it != Display2::Device()->GetVertexShaderEnd())
	{
		Display2::C_VertexShader* pVertexShader = (Display2::C_VertexShader*)it->GetKey();
		Display2::C_ResourceInfo* pResourceInfo = it->GetValue();

		sprintf_s(str, sizeof(str), "%i : size %i flags %i address 0x%.8x path '%s'",
			i,
			pVertexShader->GetSize(),
			pVertexShader->GetFlags(),
			pVertexShader->GetAddress(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
		FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.1f;

		i++;
		it++;
	}
#endif
}

void C_ResourceRender::DrawPixelShader()
{
#if TRACK_DISPLAY2_RESOURCES
	F32 x = g_debugDisplayResourceRenderX;
	F32 y = g_debugDisplayResourceRenderY;
	F32 s = g_debugDisplayResourceRenderSize;
	
	char str[1024];
	
	sprintf_s(str, sizeof(str), "Pixel Shaders %i",
		Display2::Device()->GetPixelShaderCount());
	FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
	y += s*2.2f;

	U32 i = 0;
	Display2::C_ResourceListIt it = Display2::Device()->GetPixelShaderBegin();
	while(it != Display2::Device()->GetPixelShaderEnd())
	{
		Display2::C_PixelShader* pPixelShader = (Display2::C_PixelShader*)it->GetKey();
		Display2::C_ResourceInfo* pResourceInfo = it->GetValue();

		sprintf_s(str, sizeof(str), "%i : size %i flags %i address 0x%.8x path '%s'",
			i,
			pPixelShader->GetSize(),
			pPixelShader->GetFlags(),
			pPixelShader->GetAddress(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
		FontRender()->Draw(str, x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.1f;

		i++;
		it++;
	}
#endif
}

C_ResourceRender* g_pResourceRender = NULL;

void ResourceRender(C_ResourceRender* pResourceRender)
{
	g_pResourceRender = pResourceRender;
}

}; // namespace Display
