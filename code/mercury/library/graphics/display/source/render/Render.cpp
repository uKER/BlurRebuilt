#include "stdafx.h"
#include "Display/Render/Render.h"
#include "Display/Device/Device.h"
#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Shader/VertexShaders.h"
#include "Display/Shader/PixelShaders.h"


namespace Display
{
namespace Quad
{

Debug::Bool g_debugQuadHalfPixelOffset("Graphics.Display.Quad.HalfPixelOffset", true);
Debug::Float g_debugQuadHalfPixelX("Graphics.Display.Quad.X", 0.0f, Debug::Range(-.1f, .1f), Debug::Step(0.001f));
Debug::Float g_debugQuadHalfPixelY("Graphics.Display.Quad.Y", 0.0f, Debug::Range(-.1f, .1f), Debug::Step(0.001f));

E_TextureAddress m_AddressU		= E_TextureAddress_Clamp;
E_TextureAddress m_AddressV		= E_TextureAddress_Clamp;
E_TextureAddress m_AddressW		= E_TextureAddress_Clamp;

E_TextureFilter m_FilterMag		= E_TextureFilter_Linear;
E_TextureFilter m_FilterMin		= E_TextureFilter_Linear;
E_TextureFilter m_FilterMip		= E_TextureFilter_Linear;

MathsStorage::UInt2 m_Segments	= MathsStorage::UInt2(1, 1);
F32		m_HalfPixelScaleX		= 1.0f;
F32		m_HalfPixelScaleY		= 1.0f;
bool	m_FlipHorizontal		= false;
bool	m_FlipVertical			= false;
F32		m_BiasX					= 0.0f;
F32		m_BiasY					= 0.0f;

void SetQuadSettings(MathsStorage::UInt2 segments, F32 halfPixelScaleX, F32 halfPixelScaleY, bool flipHorizontal, bool flipVertical)
{
	assert(segments.x > 0 && segments.x < 128);
	assert(segments.y > 0 && segments.y < 128);

	m_Segments			= segments;
	m_HalfPixelScaleX	= halfPixelScaleX;
	m_HalfPixelScaleY	= halfPixelScaleY;
	m_FlipHorizontal	= flipHorizontal;
	m_FlipVertical		= flipVertical;
};

void SetQuadBias(F32 biasX, F32 biasY)
{
	m_BiasX	= biasX;
	m_BiasY	= biasY;
};

void TextureAddress(const E_TextureAddress addressU, const E_TextureAddress addressV, const E_TextureAddress addressW)
{
	m_AddressU = addressU;
	m_AddressV = addressV;
	m_AddressW = addressW;
}
	
void TextureFilter(const E_TextureFilter filterMag, const E_TextureFilter filterMin, const E_TextureFilter filterMip)
{
	m_FilterMag = filterMag;
	m_FilterMin = filterMin;
	m_FilterMip = filterMip;
}

void TextureFilter(const E_TextureFilter filter)
{
	m_FilterMag = filter;
	m_FilterMin = filter;
	m_FilterMip = filter;
}

void DrawVertexColour(const Maths::RGBA colour)
{
	F32 x1 = -1.f;
	F32 y1 =  1.0f;
	F32 x2 =  1.0f;
	F32 y2 = -1.0f;

	Device()->SetVertexDeclaration(Vertex_XY_C_UV::m_pDeclaration);
	Device()->SetVertexShader(&VertexShader_XY::m_VertexShaderVertexColour);
	Device()->SetPixelShader(&PixelShader::m_PixelShaderVertexColour);
	
	Vertex_XY_C_UV* pVertex = (Vertex_XY_C_UV*)Device()->BeginVertices(E_PrimitiveType_TriangleList, 6, sizeof(Vertex_XY_C_UV), Vertex_XY_C_UV::m_pDeclaration, false); // This must not fail

	if(pVertex)
	{
		// tri 1
		pVertex->x = x1;
		pVertex->y = y1;
		pVertex->c = GPU_U8_4_TO_U32(colour.R(), colour.G(), colour.B(), colour.A());
		pVertex->u = 0.0f;
		pVertex->v = 0.0f;
		pVertex++;

		pVertex->x = x2;
		pVertex->y = y1;
		pVertex->c = GPU_U8_4_TO_U32(colour.R(), colour.G(), colour.B(), colour.A());
		pVertex->u = 0.0f;
		pVertex->v = 0.0f;
		pVertex++;
		
		pVertex->x = x1;
		pVertex->y = y2;
		pVertex->c = GPU_U8_4_TO_U32(colour.R(), colour.G(), colour.B(), colour.A());
		pVertex->u = 0.0f;
		pVertex->v = 0.0f;
		pVertex++;

		// tri 2
		pVertex->x = x1;
		pVertex->y = y2;
		pVertex->c = GPU_U8_4_TO_U32(colour.R(), colour.G(), colour.B(), colour.A());
		pVertex->u = 0.0f;
		pVertex->v = 0.0f;
		pVertex++;

		pVertex->x = x2;
		pVertex->y = y1;
		pVertex->c = GPU_U8_4_TO_U32(colour.R(), colour.G(), colour.B(), colour.A());
		pVertex->u = 0.0f;
		pVertex->v = 0.0f;
		pVertex++;

		pVertex->x = x2;
		pVertex->y = y2;
		pVertex->c = GPU_U8_4_TO_U32(colour.R(), colour.G(), colour.B(), colour.A());
		pVertex->u = 0.0f;
		pVertex->v = 0.0f;
		pVertex++;

		Device()->EndVertices(pVertex);
	}
	else
	{
		LOGERROR(g_Log, "Too many begin vertices have been issued during the frame.\n");
	}
}

void Draw()
{
	F32 u, v, du, dv;
	F32 x0 = -1.f;
	F32 y0 = 1.0f;
	F32 dx = 2.0f;
	F32 dy = -2.0f;

	if(m_FlipHorizontal)
	{
		u = 1.0f;
		du = -1.0f;
	}
	else
	{
		u = 0.0f;
		du = 1.0f;
	}

	if(m_FlipVertical)
	{
		v = 1.0f;
		dv = -1.0f;
	}
	else
	{
		v = 0.0f;
		dv = 1.0f;
	}

	// always use this vertex shader
	Device()->SetVertexShader(&VertexShader_XY::m_VertexShader);
	
	const C_Viewport& viewport = Device()->GetViewport();
	x0 -= m_BiasX / (F32)viewport.GetW();
	y0 += m_BiasY / (F32)viewport.GetH();
	dx += m_BiasX / (F32)viewport.GetW();
	dy -= m_BiasY / (F32)viewport.GetW();

	// [Blur1PCDX11] D3D11 puts pixel centres at half-integers, as PS3 does: no D3D9 half-pixel shift.
	
	Vertex_XY_UV* pVertex = (Vertex_XY_UV*)Device()->BeginVertices(E_PrimitiveType_TriangleList, m_Segments.x * m_Segments.y * 6, sizeof(Vertex_XY_UV), Vertex_XY_UV::m_pDeclaration, false); // This must not fail

	if(pVertex)
	{
		dx = dx / (F32)m_Segments.x;
		dy = dy / (F32)m_Segments.y;

		F32 x1 = x0;
		F32 y1 = y0;

		for(U32 i = 0; i < m_Segments.x; ++i)
		{
			y1 = y0;

			for(U32 j = 0; j < m_Segments.y; ++j)
			{
				// tri 1
				pVertex->x = x1;
				pVertex->y = y1;
				pVertex->u = u;
				pVertex->v = v;
				pVertex++;

				pVertex->x = x1 + dx;
				pVertex->y = y1;
				pVertex->u = u + du;
				pVertex->v = v;
				pVertex++;
				
				pVertex->x = x1;
				pVertex->y = y1 + dy;
				pVertex->u = u;
				pVertex->v = v + dv;
				pVertex++;

				// tri 2
				pVertex->x = x1;
				pVertex->y = y1 + dy;
				pVertex->u = u;
				pVertex->v = v + dv;
				pVertex++;

				pVertex->x = x1 + dx;
				pVertex->y = y1;
				pVertex->u = u + du;
				pVertex->v = v;
				pVertex++;

				pVertex->x = x1 + dx;
				pVertex->y = y1 + dy;
				pVertex->u = u + du;
				pVertex->v = v + dv;
				pVertex++;

				y1 += dy;
			}
			x1 += dx;
		}

		Device()->EndVertices(pVertex);
	}
	else
	{
		LOGERROR(g_Log, "Too many begin vertices have been issued during the frame.\n");
	}

}

void DrawFullScreen()
{
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	Display::Device()->SetCullMode(Display::E_CullMode_None);

	Draw();

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
}

void DrawFullScreen(const C_Texture* pTexture)
{
	Device()->SetPixelShader(&PixelShader::m_PixelShaderTexture);

	if(pTexture)
	{
		Device()->SetTexture(0, pTexture);
		Device()->SetTextureFilterMagMinMip(0, m_FilterMag, m_FilterMin, m_FilterMip);
		Device()->SetTextureAddressUVW(0, m_AddressU, m_AddressV, m_AddressW);
	}

	DrawFullScreen();
}

void DrawFullScreen(const C_Texture* pTexture, const C_PixelShader* pPixelShader)
{
	if(pPixelShader)
	{
		Device()->SetPixelShader(pPixelShader);
	}

	if(pTexture)
	{
		Device()->SetTexture(0, pTexture);
		Device()->SetTextureFilterMagMinMip(0, m_FilterMag, m_FilterMin, m_FilterMip);
		Device()->SetTextureAddressUVW(0, m_AddressU, m_AddressV, m_AddressW);
	}

	DrawFullScreen();
}

void DrawFullScreenDepth()
{
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthFunc(Display::E_CmpFunc_Always);
	#if defined(PLATFORM_PS3)
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	#else
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	#endif
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);

	Draw();

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
}

void DrawFullScreenDepth(const C_Texture* pColour, const C_Texture* pDepth, const C_PixelShader* pPixelShader)
{
	Display::Device()->SetPixelShader(pPixelShader);

	Display::Device()->SetTexture(0, pColour);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	
	Display::Device()->SetTexture(1, pDepth);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);

	DrawFullScreenDepth();
}

void DrawFullScreenDepth(const C_Texture* pColour, const C_Texture* pDepth)
{
	Display::Device()->SetPixelShader(&Display::PixelShader::m_PixelShaderTexture);

	Display::Device()->SetTexture(0, pColour);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	
	Display::Device()->SetTexture(1, pDepth);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);

	DrawFullScreenDepth();
}

void DrawFullScreenColour(const Maths::C_RGBA_F32& colour)
{
	F32 constant[12] = {
		colour.R(), colour.G(), colour.B(), colour.A(),
		colour.G(), colour.R(), colour.B(), colour.A(),
		colour.G(), colour.B(), colour.R(), colour.A() };
	
	Display::Device()->SetPixelShaderParameterF(PixelShader::m_PixelShaderColour_Colour, (F32*)&constant);

	Device()->SetPixelShader(&PixelShader::m_PixelShaderColour);

	DrawFullScreen();
}

#ifdef PLATFORM_PS3
void DrawFullscreenQuad(const F32 du, const F32 dv)
{
	Device()->SetVertexShader(&VertexShader_XY::m_VertexShader);

	Vertex_XY_UV* pVertex = (Vertex_XY_UV*)Device()->BeginVertices(
		E_PrimitiveType_QuadList,
		4,
		sizeof(Vertex_XY_UV),
		Vertex_XY_UV::m_pDeclaration, false); // This must not fail

	if(pVertex)
	{
		pVertex->x = -1.0f;
		pVertex->y =  1.0f;
		pVertex->u =  0.0f + du;
		pVertex->v =  0.0f + dv;
		pVertex++;

        pVertex->x = -1.0f;
		pVertex->y = -1.0f;
		pVertex->u =  0.0f + du;
		pVertex->v =  1.0f + dv;
		pVertex++;

        pVertex->x =  1.0f;
		pVertex->y = -1.0f;
		pVertex->u =  1.0f + du;
		pVertex->v =  1.0f + dv;
		pVertex++;

        pVertex->x =  1.0f;
		pVertex->y =  1.0f;
		pVertex->u =  1.0f + du;
		pVertex->v =  0.0f + dv;
		pVertex++;

		Device()->EndVertices(pVertex);
	}
}
#endif

void DrawPartScreen(F32 x1, F32 y1, F32 x2, F32 y2)
{
	Device()->SetVertexShader(&VertexShader_XY::m_VertexShader);
		
	// [Blur1PCDX11] D3D11 puts pixel centres at half-integers, as PS3 does: no D3D9 half-pixel shift.
	
	Vertex_XY_UV* pVertex = (Vertex_XY_UV*)Device()->BeginVertices(E_PrimitiveType_TriangleList, 6, sizeof(Vertex_XY_UV), Vertex_XY_UV::m_pDeclaration, false); // This must not fail

	if(pVertex)
	{
		// tri 1
		pVertex->x = x1;
		pVertex->y = y1;
		pVertex->u = 0.0f;
		pVertex->v = 1.0f;
		pVertex++;

		pVertex->x = x1;
		pVertex->y = y2;
		pVertex->u = 0.0f;
		pVertex->v = 0.0f;
		pVertex++;
		
		pVertex->x = x2;
		pVertex->y = y2;
		pVertex->u = 1.0f;
		pVertex->v = 0.0f;
		pVertex++;

		// tri 2
		pVertex->x = x1;
		pVertex->y = y1;
		pVertex->u = 0.0f;
		pVertex->v = 1.0f;
		pVertex++;

		pVertex->x = x2;
		pVertex->y = y2;
		pVertex->u = 1.0f;
		pVertex->v = 0.0f;
		pVertex++;

		pVertex->x = x2;
		pVertex->y = y1;
		pVertex->u = 1.0f;
		pVertex->v = 1.0f;
		pVertex++;

		Device()->EndVertices(pVertex);
	}
	else
	{
		LOGERROR(g_Log, "Too many begin vertices have been issued during the frame.\n");
	}
}

void DrawPartScreen(F32 x1, F32 y1, F32 x2, F32 y2, const C_Texture* pTexture)
{
	Device()->SetPixelShader(&PixelShader::m_PixelShaderTexture);

	if(pTexture)
	{
		Device()->SetTexture(0, pTexture);
		Device()->SetTextureFilterMagMinMip(0, m_FilterMag, m_FilterMin, m_FilterMip);
		Device()->SetTextureAddressUVW(0, m_AddressU, m_AddressV, m_AddressW);
	}

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	DrawPartScreen(x1, y1, x2, y2);

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
}

void DrawPartScreen(F32 x1, F32 y1, F32 x2, F32 y2, const C_Texture* pTexture, const C_PixelShader* pPixelShader)
{
	if(pPixelShader)
	{
		Device()->SetPixelShader(pPixelShader);
	}

	if(pTexture)
	{
		Device()->SetTexture(0, pTexture);
		Device()->SetTextureFilterMagMinMip(0, m_FilterMag, m_FilterMin, m_FilterMip);
		Device()->SetTextureAddressUVW(0, m_AddressU, m_AddressV, m_AddressW);
	}

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	DrawPartScreen(x1, y1, x2, y2);

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
}

} // namespace Quad
} // namespace Display
