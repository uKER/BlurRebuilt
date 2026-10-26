#ifndef __DISPLAY_CONTEXT_BASE_H__
#define __DISPLAY_CONTEXT_BASE_H__

#include "Display.h"
#include "Display/Device/DeviceContext.h"
#include "Display/context/SamplerUsageFlags.h"
#include "Display/context/SamplerState.h"

namespace Display
{

class C_ContextBase : public I_Context
{
public:

	C_ContextBase();
	~C_ContextBase();

#if !PLATFORM_SPU
	void SetView(const C_View& view) { *m_pView = view; }
	void SetColourDepthStencilSurface(const Display::C_Surface* pColourSurface, const Display::C_Surface* pDepthStencilSurface) { }
	void SetColourDepthStencilTexture(const Display::C_Texture2D* pColourTexture, const Display::C_Texture2D* pDepthStencilTexture) { m_pColourTexture = pColourTexture; m_pDepthStencilTexture = pDepthStencilTexture; }
	void SetWindowIndex(const S32 windowIndex) {};
	void SetMultisample(const E_Multisample multisample) { m_Multisample = multisample; }

	const C_View*	GetView() const { return m_pView; }
	const Display::C_Texture2D*	GetColourTexture() const { return m_pColourTexture; }
	const Display::C_Texture2D*	GetDepthStencilTexture() const { return m_pDepthStencilTexture; }
	#if PLATFORM_PC
	// [Blur2PC] retail Blur PC (Blur.exe 1.2, context +0x48): the display window's stencil mask texture
	void SetStencilTexture(const Display::C_Texture2D* pStencilTexture) { m_pStencilTexture = pStencilTexture; }
	const Display::C_Texture2D*	GetStencilTexture() const { return m_pStencilTexture; }
	#endif
#endif

	void SetVertexDeclaration(const Display2::C_VertexDeclaration* pVertexDeclaration);
	void SetVertexBuffer(const U32 stream, const Display2::C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride);
	void SetIndexBuffer(const Display2::C_IndexBuffer* pIndexBuffer);
	void DrawPrimitive(const Display2::E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount);
	void DrawIndexedPrimitive(const Display2::E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount);

	void SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);
	void SetVertexShader(const Display2::C_VertexShader* pShader);
	
	void SetPixelShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);
	void SetPixelShader(const Display2::C_PixelShader* pShader);

	void SetPixelShaderParameterF(C_PixelShaderParameter parameter, const F32* pData);
	void SetPixelShaderParameterArrayF(const C_PixelShaderParameterArray& parameterArray, const F32* pData);
	void SetPixelShaderParameters(const C_PixelShader* pPixelShader);
	
	void SetBlendEnable(const E_Bool blendEnable);
	void SetColourWriteEnable(const U32 flags);
	void SetCullMode(const E_CullMode cullMode);
	void SetDepthEnable(const E_Bool depthEnable);
	void SetDepthWriteEnable(const E_Bool depthWriteEnable);
	void SetDepthFunc(const E_CmpFunc depthFunc);
	void SetFillMode(E_FillMode fillMode);
	
	void SetStencilState(const C_StencilState& stencilState);

	void SetBlendMode(const E_BlendMode blendMode);
	
	void SetBlendState(const E_Blend blendColourSrc, const E_Blend blendColourDst, const E_BlendOp blendColourOp);
	void SetBlendState(const E_Blend blendColourSrc, const E_Blend blendColourDst, const E_BlendOp blendColourOp,
						const E_Blend blendAlphaSrc, const E_Blend blendAlphaDst, const E_BlendOp blendAlphaOp);

	void SetAlphaTestState(const E_Bool alphaTestEnable, const U32 alphaRef);
	void SetAlphaToMaskEnable(const E_Bool alphaToMaskEnable);

	#if !PLATFORM_PPU_SPU
	void SetSamplerState(const U32 sampler, const C_SamplerState *pState);
	void SetVertexSamplerState(const U32 sampler, const C_SamplerState *pState);
	#endif

	/// set texture sampler usage flags
	void SetSamplerUsageFlags(const C_SamplerUsageFlags *pFlags) {}

	void SetPredication(const U32 predication) {}
	
	/// sets point sprite enable
	void SetPointSpriteEnable(const E_Bool pointSpriteEnable);

public:

	E_CullMode														m_CullMode;
	E_Multisample													m_Multisample;

	const Display::C_Texture2D*										m_pColourTexture;
	const Display::C_Texture2D*										m_pDepthStencilTexture;
	#if PLATFORM_PC
	const Display::C_Texture2D*										m_pStencilTexture;	// [Blur2PC] retail +0x48
	#endif
	const Display2::C_VertexDeclaration*							m_pVertexDeclaration;
	const Display2::C_IndexBuffer*									m_pIndexBuffer;

	#if PLATFORM_PPU_SPU
	Display2::C_CommandList<Display2::C_ContextNoCheckNoTrack>		m_Context;
	#else
	Display2::C_ContextNoCheckNoTrack								m_Context;
	#endif

	C_View*															m_pView;
};

} // namespace Display

#endif // __DISPLAY_CONTEXT_BASE_H__
