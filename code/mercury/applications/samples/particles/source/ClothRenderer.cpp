#include "stdafx.h"
// header

#include "ClothRenderer.h"

#include "Render/Shaders/Constants.h"
#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Buffer/DynamicVB.h"
#include "Render/Shaders/Constants.h"

#include "Render/State/RenderState.h"

using namespace DynamicEffect2;

namespace DynamicEffect2
{
	Debug::Bool g_debugClothDebugRender("Render.Cloth.DebugRender", false);
	Debug::Colour g_debugClothColour("Render.Cloth.Colour", 1.f, 1.f, 1.f, 1.f, Debug::Gamma(2.2f));
	Debug::Float g_debugClothSpecularFactor("Render.Cloth.SpecularFactor", 0.1f, Debug::Range(0.0f, 2.0f));
	Debug::Float g_debugClothSpecularPower("Render.Cloth.SpecularPower", 0.5f, Debug::Range(0.0f, 5.0f));
	Debug::Float g_debugClothAmbient("Render.Cloth.Ambient", 0.25f, Debug::Range(0.0f, 5.0f));

	#if defined(DEVICE_PCD3D9)
	#include "Render/Shaders/Dynamic/Cloth/PCD3D9/vsCloth.shader"
	#include "Render/Shaders/Dynamic/Cloth/PCD3D9/psCloth.shader"
	#include "Render/Shaders/Dynamic/Cloth/PCD3D9/psClothHDR.shader"
	#include "Render/Shaders/Dynamic/Cloth/PCD3D9/psClothHDRU32.shader"
	#elif defined(DEVICE_XB2D3D9)
	#include "Render/Shaders/Dynamic/Cloth/XB2/vsCloth.shader"
	#include "Render/Shaders/Dynamic/Cloth/XB2/psCloth.shader"
	#include "Render/Shaders/Dynamic/Cloth/XB2/psClothHDR.shader"
	#include "Render/Shaders/Dynamic/Cloth/XB2/psClothHDRU32.shader"
	#elif defined(DEVICE_PS3GCM)
	#include "Render/Shaders/Dynamic/Cloth/PS3/vsCloth.shader"
	#include "Render/Shaders/Dynamic/Cloth/PS3/psCloth.shader"
	#include "Render/Shaders/Dynamic/Cloth/PS3/psClothHDR.shader"
	#include "Render/Shaders/Dynamic/Cloth/PS3/psClothHDRU32.shader"
	#endif

	typedef Display::Vertex_XYZ_C_UV_N	ClothVertex;
};

// initialise system with correct values, then define custom creation and updating functions
C_SampleClothRenderer::C_SampleClothRenderer(C_Renderer* pRenderer, const U32 iIndex) : C_ClothRenderer<C_SampleClothRenderer, ClothInstance>(pRenderer, iIndex)
{
	for(S32 i = 0; i < 32; ++i)
	{
		C_Cloth* pCloth = new C_Cloth;
		m_ObjectList.Cache(pCloth);
	}
}

bool C_SampleClothRenderer::Create()
{
	if (!Display::CreateVertexShaderFromFunc("vsCloth", 0, &m_VertexShader, sizeof(vsCloth), vsCloth))
		return false;

	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		// choose which set of shaders we need to use - we only have depth fog for shader tonemap at the moment
		if(eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psClothHDRU32", 0, &m_PixelShader, sizeof(psClothHDRU32), psClothHDRU32))
				return false;
		}
		else if(eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psClothHDR", 0, &m_PixelShader, sizeof(psClothHDR), psClothHDR))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psCloth", 0, &m_PixelShader, sizeof(psCloth), psCloth))
			return false;
	}

	m_ShaderSettings = m_PixelShader.GetParameter(CP_RENDER_CLOTH_SETTINGS, "g_vSettings");
	m_ShaderColour = m_PixelShader.GetParameter(CP_RENDER_CLOTH_COLOUR, "g_vColour");

	return true;
}

void C_SampleClothRenderer::Destroy()
{
	if(m_VertexShader.IsCreated())
		Display::Device()->Destroy(&m_VertexShader);
	if(m_PixelShader.IsCreated())
		Display::Device()->Destroy(&m_PixelShader);
}

void C_SampleClothRenderer::SetMaterial(const ClothInstance* pInstance) const
{
	// matrix
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&pInstance->transform, 4);
	
	// textures
	if(pInstance->pDiffuse && pInstance->pDiffuse->IsCreated())
		Display::Device()->SetTexture(0, pInstance->pDiffuse);
	else
		Display::Device()->SetTexture(0, &m_pRenderer->GetWhiteTexture());
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);

	// material
	MathsStorage::Float4 vSettings(g_debugClothSpecularFactor, g_debugClothSpecularPower, g_debugClothAmbient, 0.f);
	Display::Device()->SetPixelShaderParameterF(m_ShaderSettings, (F32*)&vSettings);
	Display::Device()->SetPixelShaderParameterF(m_ShaderColour, (F32*)&pInstance->vColour);

	// alpha
	if(pInstance->bTransparent)
	{
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
		Display::Device()->SetBlendMode(Display::E_BlendMode_AlphaPreMultiply);
	}
	else
	{
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	}

	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		Display::Device()->SetTexture(15, Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture());
		Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(15, Display::E_TextureAddress_Clamp);
		MathsStorage::Float4 luminanceParam = Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_LUMINANCE, (F32*)&luminanceParam, 1);
	}
}
















