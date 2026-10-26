#ifndef __DISPLAY_CONTEXT_PS3_H__
#define __DISPLAY_CONTEXT_PS3_H__

#include "Display.h"
#include "Display/Device/DeviceContext.h"
#include "Display/context/SamplerUsageFlags.h"
#include "Display/Context/Context_Base.h"

namespace Display
{

class C_RingBuffer;
class C_MemorySet;

namespace GCM
{
class C_CommandBuffer;
}

DECLARE_CONTEXT(Context, C_Context);

#if !PLATFORM_SPU
class C_Device;
#endif

/// thread-safe rendering context
class C_Context : public C_ContextBase
{
public:
	/// constructor
	C_Context();

	/// destructor
	~C_Context();

	/// return new instance of this context
	I_Context *New()
	{
#if !PLATFORM_SPU
		return new C_Context;
#endif
	}

	/// initialisation function
	void Initialise();

	/// clone state of another context
	void CloneState(const I_Context *pContext);

	/// invalidate the state of this context
	void InvalidateState();

	/// flush the draw calls in this context to the main command buffer
	void Flush();

	/// stop rendering to the command buffer
	void BeginCommandBuffer(Display::C_CommandBuffer *pCommandBuffer);

	/// stop rendering to the command buffer
	void EndCommandBuffer();

	/// set vertex declaration
	void SetVertexDeclaration(const Display2::C_VertexDeclaration* pVertexDeclaration);
	/// set vertex buffer
	void SetVertexBuffer(const U32 stream, const Display2::C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride);
	/// set index buffer
	void SetIndexBuffer(const Display2::C_IndexBuffer* pIndexBuffer);
	/// draw indexed primtive
	void DrawPrimitive(const Display2::E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount);
	/// draw indexed primtive
	void DrawIndexedPrimitive(const Display2::E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount);

	/// begin/end vertices
	void* BeginVertices(const E_PrimitiveType primitiveType, const U32 vertexCount, const U32 vertexStride, const C_VertexDeclaration* pVertexDeclaration);
	void EndVertices();

	// set alpha test function
	void SetAlphaTestFunc(const E_CmpFunc alphaTestFunc);

	/// set blend high precision enable
	void SetBlendHighPrecisionEnable(const E_Bool blendHighPrecisionEnable);

	/// set blend enable
	void SetBlendEnable(const E_Bool blendEnable);

	/// set cull mode
	void SetCullMode(const E_CullMode eCullMode);

	/// set depth enable
	void SetDepthEnable(const E_Bool depthEnable);

	/// set depthwrite enable
	void SetDepthWriteEnable(const E_Bool depthWriteEnable);

	/// set depthwrite enable
	void SetDepthFunc(const E_CmpFunc depthFunc);

	/// set wireframe / solid
	void SetFillMode(E_FillMode fillMode);

	/// change the current vertex shader
	void SetVertexShader2(const Display2::C_VertexShader *pShader);

	/// change the current pixel shader
	void SetPixelShader2(const Display2::C_PixelShader *pShader);

	/// change the current pixel shader
	void SetPixelShader(const Display2::C_PixelShader *pShader, U32 offset);

	/// change the current vertex shader
	void SetVertexShader(const Display::C_VertexShader *pShader);

	/// change the current pixel shader
	void SetPixelShader(const Display::C_PixelShader *pShader);

	/// set alpha state
	void SetAlphaTestState(const E_Bool alphaTestEnable, const U32 alphaRef);
	
	/// set alpha to mask enable
	void SetAlphaToMaskEnable(const E_Bool alphaToMaskEnable);

	/// set pixelshader parameter
	void SetPixelShaderParameterF(C_PixelShaderParameter handle, const F32 *src);

	/// commit pixelshader parameters
	void SetPixelShaderParameters(const C_PixelShader *pShader);

	/// set a pixel shader constant register
	void SetPixelShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);

	/// set a vertex shader constant register
	void SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);

	/// set a texture. this function takes a pointer to a GcmTexture or a D3DTexture
	void SetTexture(const U32 sampler, const void* pTexture);

	/// set a vertex texture. this function takes a pointer to a GcmTexture or a D3DTexture
	void SetVertexTexture(const U32 sampler, const void* pTexture);

	/// set stencil states
	void SetStencilState(const C_StencilState& stencilState);

	/// set colour write enable
	void SetColourWriteEnable(const U32 flags);

	/// set texture sampler usage flags
	void SetSamplerUsageFlags(const C_SamplerUsageFlags *pFlags);

	/// set vertex input attr usage flags
	void SetVtxAttrUsageFlags(const U32 flags);

	/// set sampler states
	void SetSamplerState(const U32 sampler, const C_SamplerState *pState);

	/// set vertex sampler states
	void SetVertexSamplerState(const U32 sampler, const C_SamplerState *pState);

	/// for command buffer callback
	int32_t CommandBufferCallback(CellGcmContextData* pContextData, uint32_t size);

	/// allocate local memory from the graphics ringbuffer
	void *Alloc(U32 cSize, U32 *pOffset = NULL);

	/// allocate main memory from the ringbuffer
	void *AllocMain(U32 cSize, U32 *pOffset = NULL);

	#if USE_GPU_TIMINGS2
	U32 GetTimeStamp(const U32 timeStampKey);
	void SetTimeStamp(const U32 timeStampIndex);
	#endif

	void SetInvalidateVertexCache();

	void SetVertexAttribOutputMask(const U32 mask);

private:

	bool AllocCommandBuffer(Display::C_CommandBuffer* pCommandBuffer, const U32 commandBuffeSize);

	bool CreateCurrentBuffer(const U32 size, const U32 offset, U8* pAddress);

	#if PLATFORM_SPU
	bool CreateScratchBuffer(const U32 size);
	bool CreateJumpToNextBuffer(const U32 size);
	#endif

	void ResetStates();

public:

	Display2::C_ContextData					m_ContextData;
	
	Display::C_CommandBuffer*				m_pCommandBuffer;
	Display::C_RingBuffer*					m_pRingBuffer;
	Display::C_CommandBuffer*				m_pCurrentBuffer;
	
	#if USE_GPU_TIMINGS2
	U32*									m_pFreeTiming;
	U32*									m_pTimingKeys;
	#endif
	
	U32										m_AlphaRef;
	U32										m_VertexElementsBitfield;
	
	int										m_iAniso[16];
	C_SamplerUsageFlags						m_SamplerUsageFlags;
	U32										m_VtxAttrUsageFlags;
	E_PrimitiveType							m_BeginVerticesPrimitive;
	U32										m_BeginVerticesVertexCount;

	Display::C_MemorySet*					m_pMemorySet;

#if PLATFORM_SPU
	Display::C_CommandBuffer*				m_pScratchBuffer;
	Display::C_CommandBuffer*				m_pJumpToNextBuffer;
#else
	C_Device*								m_pDevice;
#endif

	#if TRACK_DISPLAY2_COMMAND_BUFFERS
	U32										m_Callbacks;
	#endif
	
	bool									m_InvalidateVertexCache;
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

} // namespace Display

#endif // __DISPLAY_CONTEXT_PS3_H__
