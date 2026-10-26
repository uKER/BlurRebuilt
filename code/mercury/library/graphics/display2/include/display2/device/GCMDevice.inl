#ifndef DISPLAY2_GCM_DEVICE_INL
#define DISPLAY2_GCM_DEVICE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

INLINE U32 C_Device::IsCreated() const
{
	return m_Created;
}

INLINE C_Context& C_Device::GetContext()
{
	return m_Context;
}

INLINE const C_Context& C_Device::GetContext() const
{
	return m_Context;
}

INLINE C_ContextNoCheck& C_Device::GetContextNoCheck()
{
	return m_ContextNoCheck;
}

INLINE const C_ContextNoCheck& C_Device::GetContextNoCheck() const
{
	return m_ContextNoCheck;
}

INLINE C_ContextNoCheckNoTrack& C_Device::GetContextNoCheckNoTrack()
{
	return m_ContextNoCheckNoTrack;
}

INLINE const C_ContextNoCheckNoTrack& C_Device::GetContextNoCheckNoTrack() const
{
	return m_ContextNoCheckNoTrack;
}

INLINE C_MemoryManager* C_Device::GetMainHeap()
{
	return &m_MainHeap;
}

INLINE C_MemoryManager* C_Device::GetLocalHeap()
{
	return &m_LocalHeap;
}

INLINE C_MemoryManager* C_Device::GetCmdBufHeap()
{
	return &m_CmdBufHeap;
}

INLINE S32 C_Device::GetVertexDeclarationCount() const
{
	return m_VertexDeclarationList.Size();
}

INLINE bool C_Device::CreateColourSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pAddress, U32 addressSize)
{
	return CreateSurface(width, height, flags, format, multisample, pParams, pSurface, pAddress, addressSize);
}

INLINE bool C_Device::CreateDepthStencilSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pAddress, U32 addressSize)
{
	return CreateSurface(width, height, flags, format, multisample, pParams, pSurface, pAddress, addressSize);
}

INLINE C_VertexDeclaration* C_Device::GetVertexDeclaration(const S32 index)
{
	assert(index >= 0 && index < m_VertexDeclarationList.Size());
	return m_VertexDeclarationList[index];
}

INLINE void C_Device::Destroy(C_VertexDeclaration* pVertexDeclaration)
{
	DestroyVertexDeclaration(pVertexDeclaration);
}

INLINE void C_Device::Destroy(C_VertexBuffer* pVertexBuffer)
{
	DestroyVertexBuffer(pVertexBuffer);
}

INLINE void C_Device::Destroy(C_IndexBuffer* pIndexBuffer)
{
	DestroyIndexBuffer(pIndexBuffer);
}

INLINE void C_Device::Destroy(C_VertexShader* pVertexShader)
{
	DestroyVertexShader(pVertexShader);
}

INLINE void C_Device::Destroy(C_PixelShader* pPixelShader)
{
	DestroyPixelShader(pPixelShader);
}

INLINE void C_Device::Destroy(C_Surface* pSurface)
{
	DestroySurface(pSurface);
}

INLINE void C_Device::Destroy(C_Texture* pTexture)
{
	DestroyTexture(pTexture);
}

INLINE void C_Device::Destroy(C_RenderTexture2D* pRenderTexture)
{
	DestroyRenderTexture2D(pRenderTexture);
}

INLINE void C_Device::Destroy(C_RenderTextureCube* pRenderTexture)
{
	DestroyRenderTextureCube(pRenderTexture);
}

INLINE void C_Device::Destroy(C_RenderBuffer* pRenderBuffer)
{
	DestroyRenderBuffer(pRenderBuffer);
}

INLINE const C_AlphaTestState& C_Device::GetAlphaTestState() const
{
	return m_Context.GetAlphaTestState();
}

INLINE const C_BlendState& C_Device::GetBlendState() const
{
	return m_Context.GetBlendState();
}

INLINE const C_ColourState& C_Device::GetColourState() const
{
	return m_Context.GetColourState();
}

INLINE const C_CullState& C_Device::GetCullState() const
{
	return m_Context.GetCullState();
}

INLINE const C_DepthState& C_Device::GetDepthState() const
{
	return m_Context.GetDepthState();
}

INLINE const C_FillState& C_Device::GetFillState() const
{
	return m_Context.GetFillState();
}

INLINE const C_SamplerState& C_Device::GetSamplerState(const U32 index) const
{
	return m_Context.GetSamplerState(index);
}

INLINE const C_ScissorState& C_Device::GetScissorState() const
{
	return m_Context.GetScissorState();
}

INLINE const C_StencilState& C_Device::GetStencilState() const
{
	return m_Context.GetStencilState();
}

INLINE const C_VertexSamplerState& C_Device::GetVertexSamplerState(const U32 index) const
{
	return m_Context.GetVertexSamplerState(index);
}

INLINE void C_Device::SetAlphaTestState(const C_AlphaTestState& state)
{
	m_Context.SetAlphaTestState(state);
}

INLINE void C_Device::SetBlendState(const C_BlendState& state)
{
	m_Context.SetBlendState(state);
}

INLINE void C_Device::SetColourState(const C_ColourState& state)
{
	m_Context.SetColourState(state);
}

INLINE void C_Device::SetCullState(const C_CullState& state)
{
	m_Context.SetCullState(state);
}

INLINE void C_Device::SetDepthState(const C_DepthState& state)
{
	m_Context.SetDepthState(state);
}

INLINE void C_Device::SetFillState(const C_FillState& state)
{
	m_Context.SetFillState(state);
}

INLINE void C_Device::SetSamplerState(const U32 index, const C_SamplerState& state)
{
	m_Context.SetSamplerState(index, state);
}

INLINE void C_Device::SetScissorState(const C_ScissorState& state)
{
	m_Context.SetScissorState(state);
}

INLINE void C_Device::SetStencilState(const C_StencilState& state)
{
	m_Context.SetStencilState(state);
}

INLINE void C_Device::SetVertexSamplerState(const U32 index, const C_VertexSamplerState& state)
{
	m_Context.SetVertexSamplerState(index, state);
}

INLINE void C_Device::SetBlendMode(const E_BlendMode blendMode)
{
	assert(blendMode < E_BlendMode_Count);
	m_Context.SetAlphaTestState(m_BlendModes[blendMode].m_AlphaTestState);
	m_Context.SetBlendState(m_BlendModes[blendMode].m_BlendState);
}

INLINE void C_Device::SetCullMode(const E_CullMode cullMode)
{
	C_CullState cullState = GetCullState();
	cullState.SetCullMode(cullMode);
	m_Context.SetCullState(cullState);
}

INLINE void C_Device::SetFillMode(const E_FillMode fillMode)
{
	C_FillState fillState = GetFillState();
	fillState.SetFillMode(fillMode);
	m_Context.SetFillState(fillState);
}

INLINE void C_Device::SetIndexBuffer(const C_IndexBuffer* pIndexBuffer)
{
	m_Context.SetIndexBuffer(pIndexBuffer);
}

INLINE void C_Device::SetPixelShader(const C_PixelShader* pPixelShader)
{
	m_Context.SetPixelShader(pPixelShader);
}

INLINE void C_Device::SetPixelShader(const C_PixelShader* pPixelShader, U32 offset)
{
	m_Context.SetPixelShader(pPixelShader, offset);
}

INLINE void C_Device::SetPixelShaderParamF(const C_PixelShaderParam& shaderParam, const F32* pData, const U32 registerCount)
{
	m_Context.SetPixelShaderParamF(shaderParam, pData, registerCount);
}

INLINE void C_Device::SetTexture(const U32 index, const C_Texture* pTexture)
{
	//m_Context.SetTexture(index, pTexture);
}

INLINE void C_Device::SetVertexBuffer(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride)
{
	m_Context.SetVertexBuffer(stream, pVertexBuffer, offset, stride);
}

INLINE void C_Device::SetVertexDeclaration(const C_VertexDeclaration* pVertexDeclaration)
{
	m_Context.SetVertexDeclaration(pVertexDeclaration);
}

INLINE void C_Device::SetVertexShader(const C_VertexShader* pVertexShader)
{
	m_Context.SetVertexShader(pVertexShader);
}	

INLINE void C_Device::SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount)
{
	m_Context.SetVertexShaderConstantF(startRegister, pData, registerCount);
}

INLINE void C_Device::SetVertexShaderParamF(const C_VertexShaderParam& shaderParam, const F32* pData)
{
	if(shaderParam.m_RegisterIndex != UINT_MAX && shaderParam.m_RegisterCount != 0)
		m_Context.SetVertexShaderParamF(shaderParam, pData);
}

INLINE void C_Device::DrawPrimitive(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount)
{
	m_Context.DrawPrimitive(primitiveType, startVertex, vertexCount);
}

INLINE void C_Device::DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount)
{
	m_Context.DrawIndexedPrimitive(primitiveType, baseVertex, minIndex, vertexCount, startIndex, indexCount);
}

INLINE bool C_DeviceManager::IsCreated()
{
	return m_ioAddress != NULL ? true : false;
}

INLINE U32 C_DeviceManager::GetCBSize() const
{
	return m_cbSize;
}

INLINE U32 C_DeviceManager::GetIOSize() const
{
	return m_ioSize;
}

} // namespace Display2

#endif // DISPLAY2_GCM_DEVICE_INL
