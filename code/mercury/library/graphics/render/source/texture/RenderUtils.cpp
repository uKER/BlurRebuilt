#include "stdafx.h"
#include "Render/Texture/RenderUtils.h"
#include "Render/Texture/RenderUtils.inl"

#include "Display/Buffer/VertexDeclarations.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"

EXTERN_SHADER(psTexture2DColour);
EXTERN_SHADER(psTexture3DColour);
EXTERN_SHADER(psTextureCubeColour);
EXTERN_SHADER(psTexture2DDepth);

namespace Render
{
namespace Texture
{

Debug::Float							g_debugRenderUtilsSlice("Graphics.Render.Texture.RenderUtils.Slice", 0.0f, Debug::Range(0.0f, 1.0f));
Debug::Enum<Display::E_CubeTextureFace>	g_debugRenderUtilsFace("Graphics.Render.Texture.RenderUtils.Face", Display::E_CubeTextureFace_PosX);
Debug::Bool								g_debugRenderUtilsAllFaces("Graphics.Render.Texture.RenderUtils.RenderAllCubeMapFaces", false);

void C_RenderUtilsShaders::Create()
{
	RESOURCE_MEMORY_ID("TextureRenderUtils");

	Display::CreatePixelShaderFromFunc("Shaders/psTexture2DColour", 0, &m_Colour2DPixelShader, REF_SHADER(psTexture2DColour));
	Display::CreatePixelShaderFromFunc("Shaders/psTexture3DColour", 0, &m_Colour3DPixelShader, REF_SHADER(psTexture3DColour));
	Display::CreatePixelShaderFromFunc("Shaders/psTextureCubeColour", 0, &m_ColourCubePixelShader, REF_SHADER(psTextureCubeColour));
	Display::CreatePixelShaderFromFunc("Shaders/psTexture2DDepth", 0, &m_Depth2DPixelShader, REF_SHADER(psTexture2DDepth));
	
	m_Colour2DParameterRed				= m_Colour2DPixelShader.GetParameter(CP_RENDER_SETTINGS, "g_vRed");
	m_Colour2DParameterGreen			= m_Colour2DPixelShader.GetParameter(CP_RENDER_SETTINGS_1, "g_vGreen");
	m_Colour2DParameterBlue				= m_Colour2DPixelShader.GetParameter(CP_RENDER_SETTINGS_2, "g_vBlue");
	m_Colour2DParameterAlpha			= m_Colour2DPixelShader.GetParameter(CP_RENDER_SETTINGS_3, "g_vAlpha");
	m_Colour2DParameterBias				= m_Colour2DPixelShader.GetParameter(CP_RENDER_SETTINGS_4, "g_vBias");

	m_Colour3DParameterRed				= m_Colour3DPixelShader.GetParameter(CP_RENDER_SETTINGS, "g_vRed");
	m_Colour3DParameterGreen			= m_Colour3DPixelShader.GetParameter(CP_RENDER_SETTINGS_1, "g_vGreen");
	m_Colour3DParameterBlue				= m_Colour3DPixelShader.GetParameter(CP_RENDER_SETTINGS_2, "g_vBlue");
	m_Colour3DParameterAlpha			= m_Colour3DPixelShader.GetParameter(CP_RENDER_SETTINGS_3, "g_vAlpha");
	m_Colour3DParameterBias				= m_Colour3DPixelShader.GetParameter(CP_RENDER_SETTINGS_4, "g_vBias");
	m_Colour3DParameterSlice			= m_Colour3DPixelShader.GetParameter(CP_RENDER_SETTINGS_5, "g_vSlice");

	m_ColourCubeParameterRed			= m_ColourCubePixelShader.GetParameter(CP_RENDER_SETTINGS, "g_vRed");
	m_ColourCubeParameterGreen			= m_ColourCubePixelShader.GetParameter(CP_RENDER_SETTINGS_1, "g_vGreen");
	m_ColourCubeParameterBlue			= m_ColourCubePixelShader.GetParameter(CP_RENDER_SETTINGS_2, "g_vBlue");
	m_ColourCubeParameterAlpha			= m_ColourCubePixelShader.GetParameter(CP_RENDER_SETTINGS_3, "g_vAlpha");
	m_ColourCubeParameterBias			= m_ColourCubePixelShader.GetParameter(CP_RENDER_SETTINGS_4, "g_vBias");
	m_ColourCubeParameterTopLeft		= m_ColourCubePixelShader.GetParameter(CP_RENDER_SETTINGS_5, "g_vTopLeft");
	m_ColourCubeParameterTopRight		= m_ColourCubePixelShader.GetParameter(CP_RENDER_SETTINGS_6, "g_vTopRight");
	m_ColourCubeParameterBottomLeft		= m_ColourCubePixelShader.GetParameter(CP_RENDER_SETTINGS_7, "g_vBottomLeft");
	m_ColourCubeParameterBottomRight	= m_ColourCubePixelShader.GetParameter(CP_RENDER_SETTINGS_8, "g_vBottomRight");

	m_Depth2DParameterScale				= m_Depth2DPixelShader.GetParameter(CP_RENDER_SETTINGS, "g_vScale");
}

void C_RenderUtilsShaders::Destroy()
{
	RESOURCE_MEMORY_ID("TextureRenderUtils");

	Display::Device()->Destroy(&m_Colour2DPixelShader);
	Display::Device()->Destroy(&m_Colour3DPixelShader);
	Display::Device()->Destroy(&m_ColourCubePixelShader);
	Display::Device()->Destroy(&m_Depth2DPixelShader);
}

S32						C_RenderUtils::m_RefCount = 0;
C_RenderUtilsShaders	C_RenderUtils::m_Shaders;

C_RenderUtils::C_RenderUtils()
{
	++m_RefCount;
	if (m_RefCount == 1)
	{
		m_Shaders.Create();
	}
}

C_RenderUtils::~C_RenderUtils()
{
	--m_RefCount;
	if(m_RefCount == 0)
	{
		m_Shaders.Destroy();
	}
}

void C_RenderUtils::DisplayTexture2DDepth(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture2D* pTexture2D)
{
	DisplayTexture2DDepth(x, y, w, h, pTexture2D, Render::C_RenderState::Instance().GetDepthScaleShaderParameter());
}

void C_RenderUtils::DisplayTexture2DColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture2D* pTexture2D, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias)
{
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour2DParameterRed, (F32*)&vRed);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour2DParameterGreen, (F32*)&vGreen);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour2DParameterBlue, (F32*)&vBlue);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour2DParameterAlpha, (F32*)&vAlpha);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour2DParameterBias, (F32*)&vBias);
	DrawTextured2DRect(x, y, w, h, pTexture2D, &m_Shaders.m_Colour2DPixelShader);
}

void C_RenderUtils::DisplayTexture3DColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture3D* pTexture3D, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias)
{
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour3DParameterRed, (F32*)&vRed);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour3DParameterGreen, (F32*)&vGreen);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour3DParameterBlue, (F32*)&vBlue);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour3DParameterAlpha, (F32*)&vAlpha);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour3DParameterBias, (F32*)&vBias);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Colour3DParameterSlice, Display::C_PSConstant(g_debugRenderUtilsSlice));
	DrawTextured2DRect(x, y, w, h, pTexture3D, &m_Shaders.m_Colour3DPixelShader);
}

void C_RenderUtils::DisplayTextureCubeColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_TextureCube* pTextureCube, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias)
{
	if (g_debugRenderUtilsAllFaces)
	{
		DisplayTextureCubeAllColour(x, y, w, h, pTextureCube, vRed, vGreen, vBlue, vAlpha, vBias);
	}
	else
	{
		DisplayTextureCubeFaceColour(x, y, w, h, pTextureCube, vRed, vGreen, vBlue, vAlpha, vBias);
	}
}

void C_RenderUtils::DisplayTextureCubeAllColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_TextureCube* pTextureCube, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias)
{
	// positive z
	{
		MathsStorage::Float4 vTopLeft(-1.0f, +1.0f, +1.0f, 0.0f);
		MathsStorage::Float4 vTopRight(+1.0f, +1.0f, +1.0f, 0.0f);
		MathsStorage::Float4 vBottomLeft(-1.0f, -1.0f, +1.0f, 0.0f);
		MathsStorage::Float4 vBottomRight(+1.0f, -1.0f, +1.0f, 0.0f);
		DisplayTextureCubeColour(x + w, y, w, h, pTextureCube, vTopLeft, vTopRight, vBottomLeft, vBottomRight, vRed, vGreen, vBlue, vAlpha, vBias);
	}
	// negative y
	{
		MathsStorage::Float4 vTopLeft(-1.0f, -1.0f, +1.0f, 0.0f);
		MathsStorage::Float4 vTopRight(+1.0f, -1.0f, +1.0f, 0.0f);
		MathsStorage::Float4 vBottomLeft(-1.0f, -1.0f, -1.0f, 0.0f);
		MathsStorage::Float4 vBottomRight(+1.0f, -1.0f, -1.0f, 0.0f);
		DisplayTextureCubeColour(x + w, y + h, w, h, pTextureCube, vTopLeft, vTopRight, vBottomLeft, vBottomRight, vRed, vGreen, vBlue, vAlpha, vBias);
	}
	// negative z
	{
		MathsStorage::Float4 vTopLeft(-1.0f, -1.0f, -1.0f, 0.0f);
		MathsStorage::Float4 vTopRight(+1.0f, -1.0f, -1.0f, 0.0f);
		MathsStorage::Float4 vBottomLeft(-1.0f, +1.0f, -1.0f, 0.0f);
		MathsStorage::Float4 vBottomRight(+1.0f, +1.0f, -1.0f, 0.0f);
		DisplayTextureCubeColour(x + w, y + 2*h, w, h, pTextureCube, vTopLeft, vTopRight, vBottomLeft, vBottomRight, vRed, vGreen, vBlue, vAlpha, vBias);
	}
	// positive y
	{
		MathsStorage::Float4 vTopLeft(-1.0f, +1.0f, -1.0f, 0.0f);
		MathsStorage::Float4 vTopRight(+1.0f, +1.0f, -1.0f, 0.0f);
		MathsStorage::Float4 vBottomLeft(-1.0f, +1.0f, +1.0f, 0.0f);
		MathsStorage::Float4 vBottomRight(+1.0f, +1.0f, +1.0f, 0.0f);
		DisplayTextureCubeColour(x + w, y + 3*h, w, h, pTextureCube, vTopLeft, vTopRight, vBottomLeft, vBottomRight, vRed, vGreen, vBlue, vAlpha, vBias);
	}
	// negative x
	{
		MathsStorage::Float4 vTopLeft(-1.0f, +1.0f, +1.0f, 0.0f);
		MathsStorage::Float4 vTopRight(-1.0f, -1.0f, +1.0f, 0.0f);
		MathsStorage::Float4 vBottomLeft(-1.0f, +1.0f, -1.0f, 0.0f);
		MathsStorage::Float4 vBottomRight(-1.0f, -1.0f, -1.0f, 0.0f);
		DisplayTextureCubeColour(x, y + h, w, h, pTextureCube, vTopLeft, vTopRight, vBottomLeft, vBottomRight, vRed, vGreen, vBlue, vAlpha, vBias);
	}
	// positive x
	{
		MathsStorage::Float4 vTopLeft(+1.0f, -1.0f, +1.0f, 0.0f);
		MathsStorage::Float4 vTopRight(+1.0f, +1.0f, +1.0f, 0.0f);
		MathsStorage::Float4 vBottomLeft(+1.0f, -1.0f, -1.0f, 0.0f);
		MathsStorage::Float4 vBottomRight(+1.0f, +1.0f, -1.0f, 0.0f);
		DisplayTextureCubeColour(x + 2*h, y + h, w, h, pTextureCube, vTopLeft, vTopRight, vBottomLeft, vBottomRight, vRed, vGreen, vBlue, vAlpha, vBias);
	}
}

void C_RenderUtils::DisplayTextureCubeFaceColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_TextureCube* pTextureCube, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias)
{
	if (g_debugRenderUtilsFace == Display::E_CubeTextureFace_PosX)
	{
		DisplayTextureCubeColour(x, y, w, h, pTextureCube, MathsStorage::Float4(+1.0f, +1.0f, +1.0f, 0.0f), MathsStorage::Float4(+1.0f, +1.0f, -1.0f, 0.0f), MathsStorage::Float4(+1.0f, -1.0f, +1.0f, 0.0f), MathsStorage::Float4(+1.0f, -1.0f, -1.0f, 0.0f), vRed, vGreen, vBlue, vAlpha, vBias);
	}
	else if (g_debugRenderUtilsFace == Display::E_CubeTextureFace_NegX)
	{
		DisplayTextureCubeColour(x, y, w, h, pTextureCube, MathsStorage::Float4(-1.0f, +1.0f, +1.0f, 0.0f), MathsStorage::Float4(-1.0f, +1.0f, -1.0f, 0.0f), MathsStorage::Float4(-1.0f, -1.0f, +1.0f, 0.0f), MathsStorage::Float4(-1.0f, -1.0f, -1.0f, 0.0f), vRed, vGreen, vBlue, vAlpha, vBias);
	}
	else if (g_debugRenderUtilsFace == Display::E_CubeTextureFace_PosY)
	{
		DisplayTextureCubeColour(x, y, w, h, pTextureCube, MathsStorage::Float4(-1.0f, +1.0f, -1.0f, 0.0f), MathsStorage::Float4(+1.0f, +1.0f, -1.0f, 0.0f), MathsStorage::Float4(-1.0f, +1.0f, +1.0f, 0.0f), MathsStorage::Float4(+1.0f, +1.0f, +1.0f, 0.0f), vRed, vGreen, vBlue, vAlpha, vBias);
	}
	else if (g_debugRenderUtilsFace == Display::E_CubeTextureFace_NegY)
	{
		DisplayTextureCubeColour(x, y, w, h, pTextureCube, MathsStorage::Float4(-1.0f, -1.0f, +1.0f, 0.0f), MathsStorage::Float4(+1.0f, -1.0f, +1.0f, 0.0f), MathsStorage::Float4(-1.0f, -1.0f, -1.0f, 0.0f), MathsStorage::Float4(+1.0f, -1.0f, -1.0f, 0.0f), vRed, vGreen, vBlue, vAlpha, vBias);
	}
	else if (g_debugRenderUtilsFace == Display::E_CubeTextureFace_PosZ)
	{
		DisplayTextureCubeColour(x, y, w, h, pTextureCube, MathsStorage::Float4(-1.0f, +1.0f, +1.0f, 0.0f), MathsStorage::Float4(+1.0f, +1.0f, +1.0f, 0.0f), MathsStorage::Float4(-1.0f, -1.0f, +1.0f, 0.0f), MathsStorage::Float4(+1.0f, -1.0f, +1.0f, 0.0f), vRed, vGreen, vBlue, vAlpha, vBias);
	}
	else if (g_debugRenderUtilsFace == Display::E_CubeTextureFace_NegZ)
	{
		DisplayTextureCubeColour(x, y, w, h, pTextureCube, MathsStorage::Float4(+1.0f, +1.0f, -1.0f, 0.0f), MathsStorage::Float4(-1.0f, +1.0f, -1.0f, 0.0f), MathsStorage::Float4(+1.0f, -1.0f, -1.0f, 0.0f), MathsStorage::Float4(-1.0f, -1.0f, -1.0f, 0.0f), vRed, vGreen, vBlue, vAlpha, vBias);
	}
}

void C_RenderUtils::DisplayTextureCubeColour(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_TextureCube* pTextureCube, const MathsStorage::Float4& vTopLeft, const MathsStorage::Float4& vTopRight, const MathsStorage::Float4& vBottomLeft, const MathsStorage::Float4& vBottomRight, const MathsStorage::Float4& vRed, const MathsStorage::Float4& vGreen, const MathsStorage::Float4& vBlue, const MathsStorage::Float4& vAlpha, const MathsStorage::Float4& vBias)
{
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ColourCubeParameterRed, (F32*)&vRed);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ColourCubeParameterGreen, (F32*)&vGreen);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ColourCubeParameterBlue, (F32*)&vBlue);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ColourCubeParameterAlpha, (F32*)&vAlpha);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ColourCubeParameterBias, (F32*)&vBias);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ColourCubeParameterTopLeft, (F32*)&vTopLeft);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ColourCubeParameterTopRight, (F32*)&vTopRight);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ColourCubeParameterBottomLeft, (F32*)&vBottomLeft);
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_ColourCubeParameterBottomRight, (F32*)&vBottomRight);
	DrawTextured2DRect(x, y, w, h, pTextureCube, &m_Shaders.m_ColourCubePixelShader);
}

void C_RenderUtils::DisplayTexture2DDepth(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture2D* pTexture2D, const MathsStorage::Float4& vScale)
{
	Display::Device()->SetPixelShaderParameterF(m_Shaders.m_Depth2DParameterScale, (F32*)&vScale);
	DrawTextured2DRect(x, y, w, h, pTexture2D, &m_Shaders.m_Depth2DPixelShader);
}

void C_RenderUtils::DrawTextured2DRect(const F32 x, const F32 y, const F32 w, const F32 h, const Display::C_Texture *pTexture, Display::C_PixelShader* pPixelShader)
{
	PERF_EVENT("TextureRenderUtil");

	Display::BasicRender()->BindTextured2D(pTexture, pPixelShader);
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);

	Display::Vertex_XYZ_C_UV* pVertex = (Display::Vertex_XYZ_C_UV*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleStrip, 4, sizeof(Display::Vertex_XYZ_C_UV), Display::Vertex_XYZ_C_UV::m_pDeclaration);
	if (pVertex)
	{
		pVertex->x = x;
		pVertex->y = y;
		pVertex->c = 0xFFFFFFFF;
		pVertex->u = 0.f;
		pVertex->v = 0.f;
		pVertex++;

		pVertex->x = x + w;
		pVertex->y = y;
		pVertex->c = 0xFFFFFFFF;
		pVertex->u = 1.f;
		pVertex->v = 0.f;
		pVertex++;

		pVertex->x = x;
		pVertex->y = y + h;
		pVertex->c = 0xFFFFFFFF;
		pVertex->u = 0.f;
		pVertex->v = 1.f;
		pVertex++;

		pVertex->x = x + w;	
		pVertex->y = y + h;
		pVertex->c = 0xFFFFFFFF;
		pVertex->u = 1.f;
		pVertex->v = 1.f;
		pVertex++;
	
		Display::Device()->EndVertices();
	}
}

}; // namespace Texture
}; // namespace Render
