#include "stdafx.h"

#include "display/device/Device.h"
#include "display/context/Context.h"

namespace Display
{

DEFINE_CONTEXT(Context, C_Context);

/// constructor
C_Context::C_Context()
{
}

/// initialisation function
void C_Context::Initialise()
{
	m_pDevice = Display::Device();

	// not dependant on any other contexts
	m_Flags = m_Id;
}

/// set vertex declaration
void C_Context::SetVertexDeclaration(const C_VertexDeclaration* pVertexDeclaration)
{
	m_pDevice->SetVertexDeclaration(pVertexDeclaration);
}

/// set vertex buffer
void C_Context::SetVertexBuffer(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride)
{
	m_pDevice->SetVertexBuffer(stream, pVertexBuffer, offset, stride);
}

/// set index buffer
void C_Context::SetIndexBuffer(const C_IndexBuffer* pIndexBuffer)
{
	m_pDevice->SetIndexBuffer(pIndexBuffer);
}

/// draw primtive
void C_Context::DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount)
{
	m_pDevice->DrawIndexedPrimitive(primitiveType, baseVertex, minIndex, vertexCount, startIndex, indexCount);
}

// set alpha test function
void C_Context::SetAlphaTestFunc(const E_CmpFunc alphaTestFunc)
{
	m_pDevice->SetAlphaTestFunc(alphaTestFunc);
}

/// set blend high precision enable
void C_Context::SetBlendHighPrecisionEnable(const E_Bool blendHighPrecisionEnable)
{
}

// set wireframe / solid
void C_Context::SetFillMode(E_FillMode fillMode)
{
	m_pDevice->SetFillMode(fillMode);
}

/// set blend enable
void C_Context::SetBlendEnable(const E_Bool blendEnable)
{
	m_pDevice->SetBlendEnable(blendEnable);
}

/// set cull mode
void C_Context::SetCullMode(const E_CullMode cullMode)
{
	m_pDevice->SetCullMode(cullMode);
}

/// set depthwrite enable
void C_Context::SetDepthWriteEnable(const E_Bool depthWriteEnable)
{
	m_pDevice->SetDepthWriteEnable(depthWriteEnable);
}

/// set depth func
void C_Context::SetDepthFunc(const E_CmpFunc depthFunc)
{
	// implement this
}

/// change the current vertex shader
void C_Context::SetVertexShader(const Display::C_VertexShader *pShader)
{
	m_pDevice->SetVertexShader(pShader);
}

/// change the current pixel shader
void C_Context::SetPixelShader(const Display::C_PixelShader *pShader)
{
	m_pDevice->SetPixelShader(pShader);
}

/// set stencil states
void C_Context::SetStencilState(const Display::C_StencilState& stencilState)
{
	m_pDevice->SetStencilState(stencilState);
}

/// set colour write enable
void C_Context::SetColourWriteEnable(const U32 flags)
{
	m_pDevice->SetColourWriteEnable(flags);
}

/// set renderstate
void C_Context::SetRenderState( Display::E_Blend srcBlend, Display::E_Blend destBlend, Display::E_BlendOp blendOp,
	Display::E_Bool alphaToMaskEnable, Display::E_Bool alphaTestEnable, U32 alphaRef)
{
#ifdef PLATFORM_PS3
	m_pDevice->SetBlend(
		srcBlend,
		destBlend,
		blendOp,
		Display::E_Blend_One,
		Display::E_Blend_Zero,
		Display::E_BlendOp_Add);
#else
	m_pDevice->SetBlendSrc(srcBlend);
	m_pDevice->SetBlendDst(destBlend);
	m_pDevice->SetBlendOp(blendOp);
#endif
	m_pDevice->SetAlphaTestFunc(E_CmpFunc_Greater);
	m_pDevice->SetAlphaTestRef(alphaRef);
	m_pDevice->SetAlphaTestEnable(alphaTestEnable);
	m_pDevice->SetAlphaToMaskEnable(alphaToMaskEnable);
}

/// set pixelshader parameter
void C_Context::SetPixelShaderParameterF(Display::C_PixelShaderParameter handle, const F32 *src)
{
	Display::Device()->SetPixelShaderParameterF(handle, src);
}

/// commit pixelshader parameters
void C_Context::SetPixelShaderParameters(Display::C_PixelShader *pShader)
{
	Display::Device()->SetPixelShaderParameters(pShader);
}

/// set a pixel shader constant register
void C_Context::SetPixelShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount)
{
	m_pDevice->SetPixelShaderConstantF(startRegister, pData, registerCount);
}

/// set a vertex shader constant register
void C_Context::SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount)
{
	m_pDevice->SetVertexShaderConstantF(startRegister, pData, registerCount);
}

/// set a texture. this function takes a pointer to a GcmTexture or a D3DTexture
void C_Context::SetTexture(const U32 sampler, const void* pTexture)
{
#if PLATFORM_PC
	m_pDevice->m_pd3dDevice->SetTexture(sampler, (IDirect3DBaseTexture9*)pTexture);
#elif PLATFORM_PS3
	CellGcmTexture* p = (CellGcmTexture*)pTexture;
	if (p)
	{
		Display::CheckGCMTexture(p);
		cellGcmSetTexture(gCellGcmCurrentContext, sampler, p);
	}
	else
	{
		cellGcmSetTexture(gCellGcmCurrentContext, sampler, (CellGcmTexture*)Display::Device()->GetNullTexture()->GetGPUTexture());
	}
	cellGcmSetTextureControl(gCellGcmCurrentContext, sampler, 1, 0, 12<<8, m_iAniso[sampler] );
#endif
}

/// set a vertex texture. this function takes a pointer to a GcmTexture or a D3DTexture
void C_Context::SetVertexTexture(const U32 sampler, const void* pTexture)
{
#if PLATFORM_PC
	m_pDevice->m_pd3dDevice->SetTexture(sampler + D3DVERTEXTEXTURESAMPLER0, (IDirect3DBaseTexture9*)pTexture);
#elif PLATFORM_PS3
	CellGcmTexture* p = (CellGcmTexture*)pTexture;
	Display::CheckGCMTexture(p);
	cellGcmSetVertexTexture(gCellGcmCurrentContext, sampler, p);
	cellGcmSetVertexTextureControl(gCellGcmCurrentContext, sampler, 1, 0, 12<<8 );
#endif
}

#if PLATFORM_PS3
static const int g_iAniso[] = 
{
	CELL_GCM_TEXTURE_MAX_ANISO_1, CELL_GCM_TEXTURE_MAX_ANISO_1, CELL_GCM_TEXTURE_MAX_ANISO_2, CELL_GCM_TEXTURE_MAX_ANISO_2,
	CELL_GCM_TEXTURE_MAX_ANISO_4, CELL_GCM_TEXTURE_MAX_ANISO_4, CELL_GCM_TEXTURE_MAX_ANISO_6, CELL_GCM_TEXTURE_MAX_ANISO_6,
	CELL_GCM_TEXTURE_MAX_ANISO_8, CELL_GCM_TEXTURE_MAX_ANISO_8, CELL_GCM_TEXTURE_MAX_ANISO_10, CELL_GCM_TEXTURE_MAX_ANISO_10,
	CELL_GCM_TEXTURE_MAX_ANISO_12, CELL_GCM_TEXTURE_MAX_ANISO_12, CELL_GCM_TEXTURE_MAX_ANISO_12, CELL_GCM_TEXTURE_MAX_ANISO_12,
	CELL_GCM_TEXTURE_MAX_ANISO_16
};
#endif

/// set sampler states
void C_Context::SetSamplerState(const U32 sampler,
	const E_TextureFilter filterMag, E_TextureFilter filterMin, E_TextureFilter filterMip,
	const E_TextureAddress addressU, const E_TextureAddress addressV, const E_TextureAddress addressW,
	E_Bool gamma, U8 iMaxAniso, S8 MipMapLodBias)
{
	m_pDevice->SetTextureAddressUVW(sampler, addressU, addressV, addressW);
	m_pDevice->SetTextureFilterMagMinMip(sampler, filterMag, filterMin, filterMip);
	m_pDevice->SetTextureGamma(sampler, gamma);
	m_pDevice->SetTextureLodBias(sampler, MipMapLodBias);

#if 0 | PLATFORM_PC
	m_pDevice->m_pd3dDevice->SetSamplerState(sampler, D3DSAMP_MAXANISOTROPY, iMaxAniso);
#elif PLATFORM_PS3
	int iAniso = g_iAniso[iMaxAniso];
	m_iAniso[sampler] = iAniso;
	cellGcmSetTextureControl(gCellGcmCurrentContext, sampler, 1, 0, 12<<8, iAniso );
#endif
}

/// set vertex sampler states
void C_Context::SetVertexSamplerState(const U32 sampler,
	const E_TextureFilter filterMag, E_TextureFilter filterMin, E_TextureFilter filterMip,
	const E_TextureAddress addressU, const E_TextureAddress addressV, const E_TextureAddress addressW,
	E_Bool gamma, U8 iMaxAniso)
{
	m_pDevice->SetVertexTextureFilterMagMinMip(sampler, filterMag, filterMin, filterMip);
	m_pDevice->SetVertexTextureAddressUVW(sampler, addressU, addressV, addressW);
}

C_ContextManager g_ContextManager;

}	// namespace
