#include "stdafx.h"
#include "Display/Context/Context.h"
#include "Display/Memory/gcm/GCMRingBuffer.h"
#include "Display/Buffer/GCM/GCMCommandBuffer.h"
#include "Display/Buffer/GCM/GCMDebugCommandBuffer.h"
#include "Display/DisplayToDisplay2.h"
#include <System/Scheduler.h>

#include <Display2/Context/Context.h>
#include <Display2/Debug/Debug.h>

#if !PLATFORM_SPU
#include "display/device/Device.h"
#endif

namespace Display
{

DEFINE_CONTEXT(Context, C_Context);

static const U32 g_CommandBufferSize = 1024*16;
static const U32 g_ScratchBufferSize = 1024*4;

static const int g_iAniso[] = 
{
	CELL_GCM_TEXTURE_MAX_ANISO_1, CELL_GCM_TEXTURE_MAX_ANISO_1, CELL_GCM_TEXTURE_MAX_ANISO_2, CELL_GCM_TEXTURE_MAX_ANISO_2,
	CELL_GCM_TEXTURE_MAX_ANISO_4, CELL_GCM_TEXTURE_MAX_ANISO_4, CELL_GCM_TEXTURE_MAX_ANISO_6, CELL_GCM_TEXTURE_MAX_ANISO_6,
	CELL_GCM_TEXTURE_MAX_ANISO_8, CELL_GCM_TEXTURE_MAX_ANISO_8, CELL_GCM_TEXTURE_MAX_ANISO_10, CELL_GCM_TEXTURE_MAX_ANISO_10,
	CELL_GCM_TEXTURE_MAX_ANISO_12, CELL_GCM_TEXTURE_MAX_ANISO_12, CELL_GCM_TEXTURE_MAX_ANISO_12, CELL_GCM_TEXTURE_MAX_ANISO_12,
	CELL_GCM_TEXTURE_MAX_ANISO_16
};

static Display::C_MemorySet g_UsedMemory;

#if PLATFORM_SPU
static C_Context *g_pContext = NULL;
#else
static __thread C_Context *g_pContext = NULL;
#endif

int32_t ContextRenderCallback(CellGcmContextData* pContextData, uint32_t sizeInWords)
{
	BIZ_ASSERT(g_pContext);

	return g_pContext->CommandBufferCallback(pContextData, sizeInWords);
}

int32_t C_Context::CommandBufferCallback(CellGcmContextData* pContextData, uint32_t sizeInWords)
{
	#if PLATFORM_SPU
	//DebugBreak();
	#endif
	
	const U32 sizeInBytes = sizeInWords * 4;

	BIZ_ASSERT(pContextData);
	BIZ_ASSERT(m_pMemorySet);
	BIZ_ASSERT(m_pRingBuffer);
	BIZ_ASSERT(m_pCurrentBuffer);

	#if PLATFORM_SPU
	BIZ_ASSERT(m_pScratchBuffer);
	BIZ_ASSERT(m_pJumpToNextBuffer);
	BIZ_ASSERT(sizeInBytes < m_pScratchBuffer->m_Size);
	BIZ_ASSERT((U8*)pContextData->begin == m_pScratchBuffer->m_pAddress);
	#else
	BIZ_ASSERT(sizeInBytes < m_pCurrentBuffer->m_Size);
	#endif

	// How much of the context data have we used
	const U32 contextDataUsed = (U32)pContextData->current - (U32)pContextData->begin;

	#if PLATFORM_SPU
	if(contextDataUsed > 0)
	#else
	if(sizeInBytes > 0)
	#endif
	{
		// How much of the current buffer have been used
		#if PLATFORM_SPU
		const U32 currentBufferUsed = m_pCurrentBuffer->m_EndOffset - m_pCurrentBuffer->m_Offset;
		#endif

		#if PLATFORM_SPU
		if((currentBufferUsed + contextDataUsed) > (m_pCurrentBuffer->m_Size - 16))
		#endif
		{
			#if PLATFORM_SPU
			//DebugBreak();
			#endif

			// Alloc a new current buffer
			U8* pNewCurrentBufferAddress = (U8*)m_pRingBuffer->AllocMain(m_pCurrentBuffer->m_Size, m_pMemorySet);
			U32 newCurrentBufferOffset = m_pRingBuffer->GetOffsetFromAddressMain(pNewCurrentBufferAddress);
			
			// Set a jump to the new current buffer
			#if PLATFORM_SPU
			Display2::BuildJumpToNextCommandBuffer(m_pJumpToNextBuffer, newCurrentBufferOffset);
			Spu::Store(m_pCurrentBuffer->GetEndAddress(), m_pJumpToNextBuffer->m_pAddress, m_pJumpToNextBuffer->m_Size);
			#else
			BIZ_ASSERT((U32)pContextData->current <= (U32)pContextData->end);
			cellGcmSetJumpCommandUnsafeInline(pContextData, newCurrentBufferOffset);
			#endif

			// Point the old current buffer to the new one
			m_pCurrentBuffer->m_Offset = newCurrentBufferOffset;
			m_pCurrentBuffer->m_pAddress = pNewCurrentBufferAddress;
			m_pCurrentBuffer->m_EndOffset = m_pCurrentBuffer->m_Offset;

			#if DEBUG_COMMAND_BUFFER
			g_UserDebugCommandBuffer.Realloc(m_pCurrentBuffer->m_Size);
			#endif
		}

		// Setup the context
		#if PLATFORM_SPU
		m_pCurrentBuffer->Flush(pContextData);
		m_pScratchBuffer->Setup(pContextData, ContextRenderCallback, -4);
		#else
		m_pCurrentBuffer->Setup(pContextData, ContextRenderCallback, 12);
		#endif
	}
	#if !PLATFORM_SPU
	else
	{
		m_pCurrentBuffer->m_EndOffset = m_pCurrentBuffer->m_Offset + contextDataUsed;
	}
	#endif
	
	return CELL_OK;
}

C_Context::C_Context()
{
	m_pCommandBuffer			= NULL;
	m_pRingBuffer				= NULL;
	
	m_CullMode					= E_CullMode_None;
	m_Multisample				= E_Multisample_None;

	#if PLATFORM_SPU
	m_pCurrentBuffer			= (Display::C_CommandBuffer*)Spu::Alloc(sizeof(Display::C_CommandBuffer));
	m_pScratchBuffer			= (Display::C_CommandBuffer*)Spu::Alloc(sizeof(Display::C_CommandBuffer));
	m_pJumpToNextBuffer			= (Display::C_CommandBuffer*)Spu::Alloc(sizeof(Display::C_CommandBuffer));
	#else
	m_pCurrentBuffer			= new Display::C_CommandBuffer();
	m_pDevice					= NULL;
	#endif

	m_InvalidateVertexCache		= false;

	ResetStates();
}

C_Context::~C_Context()
{
#if !PLATFORM_SPU
	SAFE_DELETE_PTR(m_pCurrentBuffer);
#endif
}

void *C_Context::Alloc(U32 cSize, U32 *pOffset)
{
	void *pAddr = m_pRingBuffer->Alloc(cSize, m_pMemorySet);
	U32 iOffset = m_pRingBuffer->GetOffsetFromAddress(pAddr);
	if (pOffset)
		(*pOffset) = iOffset;
	return pAddr;
}

void *C_Context::AllocMain(U32 cSize, U32 *pOffset)
{
	void *pAddr = m_pRingBuffer->AllocMain(cSize, m_pMemorySet);
	U32 iOffset = m_pRingBuffer->GetOffsetFromAddress(pAddr);
	if (pOffset)
		(*pOffset) = iOffset;
	return pAddr;
}

bool C_Context::AllocCommandBuffer(Display::C_CommandBuffer* pCommandBuffer, const U32 commandBuffeSize)
{
	BIZ_ASSERT(pCommandBuffer);
	BIZ_ASSERT(m_pRingBuffer);
	BIZ_ASSERT(m_pMemorySet);
	BIZ_ASSERT(commandBuffeSize > 0);

	pCommandBuffer->m_Size = commandBuffeSize;
	pCommandBuffer->m_pAddress = (U8*)m_pRingBuffer->AllocMain(pCommandBuffer->m_Size, m_pMemorySet);
	pCommandBuffer->m_Offset = m_pRingBuffer->GetOffsetFromAddressMain(pCommandBuffer->m_pAddress);
	pCommandBuffer->m_EndOffset = pCommandBuffer->m_Offset;

	BIZ_ASSERT(pCommandBuffer->m_Offset != UINT_MAX);

#if !PLATFORM_SPU && TRACK_DISPLAY2_COMMAND_BUFFERS
	Display2::C_CommandBufferInstance* pInstance = Display2::g_CommandBufferTracker.AllocInstance(m_pCommandBuffer->m_DebugName);
	pInstance->m_pHeadAddress = pCommandBuffer->m_pAddress;
	pInstance->m_pTailAddress = pCommandBuffer->m_pAddress + pCommandBuffer->m_Size;
#endif

	#if DEBUG_COMMAND_BUFFER
	g_UserDebugCommandBuffer.Alloc(pCommandBuffer->m_Size);
	#endif

	return true;
}

bool C_Context::CreateCurrentBuffer(const U32 size, const U32 offset, U8* pAddress)
{
	BIZ_ASSERT(0);
	BIZ_ASSERT(m_pCurrentBuffer);

	m_pCurrentBuffer->m_Size = size;
	m_pCurrentBuffer->m_Offset = offset;
	m_pCurrentBuffer->m_pAddress = pAddress;
	m_pCurrentBuffer->m_EndOffset = offset;

	return true;
}

#if PLATFORM_SPU

bool C_Context::CreateScratchBuffer(const U32 size)
{
	BIZ_ASSERT(m_pScratchBuffer);

	m_pScratchBuffer->m_Size = size;
	m_pScratchBuffer->m_pAddress = (U8*)Spu::Alloc(size);

	return true;
}

bool C_Context::CreateJumpToNextBuffer(const U32 size)
{
	BIZ_ASSERT(m_pJumpToNextBuffer);

	m_pJumpToNextBuffer->m_Size = size;
	m_pJumpToNextBuffer->m_pAddress = (U8*)Spu::Alloc(size);

	return true;
}

#endif

void C_Context::Initialise()
{
	for(int i=0; i<16; i++)
		m_iAniso[i] = g_iAniso[0];

	// not dependant on any other contexts
	m_Flags = m_Id;
}

void C_Context::CloneState(const I_Context* pContextInterface)
{
	if(pContextInterface == NULL)
	{
	#if PLATFORM_SPU
		BIZ_ASSERT(0);
	#else	
		m_pDevice = Display::Device();
		m_CullMode = m_pDevice->GetCullModeDefault();
		m_pRingBuffer = Display::RingBuffer();

		#if USE_GPU_TIMINGS2
		m_pFreeTiming = &Display2::g_FreeTiming;
		m_pTimingKeys = Display2::g_TimingKeys;
		#endif
	#endif
	}
	else
	{
		Display::C_Context* pContext = (Display::C_Context*)pContextInterface;
		Spu::FetchAlloc(pContext);

		#if !PLATFORM_SPU
		m_pDevice = pContext->m_pDevice;
		*m_pView = *pContext->m_pView;
		#endif

		m_CullMode = pContext->m_CullMode;
		m_Multisample = pContext->m_Multisample;
		m_pColourTexture = pContext->m_pColourTexture;
		m_pDepthStencilTexture = pContext->m_pDepthStencilTexture;

		m_pRingBuffer = pContext->m_pRingBuffer;
 		Spu::FetchAlloc(m_pRingBuffer);

		#if USE_GPU_TIMINGS2
		m_pFreeTiming = pContext->m_pFreeTiming;
		m_pTimingKeys = pContext->m_pTimingKeys;
		#endif
	}
}

void C_Context::InvalidateState()
{
}

void C_Context::Flush()
{
}

void C_Context::ResetStates()
{
	m_AlphaRef					= 0;
	m_VertexElementsBitfield	= UINT_MAX;
	m_pVertexDeclaration		= NULL;
	m_pIndexBuffer				= NULL;

	m_SamplerUsageFlags.SetRawFlags(0);
	m_VtxAttrUsageFlags = UINT_MAX;

#if PLATFORM_PS3
	for (int i=0; i<16; i++)
		m_iAniso[i] = CELL_GCM_TEXTURE_MAX_ANISO_1;
#endif
}

void C_Context::BeginCommandBuffer(Display::C_CommandBuffer *pCommandBuffer)
{
	#if PLATFORM_SPU
	//DebugBreak();
	#endif

	BIZ_ASSERT(g_pContext == NULL);
	g_pContext = this; // for callback function

	BIZ_ASSERT(m_pCommandBuffer == NULL);
	m_pCommandBuffer = pCommandBuffer;

	#if PLATFORM_SPU
	m_pMemorySet = &g_UsedMemory;
	#else
	m_pMemorySet = m_pCommandBuffer ? &m_pCommandBuffer->m_UsedMemory : &g_UsedMemory;
	m_pMemorySet->Clear();
	#endif

	#if PLATFORM_SPU
	BIZ_ASSERT(m_pCommandBuffer);
	#else
	if(m_pCommandBuffer)
	#endif
	{
		bool success;

		// Alloc the current buffer
		success = AllocCommandBuffer(m_pCurrentBuffer, g_CommandBufferSize);
		BIZ_ASSERT(success && "Allocating current buffer");

		// Setup the command buffer
		//MT::AtomicSet(&m_pCommandBuffer->m_Size, m_pCurrentBuffer->m_Size); 
		MT::AtomicSet(&m_pCommandBuffer->m_Offset, m_pCurrentBuffer->m_Offset);
		//MT::AtomicSet(&m_pCommandBuffer->m_pAddress, m_pCurrentBuffer->m_pAddress);
		
		// Create the scratch buffer
		#if PLATFORM_SPU
		success = CreateScratchBuffer(g_ScratchBufferSize);
		BIZ_ASSERT(success && "Creating scratch buffer");
		#endif

		// Create the jump to next buffer
		#if PLATFORM_SPU
		success = CreateJumpToNextBuffer(16);
		BIZ_ASSERT(success && "Creating jump to next buffer");
		#endif

		// Create the context
		#if PLATFORM_SPU
		success = m_Context.Create(m_pScratchBuffer, &m_ContextData);
		#else
		success = m_Context.Create(m_pCurrentBuffer, &m_ContextData); 
		#endif
		BIZ_ASSERT(success && "Creating context");

		// Setup the context
		#if PLATFORM_SPU
		m_pScratchBuffer->Setup(&m_ContextData, ContextRenderCallback, -4);
		#else
		m_pCurrentBuffer->Setup(&m_ContextData, ContextRenderCallback, 12);
		#endif

		// Bind the context
		success = m_Context.Bind();
		BIZ_ASSERT(success);
	}
	
	ResetStates();
}

void C_Context::EndCommandBuffer()
{
	#if PLATFORM_SPU
	//DebugBreak();
	#endif

	BIZ_ASSERT(m_pMemorySet);

	#if PLATFORM_SPU
	BIZ_ASSERT(m_pCommandBuffer);
	#else
	if(m_pCommandBuffer)
	#endif
	{
		BIZ_ASSERT(m_pCurrentBuffer);

		#if DEBUG_COMMAND_BUFFER
		g_UserDebugCommandBuffer.Wastage((U32)m_Context.m_pData->end - (U32)m_Context.m_pData->current);
		#endif

		// Flush the context
		ContextRenderCallback(m_Context.m_pData, 0);
		
		// Unbind the context
		m_Context.Unbind(); 

		// Set the command buffer used memory
		#if PLATFORM_SPU
		Spu::Store(&m_pCommandBuffer->m_UsedMemory, m_pMemorySet, sizeof(Display::C_MemorySet));
		#endif
		
		// Set the command buffer end offset which signals the deferred command buffer manager that this command buffer has been built
		MT::AtomicSet(&m_pCommandBuffer->m_EndOffset, m_pCurrentBuffer->m_EndOffset);

		// Destroy the context
		m_Context.Destroy();
	}
	#if !PLATFORM_SPU
	else
	{
		Display::RingBuffer()->Free(m_pMemorySet);

		Display::RingBuffer()->EmitFence();
	}
	#endif

	m_pMemorySet = NULL;

	m_pCommandBuffer = NULL;

	BIZ_ASSERT(g_pContext == this);
	g_pContext = NULL;

	ResetStates();
}

void C_Context::SetVtxAttrUsageFlags(const U32 flags)
{
	// if any flags have changed that are enabled in the vertex declaration, re-upload the vertex declaration
	if (m_VtxAttrUsageFlags!=flags)
		m_Context.SetVertexDeclaration(m_pVertexDeclaration, &m_VertexElementsBitfield, flags);
	m_VtxAttrUsageFlags = flags;
}

void C_Context::SetVertexDeclaration(const Display2::C_VertexDeclaration* pVertexDeclaration)
{
	m_pVertexDeclaration = pVertexDeclaration;

	if(!m_pVertexDeclaration)
		return;

	m_Context.SetVertexDeclaration(m_pVertexDeclaration, &m_VertexElementsBitfield, m_VtxAttrUsageFlags);
}

void C_Context::SetVertexBuffer(const U32 stream, const Display2::C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride)
{
	if(!pVertexBuffer || !m_pVertexDeclaration)
		return;

	m_Context.SetVertexBuffer(stream, pVertexBuffer, offset, stride, m_pVertexDeclaration, m_VtxAttrUsageFlags);

	if(!m_InvalidateVertexCache)
		m_InvalidateVertexCache = true;
}

void C_Context::SetIndexBuffer(const Display2::C_IndexBuffer* pIndexBuffer)
{
	m_pIndexBuffer = pIndexBuffer;

	if(!m_InvalidateVertexCache)
		m_InvalidateVertexCache = true;
}

void C_Context::DrawPrimitive(const Display2::E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount)
{
	SetInvalidateVertexCache();

	m_Context.DrawPrimitive(primitiveType, startVertex, vertexCount);
}

void C_Context::DrawIndexedPrimitive(const Display2::E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount)
{
	BIZ_ASSERT(m_pIndexBuffer);

	SetInvalidateVertexCache();

	m_Context.DrawIndexedPrimitive(primitiveType, baseVertex, minIndex, vertexCount, startIndex, indexCount, m_pIndexBuffer);
}

void* C_Context::BeginVertices(const E_PrimitiveType primitiveType, const U32 vertexCount, const U32 vertexStride, const C_VertexDeclaration* pVertexDeclaration)
{
	BIZ_ASSERT(vertexCount > 0);
	BIZ_ASSERT(vertexStride > 0);
	void* pVertexData = AllocMain(vertexCount * vertexStride);
	m_BeginVerticesPrimitive = primitiveType;
	m_BeginVerticesVertexCount = vertexCount;
	U32 offset = Display2::AddressToOffset(pVertexData);

	for(U32 i = 0; i < pVertexDeclaration->m_ElementCount; i++)
	{
		const C_VertexElement& element = pVertexDeclaration->m_ElementArray[i];

		if(0 == element.m_Stream)
		{
			GCM_NAMESPACE::cellGcmSetVertexDataArrayOffset(m_Context.GetData(), element.m_Usage + element.m_UsageIndex, CELL_GCM_LOCATION_MAIN, offset + element.m_Offset);
		}
	}

	return pVertexData;
}

void C_Context::EndVertices()
{
	if(!m_InvalidateVertexCache)
		m_InvalidateVertexCache = true;

	DrawPrimitive(ToPrimitiveType2(m_BeginVerticesPrimitive), 0, m_BeginVerticesVertexCount);
}

void C_Context::SetAlphaTestFunc(const E_CmpFunc alphaTestFunc)
{
	m_Context.SetAlphaFunc(ToCmpFunc2(alphaTestFunc), m_AlphaRef);
}

void C_Context::SetBlendHighPrecisionEnable(const E_Bool blendHighPrecisionEnable)
{
}

void C_Context::SetFillMode(E_FillMode fillMode)
{
	m_Context.SetFillMode(ToFillMode2(fillMode));
}

void C_Context::SetBlendEnable(const E_Bool blendEnable)
{
	m_Context.SetBlendEnable(ToBool2(blendEnable));
}

void C_Context::SetCullMode(const E_CullMode cullMode)
{
	Display2::C_CullState cullState2;
	switch(cullMode)
	{
	case E_CullMode_Default:
		cullState2.SetCullMode(ToCullMode2(m_CullMode));
		break;
	case E_CullMode_Reversed:
		switch(m_CullMode)
		{
		case E_CullMode_Cw:
			cullState2.SetCullMode(Display2::E_CullMode_Ccw);
			break;
		case E_CullMode_Ccw:
			cullState2.SetCullMode(Display2::E_CullMode_Cw);
			break;
		default:
			cullState2.SetCullMode(Display2::E_CullMode_None);
			break;
		}
		break;
	default:
		cullState2.SetCullMode(ToCullMode2(cullMode));
		break;
	}
	m_Context.SetCullState(cullState2);
}

void C_Context::SetDepthEnable(const E_Bool depthEnable)
{
	m_Context.SetDepthEnable(ToBool2(depthEnable));
}

void C_Context::SetDepthWriteEnable(const E_Bool depthWriteEnable)
{
	m_Context.SetDepthWriteEnable(ToBool2(depthWriteEnable));
}

void C_Context::SetDepthFunc(const E_CmpFunc depthFunc)
{
	m_Context.SetDepthFunc(ToCmpFunc2(depthFunc));
}

void C_Context::SetVertexShader2(const Display2::C_VertexShader *pShader)
{
	BIZ_ASSERT(pShader);

	m_Context.SetVertexShader(pShader);
}

void C_Context::SetVertexShader(const Display::C_VertexShader *pShader)
{
	BIZ_ASSERT(pShader);

	m_Context.SetVertexShader(pShader);
}

void C_Context::SetPixelShader2(const Display2::C_PixelShader *pShader)
{
	BIZ_ASSERT(pShader);

	m_Context.SetPixelShader(pShader);
}

void C_Context::SetPixelShader(const Display2::C_PixelShader *pShader, U32 offset)
{
	BIZ_ASSERT(pShader);

	m_Context.SetPixelShader(pShader, offset);
}

void C_Context::SetPixelShader(const Display::C_PixelShader *pShader)
{
	BIZ_ASSERT(pShader);

	m_Context.SetPixelShader(pShader);
}

void C_Context::SetStencilState(const C_StencilState& stencilState)
{
	m_Context.SetStencilEnable(ToBool2(stencilState.m_StencilEnable));

	Display2::C_StencilState stencilState2;
	stencilState2.SetStencilFunc(ToCmpFunc2(stencilState.m_StencilFunc));
	stencilState2.SetStencilFail(ToStencilOp2(stencilState.m_StencilFail));
	stencilState2.SetStencilDepthFail(ToStencilOp2(stencilState.m_StencilDepthFail));
	stencilState2.SetStencilDepthPass(ToStencilOp2(stencilState.m_StencilDepthPass));
	stencilState2.SetStencilRef(stencilState.m_StencilRef);
	stencilState2.SetStencilMask(stencilState.m_StencilMask);
	stencilState2.SetStencilWriteMask(stencilState.m_StencilWriteMask);
	m_Context.SetStencilState(stencilState2);
}

void C_Context::SetColourWriteEnable(const U32 flags)
{
	m_Context.SetColourWriteEnable(flags);
}

void C_Context::SetAlphaTestState(const E_Bool alphaTestEnable, const U32 alphaRef)
{
	m_Context.SetAlphaTestEnable(alphaTestEnable);
		
	Display2::C_AlphaTestState alphaTestState2;
	alphaTestState2.SetAlphaTestCmpFunc(Display2::E_CmpFunc_Greater);
	alphaTestState2.SetAlphaTestRef(alphaRef);
	m_Context.SetAlphaTestState(alphaTestState2);
	
	m_AlphaRef = alphaRef;
}

void C_Context::SetAlphaToMaskEnable(const E_Bool alphaToMaskEnable)
{
	const uint32_t multisampleEnable = m_Multisample != E_Multisample_None ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	m_Context.SetAntiAliasingControl(multisampleEnable, ToBool2(alphaToMaskEnable), ToBool2(alphaToMaskEnable), 0xffff);
}

void C_Context::SetPixelShaderParameterF(C_PixelShaderParameter handle, const F32 *src)
{
	if(!handle)
		return;

	m_Context.SetPixelShaderParamF(handle, src, 1);
}

void C_Context::SetPixelShaderParameters(const C_PixelShader *pShader)
{
	BIZ_ASSERT(pShader);
	
	m_Context.SetPixelShaderParams(pShader);
}

void C_Context::SetPixelShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount)
{
	BIZ_ASSERT(0);
}

void C_Context::SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount)
{
	m_Context.SetVertexShaderConstantF(startRegister, pData, registerCount);
}

void C_Context::SetTexture(const U32 sampler, const void* pTexture)
{
	CellGcmTexture* pGcmTexture = (CellGcmTexture*)pTexture;
	Display2::E_Bool bEnable = Display2::E_Bool_False;
	if (!pGcmTexture)
		return;

#if !PLATFORM_SPU
	CheckGCMTexture(pGcmTexture);
#endif

	m_Context.SetTexture(
		sampler,
		pGcmTexture);
}

void C_Context::SetVertexTexture(const U32 sampler, const void* pTexture)
{
#if PLATFORM_SPU
	CellGcmTexture* pGcmTexture = (CellGcmTexture*)pTexture;
	if(!pGcmTexture)
		return;
#else
	CellGcmTexture* pGcmTexture = pTexture ? (CellGcmTexture*)pTexture : Device()->GetNullTexture()->GetGPUTexture();
#endif

#if !PLATFORM_SPU
	CheckGCMTexture(pGcmTexture);
#endif

	m_Context.SetVertexTexture(
		sampler,
		pGcmTexture);
}

void C_Context::SetSamplerState(const U32 sampler, const Display::C_SamplerState *pS)
{
	m_Context.SetSamplerState(sampler, *pS);
	
	// Hack to set max aniso without setting a texture
	int iAniso = pS->m_MaxAnisotropy;
	m_iAniso[sampler] = iAniso;
	if (m_SamplerUsageFlags.GetEnabled(0, sampler))
	{
		Display2::C_TextureState textureState2;
		textureState2.m_TextureControlEnable		= Display2::E_Bool_True;
		textureState2.m_TextureControlMinLod		= 0;
		textureState2.m_TextureControlMaxLod		= 12;
		textureState2.m_TextureControlMaxAniso		= iAniso;
		m_Context.SetTextureState_TextureControl(sampler, textureState2);
	}
}

void C_Context::SetVertexSamplerState(const U32 sampler, const Display::C_SamplerState *pS)
{
	Display2::C_VertexSamplerState tmp;
	tmp.m_SetTextureAddressBits = pS->m_SetTextureAddressBits;
	tmp.m_TextureBorderColour = pS->m_TextureBorderColour;
	tmp.m_SetTextureFilterBits = pS->m_SetTextureFilterBits;
	m_Context.SetVertexSamplerState(sampler, tmp);
}

/// set texture sampler usage flags
void C_Context::SetSamplerUsageFlags(const C_SamplerUsageFlags *pFlags)
{
	U32 iOrg = m_SamplerUsageFlags.GetRawFlags();
	U32 iNew = pFlags->GetRawFlags();
	U32 iDiff = iOrg^iNew;
	U32 iBit = 1;
	for (int iSampler=0; iSampler<16; iSampler++)
	{
		if (iDiff&iBit)
		{
			m_Context.SetTextureControl(
				iSampler,
				(iNew&iBit)?Display2::E_Bool_True:Display2::E_Bool_False,
				0,
				12<<8,
				m_iAniso[iSampler]);
		}

		iBit<<=1;
	}
	for (int iSampler=0; iSampler<4; iSampler++)
	{
		if (iDiff&iBit)
		{
			m_Context.SetVertexTextureControl(
				iSampler,
				(iNew&iBit)?Display2::E_Bool_True:Display2::E_Bool_False,
				0,
				12<<8);
		}

		iBit<<=1;
	}

	m_SamplerUsageFlags.SetRawFlags( iNew );
}

#if USE_GPU_TIMINGS2
U32 C_Context::GetTimeStamp(const U32 timeStampKey)
{
	U32 timingIndex = MT::AtomicAdd(m_pFreeTiming, 2) % Display2::E_Report_TimingCount;

	MT::AtomicSet(m_pTimingKeys + (timingIndex / 2), timeStampKey);

	return timingIndex;
}

void C_Context::SetTimeStamp(const U32 timeStampIndex)
{
	m_Context.SetTimeStamp(Display2::E_Report_TimingStart + timeStampIndex);
}
#endif

void C_Context::SetInvalidateVertexCache()
{
	if(!m_InvalidateVertexCache)
		return;

	m_Context.SetInvalidateVertexCache();
	
	m_InvalidateVertexCache = false;
}

void C_Context::SetVertexAttribOutputMask(const U32 mask)
{
	GCM_NAMESPACE::cellGcmSetVertexAttribOutputMask(m_Context.GetData(), mask);
}

C_ContextManager g_ContextManager;

void C_SamplerState::SetMaxAnisotropy(S8 x)
{
	m_MaxAnisotropy = g_iAniso[x];
}

} // namespace Display
