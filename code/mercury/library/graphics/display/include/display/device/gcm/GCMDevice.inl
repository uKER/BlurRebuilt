#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_INL
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_INL

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Debug/Variables.h>

extern Debug::Int g_debugDisplayTextureMaxAniso;
extern Debug::Float g_debugDepthBiasScaleFactor;

namespace Display
{
namespace GCM
{

INLINE void C_Sampler::InvalidateState()
{
	C_SamplerBase::InvalidateState();

	m_pTexture		= NULL;

	m_BorderColour	= m_BorderColourDefault;
	m_LodBias		= m_LodBiasDefault;
}

INLINE void C_Stream::InvalidateState()
{
	m_pVertexBuffer = NULL;
}

INLINE CGprofile C_GCMDevice::GetCGProfileVS()
{
	return m_cgLatestProfileVS;
}

INLINE CGprofile C_GCMDevice::GetCGProfilePS()
{
	return m_cgLatestProfilePS;
}
	
INLINE CGcontext C_GCMDevice::GetCGContext()
{
	return m_cgContext;
}

INLINE const S32 C_GCMDevice::GetBackBufferCount() const
{
	return m_BackBuffers.Size();
}

INLINE const C_BackBuffer& C_GCMDevice::GetBackBuffer() const
{
	BIZ_ASSERT(m_CurrentBackBuffer >= 0 && m_CurrentBackBuffer < m_BackBuffers.Size());
	return *m_BackBuffers[m_CurrentBackBuffer];
}

INLINE const C_BackBuffer& C_GCMDevice::GetBackBuffer(const S32 backBufferIndex) const
{
	BIZ_ASSERT(backBufferIndex >= 0 && backBufferIndex < m_BackBuffers.Size());
	return *m_BackBuffers[backBufferIndex];	
}

INLINE const S32 C_GCMDevice::GetFrontBufferCount() const
{
	return m_FrontBuffers.Size();
}

INLINE const C_FrontBuffer& C_GCMDevice::GetFrontBuffer() const
{
	BIZ_ASSERT(m_CurrentFrontBuffer >= 0 && m_CurrentFrontBuffer < m_FrontBuffers.Size());
	return *m_FrontBuffers[m_CurrentFrontBuffer];
}

INLINE const C_FrontBuffer& C_GCMDevice::GetFrontBuffer(const S32 frontBufferIndex) const
{
	BIZ_ASSERT(frontBufferIndex >= 0 && frontBufferIndex < m_FrontBuffers.Size());
	return *m_FrontBuffers[frontBufferIndex];
}

INLINE const C_FrontBuffer& C_GCMDevice::GetSwappedFrontBuffer() const
{
	BIZ_ASSERT(m_SwappedFrontBuffer >= 0 && m_SwappedFrontBuffer < m_FrontBuffers.Size());
	return *m_FrontBuffers[m_SwappedFrontBuffer];
}

INLINE const C_RenderTarget& C_GCMDevice::GetRenderTarget() const
{
	return m_RenderTarget;
}

INLINE const S32 C_GCMDevice::GetWindowCount() const
{
	return m_Windows.Size();
}

INLINE C_Window* C_GCMDevice::GetWindow(const S32 windowIndex)
{
	BIZ_ASSERT(windowIndex >= 0 && windowIndex < m_Windows.Size());
	return m_Windows[windowIndex];
}

INLINE void C_GCMDevice::InvalidateVertexShaderCache()
{
	m_VertexShaderCache.Clear();
}

INLINE void C_GCMDevice::SetAlphaTestEnable(const E_Bool alphaTestEnable)
{
	GCM_NAMESPACE::cellGcmSetAlphaTestEnable(alphaTestEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE);
	m_AlphaTestEnable = alphaTestEnable;
	
	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetAlphaToMaskEnable(const E_Bool alphaToMaskEnable)
{
	const uint32_t multisampleEnable = GetRenderTarget().GetColourSurface()->GetMultisample() != E_Multisample_None ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	GCM_NAMESPACE::cellGcmSetAntiAliasingControl(multisampleEnable, (alphaToMaskEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE), (alphaToMaskEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE), 0xffff);
	m_AlphaToMaskEnable = alphaToMaskEnable;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetAlphaTestFunc(const E_CmpFunc alphaTestFunc)
{
	GCM_NAMESPACE::cellGcmSetAlphaFunc(GCMCmpFunc(alphaTestFunc), m_AlphaTestRef);
	m_AlphaTestFunc = alphaTestFunc;
	
	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetAlphaTestRef(const U32 alphaTestRef)
{
	GCM_NAMESPACE::cellGcmSetAlphaFunc(GCMCmpFunc(m_AlphaTestFunc), alphaTestRef);
	m_AlphaTestRef = alphaTestRef;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetBlendDst(const E_Blend blendDst)
{
	const U32 gcmBlendSrc = GCMBlend(m_BlendSrc);
	const U32 gcmBlendDst = GCMBlend(blendDst);
	const U32 gcmBlendSrcAlpha = GCMBlend(m_BlendSrcAlpha);
	const U32 gcmBlendDstAlpha = GCMBlend(m_BlendDstAlpha);
	GCM_NAMESPACE::cellGcmSetBlendFunc(gcmBlendSrc, gcmBlendDst, gcmBlendSrcAlpha, gcmBlendDstAlpha);
	m_BlendDst = blendDst;
	
	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetBlend(const E_Blend blendSrc, const E_Blend blendDst, const E_BlendOp blendOp, const E_Blend blendSrcAlpha, const E_Blend blendDstAlpha, const E_BlendOp blendOpAlpha)
{
	if(	(blendSrc != m_BlendSrc) ||	
		(blendDst != m_BlendDst) ||	
		(blendSrcAlpha != m_BlendSrcAlpha) || 
		(blendDstAlpha != m_BlendDstAlpha) ||	
		(blendOp != m_BlendOp) ||
		(blendOpAlpha != m_BlendOpAlpha) )
	{
		const U32 gcmBlendSrc = GCMBlend(blendSrc);
		const U32 gcmBlendDst = GCMBlend(blendDst);
		const U32 gcmBlendSrcAlpha = GCMBlend(blendSrcAlpha);
		const U32 gcmBlendDstAlpha = GCMBlend(blendDstAlpha);

		GCM_NAMESPACE::cellGcmSetBlendFunc(gcmBlendSrc, gcmBlendDst, gcmBlendSrcAlpha, gcmBlendDstAlpha);

		m_BlendSrc = blendSrc;
		m_BlendDst = blendDst;
		m_BlendSrcAlpha = blendSrcAlpha;
		m_BlendDstAlpha = blendDstAlpha;

		const U32 gcmBlendOp = GCMBlendOp(blendOp);
		const U32 gcmBlendOpAlpha = GCMBlendOp(blendOpAlpha);

		GCM_NAMESPACE::cellGcmSetBlendEquation(gcmBlendOp, gcmBlendOpAlpha);

		m_BlendOp = blendOp;
		m_BlendOpAlpha = blendOpAlpha;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetBlendDstAlpha(const E_Blend blendDstAlpha)
{
	const U32 gcmBlendSrc = GCMBlend(m_BlendSrc);
	const U32 gcmBlendDst = GCMBlend(m_BlendDst);
	const U32 gcmBlendSrcAlpha = GCMBlend(m_BlendSrcAlpha);
	const U32 gcmBlendDstAlpha = GCMBlend(blendDstAlpha);
	GCM_NAMESPACE::cellGcmSetBlendFunc(gcmBlendSrc, gcmBlendDst, gcmBlendSrcAlpha, gcmBlendDstAlpha);
	m_BlendDstAlpha = blendDstAlpha;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetBlendEnable(const E_Bool blendEnable)
{
	GCM_NAMESPACE::cellGcmSetBlendEnable(blendEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE);
	m_BlendEnable = blendEnable;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetBlendOp(const E_BlendOp blendOp)
{
	const U32 gcmBlendOp = GCMBlendOp(blendOp);
	const U32 gcmBlendOpAlpha = GCMBlendOp(m_BlendOpAlpha);
	GCM_NAMESPACE::cellGcmSetBlendEquation(gcmBlendOp, gcmBlendOpAlpha);
	m_BlendOp = blendOp;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetBlendOpAlpha(const E_BlendOp blendOpAlpha)
{
	const U32 gcmBlendOp = GCMBlendOp(m_BlendOp);
	const U32 gcmBlendOpAlpha = GCMBlendOp(blendOpAlpha);
	GCM_NAMESPACE::cellGcmSetBlendEquation(gcmBlendOp, gcmBlendOpAlpha);
	m_BlendOpAlpha = blendOpAlpha;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetBlendSrc(const E_Blend blendSrc)
{
	const U32 gcmBlendSrc = GCMBlend(blendSrc);
	const U32 gcmBlendDst = GCMBlend(m_BlendDst);
	const U32 gcmBlendSrcAlpha = GCMBlend(m_BlendSrcAlpha);
	const U32 gcmBlendDstAlpha = GCMBlend(m_BlendDstAlpha);
	GCM_NAMESPACE::cellGcmSetBlendFunc(gcmBlendSrc, gcmBlendDst, gcmBlendSrcAlpha, gcmBlendDstAlpha);
	m_BlendSrc = blendSrc;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetBlendSrcAlpha(const E_Blend blendSrcAlpha)
{
	const U32 gcmBlendSrc = GCMBlend(m_BlendSrc);
	const U32 gcmBlendDst = GCMBlend(m_BlendDst);
	const U32 gcmBlendSrcAlpha = GCMBlend(blendSrcAlpha);
	const U32 gcmBlendDstAlpha = GCMBlend(m_BlendDstAlpha);
	GCM_NAMESPACE::cellGcmSetBlendFunc(gcmBlendSrc, gcmBlendDst, gcmBlendSrcAlpha, gcmBlendDstAlpha);
	m_BlendSrcAlpha = blendSrcAlpha;
	
	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetColourWriteEnable(U32 colourWriteEnable)
{
	if(colourWriteEnable == E_ColourWriteEnable_Default)
		colourWriteEnable = m_ColourWriteEnableDefault;

	GCM_NAMESPACE::cellGcmSetColorMask(colourWriteEnable);
}

INLINE void C_GCMDevice::SetCullMode(E_CullMode cullMode)
{
	if(cullMode == E_CullMode_Front)
		cullMode = E_CullMode_Reversed;
	else if(cullMode == E_CullMode_Back)
		cullMode = E_CullMode_Default;

	if(cullMode == E_CullMode_Default)
		cullMode = m_CullModeDefault;
	if(cullMode == E_CullMode_Reversed)
		cullMode = FlippedCullMode(m_CullModeDefault);

	if(m_CullModeFlip)
		cullMode = FlippedCullMode(cullMode);

	switch(cullMode)
	{
	case E_CullMode_None:
		{
			GCM_NAMESPACE::cellGcmSetCullFaceEnable(CELL_GCM_FALSE);
		}
		break;
	
	case E_CullMode_Cw:
		{
			GCM_NAMESPACE::cellGcmSetCullFaceEnable(CELL_GCM_TRUE);
			GCM_NAMESPACE::cellGcmSetFrontFace(CELL_GCM_CW);
		}
		break;

	case E_CullMode_Ccw:
		{
			GCM_NAMESPACE::cellGcmSetCullFaceEnable(CELL_GCM_TRUE);
			GCM_NAMESPACE::cellGcmSetFrontFace(CELL_GCM_CCW);
		}
		break;
	
	default:
		BIZ_ASSERT_FAIL("Unsupported cull mode");
		break;
	}

	m_CullMode = cullMode;
	
	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetCullModeFlip()
{
	m_CullModeFlip = !m_CullModeFlip;
	if (m_CullModeFlip)
		SetCullMode(m_CullMode);
	else
		SetCullMode(FlippedCullMode(m_CullMode));
}

INLINE void C_GCMDevice::SetCullModeFlip(const bool cullModeFlip)
{
	if (m_CullModeFlip != cullModeFlip)
	{
		m_CullModeFlip = cullModeFlip;
		if (m_CullModeFlip)
			SetCullMode(m_CullMode);
		else
			SetCullMode(FlippedCullMode(m_CullMode));
	}
}

INLINE void C_GCMDevice::SetDepthBias(const F32 depthBias)
{	
	if(depthBias == 0.0f)
	{
		GCM_NAMESPACE::cellGcmSetPolygonOffsetFillEnable(CELL_GCM_FALSE);
	}
	else
	{
		GCM_NAMESPACE::cellGcmSetPolygonOffsetFillEnable(CELL_GCM_TRUE);

		const F32 factor = 0.0f;
		const F32 units = depthBias * (F32)g_debugDepthBiasScaleFactor;

		GCM_NAMESPACE::cellGcmSetPolygonOffset(factor, units);
	}

}

INLINE void C_GCMDevice::SetDepthEnable(E_Bool depthEnable)
{
	if(depthEnable == E_Bool_Default)
		depthEnable = m_DepthEnableDefault;

#if 1
	if(!m_RenderTarget.m_pDepthStencilSurface)
		depthEnable = E_Bool_False; // Ensure depth is disabled if no depth stencil buffer

	//if(depthEnable != m_DepthEnable)
	{
		GCM_NAMESPACE::cellGcmSetDepthTestEnable(depthEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE);
			
		m_DepthEnable = depthEnable;
	}
#else
	if(depthEnable != m_DepthEnable)
	{
		if(!m_RenderTarget.m_pDepthStencilSurface && depthEnable == E_Bool_True)
		{
			// dont enable depth test if a depth stencil surface is not set...this will crash gcm!
		}
		else
		{
			GCM_NAMESPACE::cellGcmSetDepthTestEnable(depthEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE);
			m_DepthEnable = depthEnable;
		}
	}
#endif

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetDepthFunc(E_CmpFunc depthFunc)
{
	if(depthFunc == E_CmpFunc_Default)
		depthFunc = m_DepthFuncDefault;

	if(m_RenderTarget.m_pDepthStencilSurface)
	//if(m_RenderTarget.m_pDepthStencilSurface)
	{
		GCM_NAMESPACE::cellGcmSetDepthFunc(GCMCmpFunc(depthFunc));

		m_DepthFunc = depthFunc;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}
	
INLINE void C_GCMDevice::SetDepthWriteEnable(E_Bool depthWriteEnable)
{
	if(depthWriteEnable == E_Bool_Default)
		depthWriteEnable = m_DepthWriteEnableDefault;

#if 1
	if(!m_RenderTarget.m_pDepthStencilSurface)
		depthWriteEnable = E_Bool_False; // Ensure depth write is disabled if no depth stencil buffer

	GCM_NAMESPACE::cellGcmSetDepthMask(depthWriteEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE);
	m_DepthWriteEnable = depthWriteEnable;
#else
	if(m_RenderTarget.m_pDepthStencilSurface && depthWriteEnable != m_DepthWriteEnable)
	//if(m_RenderTarget.m_pDepthStencilSurface && depthWriteEnable)
	{
		GCM_NAMESPACE::cellGcmSetDepthMask(depthWriteEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE);

		m_DepthWriteEnable = depthWriteEnable;
	}
#endif

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetFillMode(E_FillMode fillMode)
{
	if(fillMode == E_FillMode_Default)
		fillMode = m_FillModeDefault;

	GCM_NAMESPACE::cellGcmSetFrontPolygonMode(GCMFillMode(fillMode));
	m_FillMode = fillMode;

#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetPointSpriteEnable(const E_Bool pointSpriteEnable)
{
	GCM_NAMESPACE::cellGcmSetPointSpriteControl(pointSpriteEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE,
												CELL_GCM_POINT_SPRITE_RMODE_ZERO,
												CELL_GCM_POINT_SPRITE_TEX0);
	m_PointSpriteEnable = pointSpriteEnable;
	
	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetIndexBuffer(const C_IndexBuffer* pIndexBuffer)
{
	BIZ_ASSERT(HaveThreadOwnership());

	if(!m_SetVertexDeclaration)
		m_SetVertexDeclaration = true;
		
	if(!m_InvalidateVertexCache)
		m_InvalidateVertexCache = true;

	m_pIndexBuffer = pIndexBuffer;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetPixelShader(const C_PixelShader* pPixelShader)
{
	BIZ_ASSERT(HaveThreadOwnership());

	m_Display2.GetContextNoCheckNoTrack().SetPixelShader(pPixelShader);
	m_pPixelShader = pPixelShader;
}

INLINE void C_GCMDevice::SetPixelShader(const C_PixelShader* pPixelShader, const U8* pMicroCode)
{
#ifdef USE_CGB_SHADERS
	//if(pPixelShader != m_pPixelShader) // Shader needs to be set as its microcode has been patched.
	{
		if(pPixelShader)
		{
			U32 offset = 0;
			cellGcmAddressToOffset(pMicroCode, &offset);
			
			CellCgbFragmentProgramConfiguration cgbProgramConfig;

			cgbProgramConfig.offset					= offset;
			cgbProgramConfig.attributeInputMask 	= pPixelShader->m_ProgramConfig.attributeInputMask;
			cgbProgramConfig.texCoordsInputMask 	= pPixelShader->m_ProgramConfig.texCoordsInputMask;
			cgbProgramConfig.texCoords2D			= pPixelShader->m_ProgramConfig.texCoords2D;
			cgbProgramConfig.texCoordsCentroid		= pPixelShader->m_ProgramConfig.texCoordsCentroid;
			cgbProgramConfig.fragmentControl		= pPixelShader->m_ProgramConfig.fragmentControl;
			cgbProgramConfig.registerCount			= pPixelShader->m_ProgramConfig.registerCount;
			
			GCM_NAMESPACE::cellGcmSetFragmentProgramLoadLocation(&cgbProgramConfig, CELL_GCM_LOCATION_LOCAL);
			
			//GCM_NAMESPACE::cellGcmSetUpdateFragmentProgramParameter(offset);

			GCM_NAMESPACE::cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);
		}

		m_pPixelShader = pPixelShader;
	}
#else
	if(pPixelShader && pMicroCode)
	{
		U32 gcmOffset = 0;
		GCM_NAMESPACE::cellGcmAddressToOffset(pMicroCode, &gcmOffset);
		
		GCM_NAMESPACE::cellGcmSetFragmentProgramOffset(pPixelShader->m_cgProgram, gcmOffset, 0);
		GCM_NAMESPACE::cellGcmSetFragmentProgramControl(pPixelShader->m_cgProgram, 1, 1, 0);

		m_pPixelShader = 0;
	}
#endif

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetPixelShaderConstantB(const U32 startRegister, const BOOL* pData, const U32 registerCount)
{
}

INLINE void C_GCMDevice::SetPixelShaderConstantI(const U32 startRegister, const S32* pData, const U32 registerCount)
{
}

INLINE void C_GCMDevice::SetPixelShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount)
{
}

INLINE void C_GCMDevice::SetPixelShaderEmbeddedConstantF(const uint32_t dstOffset, const float* pData, const uint32_t sizeInWords, const uint32_t location)
{
    void* pointer;
	GCM_NAMESPACE::cellGcmSetTransferLocation(location);
    GCM_NAMESPACE::cellGcmSetInlineTransferPointer(dstOffset, sizeInWords, &pointer);
    float* fp = (float*)pointer;
    float* src = (float*)pData;
    for(uint32_t j=0; j<sizeInWords; j++)
    {
        *fp = cellGcmSwap16Float32(*src);
        fp++; src++;
    }
}

INLINE void C_GCMDevice::SetPixelShaderParameterF(const C_PixelShaderParameter parameter, const F32* pData)
{
	m_Display2.GetContextNoCheckNoTrack().SetPixelShaderParamF(parameter, pData);
}

INLINE void C_GCMDevice::SetPixelShaderParameterArrayF(const C_PixelShaderParameterArray& parameterArray, const F32* pData)
{
	for(S32 i=0; i<parameterArray.Size(); i++)
	{
		SetPixelShaderParameterF(parameterArray[i], &pData[i*4]);
	}
}

INLINE void C_GCMDevice::SetPixelShaderParameters(const C_PixelShader* pPixelShader)
{
	if(m_pPixelShader == pPixelShader)
	{
		GCM_NAMESPACE::cellGcmSetUpdateFragmentProgramParameter(gCellGcmCurrentContext, pPixelShader->GetOffset());
	}
}

INLINE void C_GCMDevice::SetPresentInterval(E_PresentInterval presentInterval)
{
	if(presentInterval == E_PresentInterval_Default)
		presentInterval = m_PresentIntervalDefault;

	m_PresentInterval = presentInterval;
}

INLINE void C_GCMDevice::SetSlopeScaledBias(const F32 slopeScaledBias)
{
}

INLINE void C_GCMDevice::SetStencilState(const C_StencilState& stencilState)
{
	GCM_NAMESPACE::cellGcmSetStencilTestEnable(
		stencilState.m_StencilEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE);
	GCM_NAMESPACE::cellGcmSetStencilFunc(
		GCMCmpFunc(stencilState.m_StencilFunc),
		stencilState.m_StencilRef,
		stencilState.m_StencilMask);
	GCM_NAMESPACE::cellGcmSetStencilMask(
		stencilState.m_StencilWriteMask);
	GCM_NAMESPACE::cellGcmSetStencilOp(
		GCMStencilOp(stencilState.m_StencilFail),
		GCMStencilOp(stencilState.m_StencilDepthFail),
		GCMStencilOp(stencilState.m_StencilDepthPass));

	m_StencilEnable = stencilState.m_StencilEnable;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetStencilEnable(E_Bool stencilEnable)
{
	if(stencilEnable == E_Bool_Default)
		stencilEnable = m_StencilEnableDefault;

	GCM_NAMESPACE::cellGcmSetStencilTestEnable(stencilEnable == E_Bool_True ? CELL_GCM_TRUE : CELL_GCM_FALSE);
	m_StencilEnable = stencilEnable;
}

INLINE void C_GCMDevice::SetStencilFail(const E_StencilOp stencilFail)
{
}

INLINE void C_GCMDevice::SetStencilDepthFail(const E_StencilOp stencilDepthFail)
{
}

INLINE void C_GCMDevice::SetStencilPass(const E_StencilOp stencilPass)
{
}

INLINE void C_GCMDevice::SetStencilFunc(const E_CmpFunc stencilFunc)
{
}

INLINE void C_GCMDevice::SetStencilRef(const S32 stencilRef)
{
}

INLINE void C_GCMDevice::SetStencilMask(const U32 stencilMask)
{
}

INLINE void C_GCMDevice::SetStencilWriteMask(const U32 stencilWriteMask)
{
}

INLINE void C_GCMDevice::SetStencilCcwEnable(const E_Bool stencilCcwEnable)
{
}

INLINE void C_GCMDevice::SetStencilCcwFail(const E_StencilOp stencilFail)
{
}

INLINE void C_GCMDevice::SetStencilCcwDepthFail(const E_StencilOp stencilDepthFail)
{
}

INLINE void C_GCMDevice::SetStencilCcwPass(const E_StencilOp stencilPass)
{
}

INLINE void C_GCMDevice::SetStencilCcwFunc(const E_CmpFunc stencilFunc)
{
}

INLINE void C_GCMDevice::SetStencilCcwRef(const S32 stencilRef)
{
}

INLINE void C_GCMDevice::SetStencilCcwMask(const U32 stencilMask)
{
}

INLINE void C_GCMDevice::SetStencilCcwWriteMask(const U32 stencilWriteMask)
{
}

INLINE void C_GCMDevice::SetTexture(const U32 sampler, const C_Texture* pTexture)
{
	BIZ_ASSERT(HaveThreadOwnership());

	BIZ_ASSERT(sampler < E_Sampler_Count);
	
	if(pTexture != m_Sampler[sampler].m_pTexture)
	{
		if(pTexture && pTexture->IsCreated())
		{
			SetTextureUncached(sampler, &pTexture->m_gcmTexture);

			const U16 minLod = 0;
			const U16 maxLod = (pTexture->m_gcmTexture.mipmap-1)<<8;

			#ifdef DEVICE_DEBUG
			const U8 maxAniso = g_debugDisplayTextureMaxAniso >= 0 ? (U8)g_debugDisplayTextureMaxAniso : (U8)m_MaxAniso;
			#else
			const U8 maxAniso = (U8)m_MaxAniso;
			#endif

			const C_TextureControl textureControl(
				CELL_GCM_TRUE,
				minLod,
				maxLod,
				pTexture->GetFormat() == Display2::E_TextureFormat_R32F ? CELL_GCM_TEXTURE_MAX_ANISO_1 : maxAniso);

			SetTextureControlUncached(sampler, textureControl);

			m_Sampler[sampler].m_pTexture = pTexture;
		}
		else
		{
			SetTextureUncached(sampler, &m_NullTexture.m_gcmTexture);

			const C_TextureControl textureControl(
				CELL_GCM_FALSE,
				0,
				1,
				CELL_GCM_TEXTURE_MAX_ANISO_1);

			SetTextureControlCached(sampler, textureControl);

			m_Sampler[sampler].m_pTexture = &m_NullTexture;
		}
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetTextureAddressU(const U32 sampler, E_TextureAddress addressU)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(addressU == E_TextureAddress_Default)
		addressU = m_Sampler[sampler].m_AddressUDefault;
	
	C_TextureAddress& textureAddress = GetTextureAddress(sampler);
	const U8 gcmAddressU = GCMTextureAddress(addressU);
	if(	m_DirtyFlags.m_Samplers[sampler].m_AddressU ||
		gcmAddressU != textureAddress.GetWrapS())
	{
		textureAddress.SetWrapS(gcmAddressU);

		SetTextureAddressUncached(sampler, textureAddress);

		m_DirtyFlags.m_Samplers[sampler].m_AddressU = false;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetTextureAddressV(const U32 sampler, E_TextureAddress addressV)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(addressV == E_TextureAddress_Default)
		addressV = m_Sampler[sampler].m_AddressVDefault;
	
	C_TextureAddress& textureAddress = GetTextureAddress(sampler);
	const U8 gcmAddressV = GCMTextureAddress(addressV);
	if(	m_DirtyFlags.m_Samplers[sampler].m_AddressV ||
		gcmAddressV != textureAddress.GetWrapT())
	{
		textureAddress.SetWrapT(gcmAddressV);

		SetTextureAddressUncached(sampler, textureAddress);

		m_DirtyFlags.m_Samplers[sampler].m_AddressV = false;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetTextureAddressW(const U32 sampler, E_TextureAddress addressW)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);
	
	if(addressW == E_TextureAddress_Default)
		addressW = m_Sampler[sampler].m_AddressWDefault;
	
	C_TextureAddress& textureAddress = GetTextureAddress(sampler);
	const U8 gcmAddressW = GCMTextureAddress(addressW);
	if(	m_DirtyFlags.m_Samplers[sampler].m_AddressW ||
		gcmAddressW != textureAddress.GetWrapR())
	{
		textureAddress.SetWrapR(gcmAddressW);

		SetTextureAddressUncached(sampler, textureAddress);

		m_DirtyFlags.m_Samplers[sampler].m_AddressW = false;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetTextureAddressUVW(const U32 sampler, E_TextureAddress addressU, E_TextureAddress addressV, E_TextureAddress addressW)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(addressU == E_TextureAddress_Default)
		addressU = m_Sampler[sampler].m_AddressUDefault;
	if(addressV == E_TextureAddress_Default)
		addressV = m_Sampler[sampler].m_AddressVDefault;
	if(addressW == E_TextureAddress_Default)
		addressW = m_Sampler[sampler].m_AddressWDefault;
	
	C_TextureAddress& oldTextureAddress = GetTextureAddress(sampler);
	C_TextureAddress newTextureAddress = oldTextureAddress;
	newTextureAddress.SetAddressU(addressU);
	newTextureAddress.SetAddressV(addressV);
	newTextureAddress.SetAddressW(addressW);
	if(	m_DirtyFlags.m_Samplers[sampler].m_AddressU ||
		m_DirtyFlags.m_Samplers[sampler].m_AddressV ||
		m_DirtyFlags.m_Samplers[sampler].m_AddressW ||
		newTextureAddress != oldTextureAddress)
	{
		oldTextureAddress = newTextureAddress;

		SetTextureAddressUncached(sampler, oldTextureAddress);

		m_DirtyFlags.m_Samplers[sampler].m_AddressU = false;
		m_DirtyFlags.m_Samplers[sampler].m_AddressV = false;
		m_DirtyFlags.m_Samplers[sampler].m_AddressW = false;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetTextureFilterMag(const U32 sampler, E_TextureFilter filterMag)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(filterMag == E_TextureFilter_Default)
		filterMag = m_Sampler[sampler].m_FilterMagDefault;
	
	C_TextureFilter& textureFilter = GetTextureFilter(sampler);
	const U8 gcmFilterMag = GCMTextureFilterMag(filterMag);
	if(	m_DirtyFlags.m_Samplers[sampler].m_FilterMag ||
		gcmFilterMag != textureFilter.GetMag())
	{
		textureFilter.SetMag(gcmFilterMag);

		SetTextureFilterUncached(sampler, textureFilter);

		m_DirtyFlags.m_Samplers[sampler].m_FilterMag = false;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetTextureFilterMin(const U32 sampler, E_TextureFilter filterMin)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(filterMin == E_TextureFilter_Default)
		filterMin = m_Sampler[sampler].m_FilterMinDefault;
	
	m_Sampler[sampler].m_FilterMin = filterMin;
	C_TextureFilter& textureFilter = GetTextureFilter(sampler);
	const U8 gcmFilterMin = GCMTextureFilterMin(filterMin, m_Sampler[sampler].m_FilterMip);
	if(	m_DirtyFlags.m_Samplers[sampler].m_FilterMin ||
		gcmFilterMin != textureFilter.GetMin())
	{
		textureFilter.SetMin(gcmFilterMin);

		SetTextureFilterUncached(sampler, textureFilter);

		m_DirtyFlags.m_Samplers[sampler].m_FilterMin = false;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetTextureFilterMip(const U32 sampler, E_TextureFilter filterMip)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(filterMip == E_TextureFilter_Default)
		filterMip = m_Sampler[sampler].m_FilterMipDefault;
	
	m_Sampler[sampler].m_FilterMip = filterMip;
	C_TextureFilter& textureFilter = GetTextureFilter(sampler);
	const U8 gcmFilterMin = GCMTextureFilterMin(m_Sampler[sampler].m_FilterMin, m_Sampler[sampler].m_FilterMip);
	if(	m_DirtyFlags.m_Samplers[sampler].m_FilterMip ||
		gcmFilterMin != textureFilter.GetMin())
	{
		textureFilter.SetMin(gcmFilterMin);

		SetTextureFilterUncached(sampler, textureFilter);

		m_DirtyFlags.m_Samplers[sampler].m_FilterMip = false;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetTextureFilterMagMinMip(const U32 sampler, E_TextureFilter filterMag, E_TextureFilter filterMin, E_TextureFilter filterMip)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(filterMag == E_TextureFilter_Default)
		filterMag = m_Sampler[sampler].m_FilterMagDefault;
	if(filterMin == E_TextureFilter_Default)
		filterMin = m_Sampler[sampler].m_FilterMinDefault;
	if(filterMip == E_TextureFilter_Default)
		filterMip = m_Sampler[sampler].m_FilterMipDefault;

	m_Sampler[sampler].m_FilterMin = filterMin;
	m_Sampler[sampler].m_FilterMip = filterMip;

	C_TextureFilter& oldTextureFilter = GetTextureFilter(sampler);
	C_TextureFilter newTextureFilter = oldTextureFilter;
	newTextureFilter.SetFilterMag(filterMag);
	newTextureFilter.SetFilterMin(filterMin, filterMip);
	if(	m_DirtyFlags.m_Samplers[sampler].m_FilterMag ||
		m_DirtyFlags.m_Samplers[sampler].m_FilterMin ||
		m_DirtyFlags.m_Samplers[sampler].m_FilterMip ||
		newTextureFilter != oldTextureFilter)
	{
		oldTextureFilter = newTextureFilter;

		SetTextureFilterUncached(sampler, oldTextureFilter);

		m_DirtyFlags.m_Samplers[sampler].m_FilterMag = false;
		m_DirtyFlags.m_Samplers[sampler].m_FilterMin = false;
		m_DirtyFlags.m_Samplers[sampler].m_FilterMip = false;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetTextureBorderColour(const U32 sampler, const U32 borderColour)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);
	
	C_TextureSampler& textureSampler = GetTextureSampler(sampler);
	if(	m_DirtyFlags.m_Samplers[sampler].m_BorderColour ||
		borderColour != textureSampler.m_BorderColour)
	{
		textureSampler.m_BorderColour = borderColour;

		SetTextureBorderColourUncached(sampler, textureSampler.m_BorderColour);

		m_DirtyFlags.m_Samplers[sampler].m_BorderColour = false;
	}
}

INLINE void C_GCMDevice::SetTextureLodBias(const U32 sampler, const S8 lodBias)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);
	
	C_TextureFilter& textureFilter = GetTextureFilter(sampler);
	const S16 gcmLodBias = GCMLodBias(lodBias);
	if(	m_DirtyFlags.m_Samplers[sampler].m_LodBias ||
		gcmLodBias != textureFilter.GetBias())
	{
		textureFilter.SetBias(gcmLodBias);

		SetTextureFilterUncached(sampler, textureFilter);

		m_DirtyFlags.m_Samplers[sampler].m_LodBias = false;
	}
}

INLINE void C_GCMDevice::SetTextureGamma(const U32 sampler, const E_Bool gamma)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);
	
	const U8 gcmGamma = GCMGamma(gamma);
	C_TextureAddress& textureAddress = GetTextureAddress(sampler);
	if(	m_DirtyFlags.m_Samplers[sampler].m_Gamma ||
		gcmGamma != textureAddress.GetGamma())
	{
		textureAddress.SetGamma(gcmGamma);

		SetTextureAddressUncached(sampler, textureAddress);

		m_DirtyFlags.m_Samplers[sampler].m_Gamma = false;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetVertexTexture(const U32 sampler, const C_Texture* pTexture)
{
	BIZ_ASSERT(HaveThreadOwnership());

	BIZ_ASSERT(sampler < E_VertexSampler_Count);
	
	if(pTexture != m_VertexSampler[sampler].m_pTexture)
	{
		GCM_NAMESPACE::cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);

		if(pTexture && pTexture->IsCreated())
		{
			CheckGCMVertexTexture(&pTexture->m_gcmTexture);
			GCM_NAMESPACE::cellGcmSetVertexTexture(sampler, &pTexture->m_gcmTexture);

			const U16 minLod = 0;
			const U16 maxLod = (pTexture->m_gcmTexture.mipmap-1)<<8;

			GCM_NAMESPACE::cellGcmSetVertexTextureControl(
				sampler,
				CELL_GCM_TRUE,
				minLod,
				maxLod);

			m_VertexSampler[sampler].m_pTexture = pTexture;
		}
		else
		{
			BIZ_ASSERT(&m_NullTexture.m_gcmTexture);
			CheckGCMVertexTexture(&m_NullTexture.m_gcmTexture);
			GCM_NAMESPACE::cellGcmSetVertexTexture(sampler, &m_NullTexture.m_gcmTexture);

			GCM_NAMESPACE::cellGcmSetVertexTextureControl(
				sampler,
				CELL_GCM_FALSE,
				0,
				1);

			m_VertexSampler[sampler].m_pTexture = &m_NullTexture;
		}
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetVertexTextureAddressU(const U32 sampler, E_TextureAddress addressU)
{
	BIZ_ASSERT(sampler < E_VertexSampler_Count);

	if(addressU == E_TextureAddress_Default)
		addressU = m_VertexSampler[sampler].m_AddressUDefault;

	if(m_VertexSampler[sampler].m_AddressU == addressU)
		return;

	const U8 gcmAddressU = GCMTextureAddress(addressU);
	const U8 gcmAddressV = GCMTextureAddress(m_VertexSampler[sampler].m_AddressV);

	GCM_NAMESPACE::cellGcmSetVertexTextureAddress(
		sampler,
		gcmAddressU,
		gcmAddressV);

	m_VertexSampler[sampler].m_AddressU = addressU;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetVertexTextureAddressV(const U32 sampler, E_TextureAddress addressV)
{
	BIZ_ASSERT(sampler < E_VertexSampler_Count);

	if(addressV == E_TextureAddress_Default)
		addressV = m_VertexSampler[sampler].m_AddressVDefault;

	if(m_VertexSampler[sampler].m_AddressV == addressV)
		return;

	const U8 gcmAddressU = GCMTextureAddress(m_VertexSampler[sampler].m_AddressU);
	const U8 gcmAddressV = GCMTextureAddress(addressV);

	GCM_NAMESPACE::cellGcmSetVertexTextureAddress(
		sampler,
		gcmAddressU,
		gcmAddressV);

	m_VertexSampler[sampler].m_AddressV = addressV;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetVertexTextureAddressW(const U32 sampler, E_TextureAddress addressW)
{
}

INLINE void C_GCMDevice::SetVertexTextureAddressUVW(const U32 sampler, E_TextureAddress addressU, E_TextureAddress addressV, E_TextureAddress addressW)
{
	BIZ_ASSERT(sampler < E_VertexSampler_Count);

	if(addressU == E_TextureAddress_Default)
		addressU = m_VertexSampler[sampler].m_AddressUDefault;
	if(addressV == E_TextureAddress_Default)
		addressV = m_VertexSampler[sampler].m_AddressVDefault;

	if(m_VertexSampler[sampler].m_AddressU == addressU && m_VertexSampler[sampler].m_AddressV == addressV)
		return;

	const U8 gcmAddressU = GCMTextureAddress(addressU);
	const U8 gcmAddressV = GCMTextureAddress(addressV);

	GCM_NAMESPACE::cellGcmSetVertexTextureAddress(
		sampler,
		gcmAddressU,
		gcmAddressV);

	m_VertexSampler[sampler].m_AddressU = addressU;
	m_VertexSampler[sampler].m_AddressV = addressV;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetVertexTextureFilterMag(const U32 sampler, E_TextureFilter filterMag)
{
}

INLINE void C_GCMDevice::SetVertexTextureFilterMin(const U32 sampler, E_TextureFilter filterMin)
{
}

INLINE void C_GCMDevice::SetVertexTextureFilterMip(const U32 sampler, E_TextureFilter filterMip)
{
}

INLINE void C_GCMDevice::SetVertexTextureFilterMagMinMip(const U32 sampler, E_TextureFilter filterMag, E_TextureFilter filterMin, E_TextureFilter filterMip)
{
}

INLINE void C_GCMDevice::SetVertexTextureBorderColour(const U32 sampler, const U32 borderColour)
{
	BIZ_ASSERT(sampler < E_VertexSampler_Count);

	if(m_VertexSampler[sampler].m_BorderColour == borderColour)
		return;

	GCM_NAMESPACE::cellGcmSetVertexTextureBorderColor(
		sampler,
		borderColour);

	m_VertexSampler[sampler].m_BorderColour = borderColour;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetVertexTextureLodBias(const U32 sampler, const S8 lodBias)
{
	BIZ_ASSERT(sampler < E_VertexSampler_Count);

	if(m_VertexSampler[sampler].m_LodBias == lodBias)
		return;
	
	const S16 gcmLodBias = GCMLodBias(lodBias);

	GCM_NAMESPACE::cellGcmSetVertexTextureFilter(
		sampler,
		gcmLodBias);

	m_VertexSampler[sampler].m_LodBias = lodBias;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetVertexTextureGamma(const U32 sampler, const E_Bool gamma)
{
}

INLINE void C_GCMDevice::SetVertexBuffer(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride)
{
	BIZ_ASSERT(HaveThreadOwnership());

	BIZ_ASSERT(stream < E_Stream_Count);

	if(pVertexBuffer != m_Stream[stream].m_pVertexBuffer || stride != m_Stream[stream].m_VertexStride  || offset != m_Stream[stream].m_VertexOffset)
	{
		if(!m_SetVertexDeclaration)
			m_SetVertexDeclaration = true;

		m_Stream[stream].m_pVertexBuffer = pVertexBuffer;
		m_Stream[stream].m_VertexStride = stride;
		m_Stream[stream].m_VertexOffset = offset;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetVertexDeclaration(const C_VertexDeclaration* pVertexDeclaration)
{
	BIZ_ASSERT(HaveThreadOwnership());

	if(!m_SetVertexDeclaration)
		m_SetVertexDeclaration = true;

	m_pVertexDeclaration = pVertexDeclaration;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetVertexShaderConstantB(const U32 startRegister, const BOOL* pData, const U32 registerCount)
{
}

INLINE void C_GCMDevice::SetVertexShaderConstantI(const U32 startRegister, const S32* pData, const U32 registerCount)
{
}

INLINE void C_GCMDevice::SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount)
{
	GCM_NAMESPACE::cellGcmSetVertexProgramConstants(startRegister, registerCount * 4, pData);

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetVertexShaderParameterF(const C_VertexShaderParameter startParameter, const F32* pData, const U32 parameterCount)
{
	GCM_NAMESPACE::cellGcmSetVertexProgramConstants(startParameter.m_RegisterIndex, parameterCount * 4, pData);

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetViewport(const C_Viewport& viewport)
{
	m_Viewport = viewport;

	F32 scale[4] =
	{
		(F32)viewport.m_W * 0.5f,
		(F32)viewport.m_H * -0.5f,
		1.f / (viewport.m_MaxZ - viewport.m_MinZ),
		0.f
	};
	F32 offset[4] =
	{
		(F32)viewport.m_X + (F32)viewport.m_W * 0.5f,
		(F32)viewport.m_Y + (F32)viewport.m_H * 0.5f, 
		viewport.m_MinZ / (viewport.m_MaxZ - viewport.m_MinZ),
		0.f
	};
	GCM_NAMESPACE::cellGcmSetViewport(viewport.m_X, viewport.m_Y, viewport.m_W, viewport.m_H, viewport.m_MinZ, viewport.m_MaxZ, scale, offset);

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

INLINE void C_GCMDevice::SetColourDepthStencilSurface()
{
	SetColourSurface();
	SetDepthStencilSurface();
	SetSurface();
}

INLINE void C_GCMDevice::SetColourDepthStencilSurface(const C_Surface* pColourSurface, const C_Surface* pDepthStencilSurface)
{
	SetColourSurface(pColourSurface);
	SetDepthStencilSurface(pDepthStencilSurface);
	SetSurface();
}

INLINE void C_GCMDevice::SetColourSurface()
{
	m_RenderTarget.m_pColourSurface = GetBackBuffer(m_CurrentBackBuffer).GetColourSurface();
}

INLINE void C_GCMDevice::SetDepthStencilSurface()
{
	m_RenderTarget.m_pDepthStencilSurface = GetBackBuffer(m_CurrentBackBuffer).GetDepthStencilSurface();
}

INLINE void C_GCMDevice::SetColourSurface(const C_Surface* pSurface)
{
	m_RenderTarget.m_pColourSurface = pSurface;
}

INLINE void C_GCMDevice::SetColourSurface(const U32 index, const C_Surface* pSurface)
{
	m_RenderTarget.m_pColourSurface = pSurface;
}

INLINE void C_GCMDevice::SetDepthStencilSurface(const C_Surface* pSurface)
{
	m_RenderTarget.m_pDepthStencilSurface = pSurface;
}

INLINE bool C_GCMDevice::CreateColourSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, U8* pAddress, const U32 addressSize)
{
	return CreateSurface(width, height, flags, format, multisample, pParams, pSurface, pAddress, addressSize);
}

INLINE bool C_GCMDevice::CreateDepthStencilSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, U8* pAddress, const U32 addressSize)
{
	return CreateSurface(width, height, flags, format, multisample, pParams, pSurface, pAddress, addressSize);
}

INLINE void C_GCMDevice::DestroyColourSurface(C_Surface* pSurface)
{
	DestroySurface(pSurface);
}

INLINE void C_GCMDevice::DestroyDepthStencilSurface(C_Surface* pSurface)
{
	DestroySurface(pSurface);
}

INLINE U32 C_GCMDevice::GetFreeResourceMemoryMemory()
{
	return m_VideoMemory.GetFree();
}

INLINE const C_VertexShader* C_GCMDevice::GetVertexShader()
{
	return m_pVertexShader;
}

INLINE const C_PixelShader* C_GCMDevice::GetPixelShader()
{
	return m_pPixelShader;
}

INLINE void C_GCMDevice::Finish()
{
	GCM_NAMESPACE::cellGcmFinish(m_FinishRef);
	m_FinishRef = m_FinishRef == UINT_MAX ? 0 : m_FinishRef++;
}

#ifdef GCM_DEBUG
INLINE void C_GCMDevice::CheckForErrors()
{
	GCM_NAMESPACE::cellGcmFinish(m_FinishRef);
	m_FinishRef = m_FinishRef == UINT_MAX ? 0 : m_FinishRef++;
}
#endif

INLINE C_MemoryBuffer* C_GCMDevice::GetCmdBufMemory()
{
	return &m_CmdBufMemory;
}

INLINE C_MemoryBuffer* C_GCMDevice::GetVideoMemory()
{
	return &m_VideoMemory;
}

INLINE C_MemoryBuffer* C_GCMDevice::GetSystemMemory()
{
	return &m_SystemMemory;
}

INLINE C_MemoryBuffer* C_GCMDevice::GetMainMemory()
{
	return &m_SystemMemory;
}

INLINE void C_GCMDevice::SetMaxBeginVerticesUsage(const U32 beginVerticesUsage)
{
	m_MaxBeginVerticesUsage = beginVerticesUsage;
}

INLINE void C_GCMDevice::SetUserMaxBeginVerticesUsage(const F32 beginVerticesUsageScale)
{
	BIZ_ASSERT(beginVerticesUsageScale >= 0.0f && beginVerticesUsageScale <= 1.0f);
	m_UserMaxBeginVerticesUsage = (U32)(((F32)m_MaxBeginVerticesUsage)*beginVerticesUsageScale);
	m_UserBeginVerticesUsage = 0;
}

INLINE void C_GCMDevice::SetScissorTest(const E_Bool scissorTestEnable)
{
	if(scissorTestEnable == E_Bool_False)
		GCM_NAMESPACE::cellGcmSetScissor(0, 0, 4096, 4096);
}

INLINE void C_GCMDevice::SetScissorRect(const U32 x, const U32 y, const U32 w, const U32 h)
{
	GCM_NAMESPACE::cellGcmSetScissor(x, y, w, h);
}

INLINE const bool C_GCMDevice::BeginVerticesLocked() const
{
	return m_BeginVerticesLocked;
}

INLINE C_TextureSampler& C_GCMDevice::GetTextureSampler(const U32 sampler)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);
	return m_TextureSampler[sampler];
}

INLINE C_TextureAddress& C_GCMDevice::GetTextureAddress(const U32 sampler)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);
	return m_TextureSampler[sampler].m_Address;
}

INLINE C_TextureControl& C_GCMDevice::GetTextureControl(const U32 sampler)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);
	return m_TextureSampler[sampler].m_Control;
}

INLINE C_TextureFilter& C_GCMDevice::GetTextureFilter(const U32 sampler)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);
	return m_TextureSampler[sampler].m_Filter;
}

INLINE void C_GCMDevice::SetTextureCached(const U32 sampler, const C_Texture* pTexture)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(pTexture == m_Sampler[sampler].m_pTexture)
		return;

	SetTextureUncached(sampler, &pTexture->m_gcmTexture);

	m_Sampler[sampler].m_pTexture = pTexture;
}

INLINE void C_GCMDevice::SetTextureAddressCached(const U32 sampler, const C_TextureAddress& textureAddress)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(textureAddress == m_TextureSampler[sampler].m_Address)
		return;

	SetTextureAddressUncached(sampler, textureAddress);

	m_TextureSampler[sampler].m_Address = textureAddress;
}

INLINE void C_GCMDevice::SetTextureControlCached(const U32 sampler, const C_TextureControl& textureControl)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(textureControl == m_TextureSampler[sampler].m_Control)
		return;

	SetTextureControlUncached(sampler, textureControl);

	m_TextureSampler[sampler].m_Control = textureControl;
}

INLINE void C_GCMDevice::SetTextureFilterCached(const U32 sampler, const C_TextureFilter& textureFilter)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if(textureFilter == m_TextureSampler[sampler].m_Filter)
		return;

	SetTextureFilterUncached(sampler, textureFilter);

	m_TextureSampler[sampler].m_Filter = textureFilter;
}

INLINE void C_GCMDevice::SetTextureUncached(const U32 sampler, const CellGcmTexture* pgcmTexture)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);
	
	GCM_NAMESPACE::cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);

	BIZ_ASSERT(pgcmTexture);
	CheckGCMTexture(pgcmTexture);
	GCM_NAMESPACE::cellGcmSetTexture(sampler, pgcmTexture);
}

INLINE void C_GCMDevice::SetTextureAddressUncached(const U32 sampler, const C_TextureAddress& textureAddress)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	GCM_NAMESPACE::cellGcmSetTextureAddress(
		sampler,
		textureAddress.m_wraps,
		textureAddress.m_wrapt,
		textureAddress.m_wrapr,
		textureAddress.m_unsignedRemap,
		textureAddress.m_zfunc,
		textureAddress.m_gamma);
}

INLINE void C_GCMDevice::SetTextureControlUncached(const U32 sampler, const C_TextureControl& textureControl)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	GCM_NAMESPACE::cellGcmSetTextureControl(
		sampler,
		textureControl.m_enable,
		textureControl.m_minLod,
		textureControl.m_maxLod,
		textureControl.m_maxAniso);
}

INLINE void C_GCMDevice::SetTextureFilterUncached(const U32 sampler, const C_TextureFilter& textureFilter)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	GCM_NAMESPACE::cellGcmSetTextureFilter(
		sampler,
		textureFilter.m_bias,
		textureFilter.m_min,
		textureFilter.m_mag,
		textureFilter.m_conv);
}

INLINE void C_GCMDevice::SetTextureBorderColourUncached(const U32 sampler, const U32 borderColour)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	GCM_NAMESPACE::cellGcmSetTextureBorderColor(
		sampler,
		borderColour);
}

INLINE const U32 C_GCMDevice::GetVertexDeclarationCount() const
{
	return m_Display2.GetVertexDeclarationCount();
}

INLINE const U32 C_GCMDevice::GetMaxAniso() const
{
	return m_MaxAniso;
}

INLINE void C_GCMDevice::SetMaxAniso(const U32 maxAniso)
{
	m_MaxAniso = maxAniso;
}

INLINE C_Texture2D *C_GCMDevice::GetNullTexture()
{
	return &m_NullTexture;
}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_INL
