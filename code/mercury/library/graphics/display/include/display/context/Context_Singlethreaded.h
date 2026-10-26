#ifndef __DISPLAY_CONTEXT_SINGLETHREADED_H__
#define __DISPLAY_CONTEXT_SINGLETHREADED_H__

#include "display.h"
#include "Display/Device/DeviceContext.h"

namespace Display
{

DECLARE_CONTEXT(Context, C_Context);

class C_Device;

/// thread-safe rendering context
class C_Context : public I_Context
{
public:
	/// constructor
	C_Context();

	/// return new instance of this context
	I_Context *New() { return new C_Context; }

	/// initialisation function
	void Initialise();

	/// clone state of another context
	void CloneState(const I_Context *pContext) {}

	/// invalidate the state of this context
	void InvalidateState() {}

	/// flush the draw calls in this context to the main command buffer
	void Flush() {}

	/// stop rendering to the command buffer
	void BeginCommandBuffer(Display::C_CommandBuffer *pCommandBuffer) {}

	/// stop rendering to the command buffer
	void EndCommandBuffer() {}

	/// set vertex declaration
	void SetVertexDeclaration(const C_VertexDeclaration* pVertexDeclaration);

	/// set vertex buffer
	void SetVertexBuffer(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride);

	/// set index buffer
	void SetIndexBuffer(const C_IndexBuffer* pIndexBuffer);

	/// draw primtive
	void DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount);

	// set alpha test function
	void SetAlphaTestFunc(const E_CmpFunc alphaTestFunc);

	/// set blend high precision enable
	void SetBlendHighPrecisionEnable(const E_Bool blendHighPrecisionEnable);

	/// set blend enable
	void SetBlendEnable(const E_Bool blendEnable);

	/// set cull mode
	void SetCullMode(const E_CullMode eCullMode);

	/// set depthwrite enable
	void SetDepthWriteEnable(const E_Bool depthWriteEnable);
	
	/// set depth func
	void SetDepthFunc(const E_CmpFunc depthFunc);

	/// set wireframe / solid
	void SetFillMode(E_FillMode fillMode);

	/// change the current vertex shader
	void SetVertexShader(const Display::C_VertexShader *pShader);

	/// change the current pixel shader
	void SetPixelShader(const Display::C_PixelShader *pShader);

	/// set renderstate
	void SetRenderState( Display::E_Blend srcBlend, Display::E_Blend destBlend, Display::E_BlendOp blendOp,
		Display::E_Bool alphaToMaskEnable, Display::E_Bool alphaTestEnable, U32 alphaRef);

	/// set pixelshader parameter
	void SetPixelShaderParameterF(Display::C_PixelShaderParameter handle, const F32 *src);

	/// commit pixelshader parameters
	void SetPixelShaderParameters(Display::C_PixelShader *pShader);

	/// set a pixel shader constant register
	void SetPixelShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);

	/// set a vertex shader constant register
	void SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);

	/// set a texture. this function takes a pointer to a GcmTexture or a D3DTexture
	void SetTexture(const U32 sampler, const void* pTexture);

	/// set a vertex texture. this function takes a pointer to a GcmTexture or a D3DTexture
	void SetVertexTexture(const U32 sampler, const void* pTexture);

	/// set stencil states
	void SetStencilState(const Display::C_StencilState& stencilState);

	/// set colour write enable
	void SetColourWriteEnable(const U32 flags);

	/// sets predication for tiling
	void SetPredication(const U32 predication) {}

	/// set sampler states
	void SetSamplerState(const U32 sampler,
		const E_TextureFilter filterMag, E_TextureFilter filterMin, E_TextureFilter filterMip,
		const E_TextureAddress addressU, const E_TextureAddress addressV, const E_TextureAddress addressW,
		E_Bool gamma, U8 iMaxAniso, S8 MipMapLodBias);

	/// set vertex sampler states
	void SetVertexSamplerState(const U32 sampler,
		const E_TextureFilter filterMag, E_TextureFilter filterMin, E_TextureFilter filterMip,
		const E_TextureAddress addressU, const E_TextureAddress addressV, const E_TextureAddress addressW,
		E_Bool gamma, U8 iMaxAniso);

#if !PLATFORM_SPU
	const C_View* GetView() const { return Display::Device()->GetView(); }
#endif

public:
	Display::C_Device *m_pDevice;
#if PLATFORM_PS3
	int m_iAniso[16];
#endif
};

/// keeps track of contexts, automatically allocating them and freeing them up when the graphics card has finished with them
class C_ContextManager
{
public:
	/// allocate a new context
	/// anything that is rendered into the context will be inserted to the commandbuffer at the point when the context was created
	C_Context *CreateContext(bool bMultiThreaded) { assert(!bMultiThreaded); return new C_Context; }

	/// mark the end of a frame. this function is called when the graphics card has finished rendering
	/// this frees up any commandbuffers that were using to render the previous frame
	void EndFrame() {}
};

extern C_ContextManager g_ContextManager;

}

#endif
