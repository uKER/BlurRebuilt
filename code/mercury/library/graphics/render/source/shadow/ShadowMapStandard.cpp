#include "stdafx.h"
#include "Render/Shadow/ShadowMapStandard.h"

EXTERN_SHADER(vsShadowMapStandardDepth);
EXTERN_SHADER(psShadowMapStandardDepth);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Shadow/PS3/vsShadowMapStandardDepth.shader"
#include "Render/Shaders/Shadow/PS3/psShadowMapStandardDepth.shader"
#endif
#endif

namespace Render
{
namespace Shadow
{

C_StandardMapShaders C_StandardMap::m_Shaders;

C_StandardMap::C_StandardMap()
{
	m_Levels				= 1;
	m_ColourFormat			= Display::E_Format_R32F;
	m_DepthStencilFormat	= Display::Device()->GetBackBuffer().GetDepthStencilFormat();

	Display::MatrixSetIdentity(&m_LightMatrix);
	Display::MatrixSetIdentity(&m_ShadowMatrix);
	m_hLightMatrix.SetIdentity();
	m_hShadowMatrix.SetIdentity();
	m_hTextureMatrix.AX(Maths::HVec4(0.5f,  0.0f, 0.0f, 0.0f));
	m_hTextureMatrix.AY(Maths::HVec4(0.0f, -0.5f, 0.0f, 0.0f));
	m_hTextureMatrix.AZ(Maths::HVec4(0.0f,  0.0f, 1.0f, 0.0f));
	m_hTextureMatrix.AW(Maths::HVec4(0.5f,  0.5f, 0.0f, 1.0f));

	g_debugUseDepthStencilTexture = true;

	g_pPcfShadowMap = this;
}

C_StandardMap::~C_StandardMap()
{
	g_pPcfShadowMap = NULL;
}

bool C_StandardMap::OnCreate()
{
	RESOURCE_MEMORY_ID("ShadowMap");

//	if(!CreateShaders())
//		return false;

	return true;
}

void C_StandardMap::OnDestroy()
{
//	DestroyShaders();
}

bool C_StandardMap::CreateShaders()
{
	if(!Display::CreateVertexShaderFromFunc("Shaders/vsShadowMapStandardDepth", 0, &m_Shaders.m_vsDepth, REF_SHADER(vsShadowMapStandardDepth)))
		return false;
	if(!Display::CreatePixelShaderFromFunc("Shaders/psShadowMapStandardDepth", 0, &m_Shaders.m_psDepth, REF_SHADER(psShadowMapStandardDepth)))
		return false;
	
	return true;
}

void C_StandardMap::DestroyShaders()
{
	Display::Device()->Destroy(&m_Shaders.m_vsDepth);
	Display::Device()->Destroy(&m_Shaders.m_psDepth);
}

void C_StandardMap::SetShaders()
{
	Display::Device()->SetVertexShader(&m_Shaders.m_vsDepth);
	
	Display::Device()->SetPixelShader(&m_Shaders.m_psDepth);
}

void C_StandardMap::SetTexture(const U32 sampler, const U32 lightMatrixRegister, const U32 shadowMatrixRegister)
{
	Display::Device()->SetVertexShaderConstantF(lightMatrixRegister, (F32*)&m_LightMatrix, 4);
	Display::Device()->SetVertexShaderConstantF(shadowMatrixRegister, (F32*)&m_ShadowMatrix, 4);

	Display::Device()->SetTexture(sampler, &m_RenderTexture.GetTexture());
	#if defined(USE_HARDWARESHADOWMAP)
	Display::Device()->SetTextureFilterMagMinMip(sampler, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_None);
	#else
	Display::Device()->SetTextureFilterMagMinMip(sampler, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_None);
	#endif
#ifdef PLATFORM_PS3
	Display::C_TextureAddress textureAddress;
	textureAddress.SetAddressU(Display::E_TextureAddress_Border);
	textureAddress.SetAddressV(Display::E_TextureAddress_Border);
	textureAddress.SetAddressW(Display::E_TextureAddress_Clamp);
	textureAddress.m_zfunc = CELL_GCM_TEXTURE_ZFUNC_LESS;
	Display::Device()->SetTextureAddressCached(sampler, textureAddress);
#else
	Display::Device()->SetTextureAddressUVW(sampler, Display::E_TextureAddress_Border, Display::E_TextureAddress_Border, Display::E_TextureAddress_Clamp);
#endif
	Display::Device()->SetTextureBorderColour(sampler, 0xFFFFFFFF);
}

void C_StandardMap::Bind()
{
	C_ShadowMap::Bind();

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	m_hLightMatrix = viewSettings3D.m_WorldToDevice;
	m_hShadowMatrix = viewSettings3D.m_WorldToDevice * m_hTextureMatrix;

	Maths::Store(m_hLightMatrix, &m_LightMatrix);
	Maths::Store(m_hShadowMatrix, &m_ShadowMatrix);

	SetShaders();
}

void C_StandardMap::Unbind()
{

	C_ShadowMap::Unbind();
}

void C_StandardMap::DebugRender()
{
	if(!m_RenderTexture.IsCreated())
		return;

	#ifndef PLATFORM_PS3
	Display::BasicRender()->DrawTextured2DRect(1024.f - 300.f - 20.f, 20.f, 300.f, 300.f, Maths::RGBA(255, 255, 255, 255), &m_RenderTexture.GetTexture());
	#endif

	#ifdef PLATFORM_PS3
	Display::C_Texture2D texture;

	CellGcmTexture* pGcmTexture = texture.GetGPUTexture();
	pGcmTexture->format			= CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_LN;
	pGcmTexture->mipmap			= 1;
	pGcmTexture->dimension		= CELL_GCM_TEXTURE_DIMENSION_2;
	pGcmTexture->cubemap		= 0;
	pGcmTexture->remap			=	CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
									CELL_GCM_TEXTURE_REMAP_FROM_G << 6 |
									CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
									CELL_GCM_TEXTURE_REMAP_FROM_A << 2 |
									CELL_GCM_TEXTURE_REMAP_FROM_B;
	pGcmTexture->width			= m_Width;
	pGcmTexture->height			= m_Height;
	pGcmTexture->depth			= 1;
	pGcmTexture->location		= CELL_GCM_LOCATION_LOCAL;
	pGcmTexture->pitch			= m_DepthStencilSurface.GetPitch();
	pGcmTexture->offset			= m_DepthStencilSurface.GetOffset();

	Display::BasicRender()->DrawTextured2DRect(1024.f - 300.f - 20.f, 20.f, 300.f, 300.f, Maths::RGBA(255, 255, 255, 255), &texture);
	#endif
}

C_StandardMap* g_pPcfShadowMap = NULL;

} // namespace Shadow
} // namespace Render
