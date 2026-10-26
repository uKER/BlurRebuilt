#include "stdafx.h"
#include "Display/Render/FontRender.h"
#include "Display/Texture/TextureUtils.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Display/Shaders/Render/PS3/vsFontRender.shader"
#include "Display/Shaders/Render/PS3/psFontRender.shader"
#endif
#endif

EXTERN_SHADER(vsFontRender);
EXTERN_SHADER(psFontRender);

Debug::Float g_debugDisplayFontRender3DFontWinSize("Graphics.Display.FontRender.3DFontWinSize",	100.0f,		Debug::Range(0.0f, 200.0f));
Debug::Float g_debugDisplayFontRender3DFontMinSize("Graphics.Display.FontRender.3DFontMinSize",	0.0f,		Debug::Range(0.0f, 1024.0f));
Debug::Float g_debugDisplayFontRender3DFontMaxSize("Graphics.Display.FontRender.3DFontMaxSize",	1024.0f,	Debug::Range(0.0f, 1024.0f));

Debug::Float g_debugDisplayFontRenderU1("Graphics.Display.FontRender.U1",	0.0f,	Debug::Range(0.0f, 1.0f));
Debug::Float g_debugDisplayFontRenderV1("Graphics.Display.FontRender.V1",	0.0f,	Debug::Range(0.0f, 1.0f));
Debug::Float g_debugDisplayFontRenderW1("Graphics.Display.FontRender.W1",	1.0f,	Debug::Range(0.0f, 2.0f));

namespace Display
{

#include "Display/Render/FontRenderOffsets.h"

C_FontRender::C_FontRender() : I_Renderer("FontRender"), m_pVertexDeclaration(NULL), m_Created(false)
{
	assert(g_pFontRender == NULL);
	SetAssetPath("Textures/");

	g_pFontRender = this;
}

C_FontRender::~C_FontRender()
{
	DestroyRenderer();

	assert(g_pFontRender == this);
	g_pFontRender = NULL;
}

bool C_FontRender::OnCreate()
{
	assert(!m_Created);

	RESOURCE_MEMORY_ID("FontRender");

	if(!m_pVertexDeclaration)
	{
		C_VertexFormat vertexFormat;
		vertexFormat.AddElement(0, E_VertexElementType_F32_2, E_VertexElementUsage_Position, 0);
		vertexFormat.AddElement(0, E_VertexElementType_Colour, E_VertexElementUsage_Colour, 0);
		vertexFormat.AddElement(0, E_VertexElementType_F32_2, E_VertexElementUsage_TexCoord, 0);
		m_pVertexDeclaration = Device()->CreateVertexDeclaration(vertexFormat);
	}

#if defined(DEVICE_PCD3D11) || defined(DEVICE_PS3GCM) // [Blur1PCDX11] embedded built-ins
	if(!CreateVertexShaderFromFunc("Shaders/vsFontRender", 0, &m_VertexShader, REF_SHADER(vsFontRender)))
		return false;
	if(!CreatePixelShaderFromFunc("Shaders/psFontRender", 0, &m_PixelShader, REF_SHADER(psFontRender)))
		return false;
#else
	if(!CreateVertexShaderFromFile("Shaders/vsFontRender", 0, &m_VertexShader))
		return false;
	if(!CreatePixelShaderFromFile("Shaders/psFontRender", 0, &m_PixelShader))
		return false;
#endif


	#ifdef SHADERS_CG_VS
	m_cgClipVector = Device()->GetShaderParameter(m_VertexShader.m_cgProgram, "g_ClipVector");
	#endif

	m_Created = true;
	
	return true;
}

void C_FontRender::OnDestroy()
{
	Device()->Destroy(&m_VertexShader);
	Device()->Destroy(&m_PixelShader);
	Device()->Destroy(&m_Texture);

	m_Created = false;
}

void C_FontRender::Draw(const char* pChars, F32 x, F32 y, F32 w, F32 h, const Maths::RGBA col, const E_FontRenderAlign align, const bool leading)
{
	if(!m_Created)
		return;

	PERF_EVENT("RenderFont");

	if(!m_Texture.IsCreated())
	{
		if (Sys::GetDefaultFileSystem()->FileExists(GetAssetPath() + "Font.dds") &&
			!CreateTexture2DFromFile(GetAssetPath() + "Font.dds", &m_Texture))
			return;

		if (Sys::GetDefaultFileSystem()->FileExists(Util::C_String("DontShip/") + GetAssetPath() + "Font.dds") &&
			!CreateTexture2DFromFile(Util::C_String("DontShip/") + GetAssetPath() + "Font.dds", &m_Texture))
			return;
	}

	const U32 charCount = strlen(pChars);
	if(charCount == 0)
		return;

	y -= h * 0.5f;

	#ifdef SHADERS_CG_VS
	Device()->SetVertexShaderParameter(m_cgClipVector, (F32*)&Device()->m_ClipVector);
	#endif

	Device()->SetDepthEnable(E_Bool_False);
	Device()->SetDepthWriteEnable(E_Bool_False);
	Device()->SetBlendMode(E_BlendMode_Alpha);
	Device()->SetVertexShader(&m_VertexShader);
	Device()->SetPixelShader(&m_PixelShader);
	Device()->SetTexture(0, &m_Texture);
	Device()->SetTextureFilterMagMinMip(0, E_TextureFilter_Linear, E_TextureFilter_Linear, E_TextureFilter_Linear);
	Device()->SetTextureAddressUVW(0, E_TextureAddress_Clamp, E_TextureAddress_Clamp, E_TextureAddress_Clamp);

	C_FontVertex* pVertex = (C_FontVertex*)Device()->BeginVertices(E_PrimitiveType_TriangleList, charCount * 6, sizeof(C_FontVertex), m_pVertexDeclaration, true); // This can fail

	if(pVertex)
	{
		const U32 gpuCol = GPU_RGBA(col.R(), col.G(), col.B(), col.A());

		switch(align)
		{
		case E_FontRenderAlign_Ct:
			{
				F32 w1 = 0.f;
				for(U32 charIndex=0; charIndex<charCount; charIndex++)
				{
					const S32 charID = (S32)pChars[charIndex] - 32;

					const F32 u = F32(charID % 16) / 16.f;

					const F32 u1 = g_FontOffsets[charID].x;
					const F32 v1 = g_FontOffsets[charID].y;

					w1 += w * (1.f - ((u1 + v1) / 0.0625f));
				}
				x -= (w1 / 2.f);
			}
			break;
		case E_FontRenderAlign_Rt:
			{
				for(U32 charIndex=0; charIndex<charCount; charIndex++)
				{
					const S32 charID = (S32)pChars[charIndex] - 32;

					const F32 u = F32(charID % 16) / 16.f;
					
					const F32 u1 = g_FontOffsets[charID].x;
					const F32 v1 = g_FontOffsets[charID].y;

					const F32 w1 = w * (1.f - ((u1 + v1) / 0.0625f));

					x -= w1;
				}
			}
			break;
		}

		for(U32 charIndex=0; charIndex<charCount; charIndex++)
		{  
			S32 charID = (S32)pChars[charIndex] - 32;
			if(charID < 0 || charID >= 256) charID = 0;
			
			const F32 u = F32(charID % 16) / 16.f;
			const F32 v = F32(charID / 16) / 16.f;

			const F32 u1 = leading ? (F32)g_debugDisplayFontRenderU1 : g_FontOffsets[charID].x;
			const F32 v1 = leading ? (F32)g_debugDisplayFontRenderV1 : g_FontOffsets[charID].y;

			const F32 w1 = leading ? w * (F32)g_debugDisplayFontRenderW1 : w * (1.f - ((u1 + v1) / 0.0625f));

			pVertex->x = x;
			pVertex->y = y;
			pVertex->c = gpuCol;
			pVertex->u = u + u1;
			pVertex->v = v;
			pVertex++;

			pVertex->x = x + w1;
			pVertex->y = y;
			pVertex->c = gpuCol;
			pVertex->u = u + 0.0625f - v1;
			pVertex->v = v;
			pVertex++;

			pVertex->x = x + w1;
			pVertex->y = y + h;
			pVertex->c = gpuCol;
			pVertex->u = u + 0.0625f - v1;
			pVertex->v = v + 0.0625f;
			pVertex++;

			pVertex->x = x + w1;
			pVertex->y = y + h;
			pVertex->c = gpuCol;
			pVertex->u = u + 0.0625f - v1;
			pVertex->v = v + 0.0625f;
			pVertex++;

			pVertex->x = x;
			pVertex->y = y + h;
			pVertex->c = gpuCol;
			pVertex->u = u + u1;
			pVertex->v = v + 0.0625f;
			pVertex++;

			pVertex->x = x;
			pVertex->y = y;
			pVertex->c = gpuCol;
			pVertex->u = u + u1;
			pVertex->v = v;
			pVertex++;

			x += w1;
		}

		Device()->EndVertices(pVertex);
	}

	Device()->SetDepthEnable(E_Bool_True);
	Device()->SetDepthWriteEnable(E_Bool_True);
}

void C_FontRender::Draw2D(const char* pChars, const Maths::HVec2& pos, const Maths::HVec2& dim, const Maths::RGBA col, const E_FontRenderAlign align)
{
	Draw(pChars, pos.X(), pos.Y(), dim.X(), dim.Y(), col, align);
}

void C_FontRender::Draw3D(const char* pChars, const Maths::HVec3& pos, const F32 size, const Maths::RGBA col, const E_FontRenderAlign align)
{
	const C_ViewFrustum& viewFrustum = Device()->GetViewFrustum();
	const Maths::C_Sphere sphere(pos, size);
	if(viewFrustum.CullSphere(sphere))
	{
		const C_ViewSettings3D& viewSettings3D = Device()->GetViewSettings3D();
		const Maths::HVec3 windowPos = viewSettings3D.TransformWorldToWindow(pos);
		if(windowPos.Z() > 0.0f)
		{
			const C_ViewSettings2D& viewSettings2D = Device()->GetViewSettings2D();
			const Maths::C_Rect frame = viewSettings2D.m_ViewExtents;
			
			if(windowPos.X() < frame.Min().X())
				return;
			if(windowPos.Y() < frame.Min().Y())
				return;
			if(windowPos.X() > frame.Max().X())
				return;
			if(windowPos.Y() > frame.Max().Y())
				return;

			F32 windowSize = viewSettings3D.GetScreenSize(sphere);
			if(windowSize < g_debugDisplayFontRender3DFontMinSize)
				windowSize = g_debugDisplayFontRender3DFontMinSize;
			if(windowSize > g_debugDisplayFontRender3DFontMaxSize)
				windowSize = g_debugDisplayFontRender3DFontMaxSize;
			F32 windowScale = windowSize/g_debugDisplayFontRender3DFontMaxSize;
			F32 scale = windowScale*g_debugDisplayFontRender3DFontWinSize;

			Draw(pChars, windowPos.X(), windowPos.Y(), scale*size, scale*size, col, align);
		}
	}
}

F32 C_FontRender::GetSpacing(const char* pChars, const F32 w) const
{
	F32 spacing = 0.f;
	const U32 charCount = strlen(pChars);
	for(U32 charIndex=0; charIndex<charCount; charIndex++)
	{
		const S32 charID = (S32)pChars[charIndex] - 32;

		const F32 u = F32(charID % 16) / 16.f;
		
		const F32 u1 = g_FontOffsets[charID].x;
		const F32 v1 = g_FontOffsets[charID].y;

		const F32 w1 = w * (1.f - ((u1 + v1) / 0.0625f));
		
		spacing += w1;
	}
	return spacing;
}

bool C_FontRender::Transform3Dto2D(const Maths::HVec3& worldPos, const F32 worldSize, Maths::HVec2& windowPos, Maths::HVec2& windowDim)
{
	const C_ViewFrustum& viewFrustum = Device()->GetViewFrustum();
	const Maths::C_Sphere sphere(worldPos, worldSize);
	if(viewFrustum.CullSphere(sphere))
	{
		const C_ViewSettings3D& viewSettings3D = Device()->GetViewSettings3D();
		const Maths::HVec3 tempPos = viewSettings3D.TransformWorldToWindow(worldPos);
		if(tempPos.Z() > 0.0f)
		{
			const C_ViewSettings2D& viewSettings2D = Device()->GetViewSettings2D();
			const Maths::C_Rect frame = viewSettings2D.m_ViewExtents;
			
			if(tempPos.X() < frame.Min().X())
				return false;
			if(tempPos.Y() < frame.Min().Y())
				return false;
			if(tempPos.X() > frame.Max().X())
				return false;
			if(tempPos.Y() > frame.Max().Y())
				return false;

			F32 windowSize = viewSettings3D.GetScreenSize(sphere);
			if(windowSize < g_debugDisplayFontRender3DFontMinSize)
				windowSize = g_debugDisplayFontRender3DFontMinSize;
			if(windowSize > g_debugDisplayFontRender3DFontMaxSize)
				windowSize = g_debugDisplayFontRender3DFontMaxSize;
			F32 windowScale = windowSize/g_debugDisplayFontRender3DFontMaxSize;
			F32 scale = windowScale*g_debugDisplayFontRender3DFontWinSize;

			windowPos = Maths::HVec2(tempPos.X(), tempPos.Y());
			windowDim = Maths::HVec2(scale*worldSize, scale*worldSize);

			return true;
		}
	}

	return false;
}

C_FontRender* g_pFontRender = NULL;

void FontRender(C_FontRender* pFontRender)
{
	g_pFontRender = pFontRender;
}

}; // namespace Display
