#define GPU_FUNC(_fnc) GPU_CLASS::_fnc
#ifdef CFG_DEBUG
#define GPU_DECL(_rtn) INLINE _rtn
#else
#define GPU_DECL(_rtn) FORCE_INLINE _rtn
#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------

#if GPU_CHECK || GPU_TRACK

GPU_DECL(const C_AlphaTestState&) GPU_FUNC(GetAlphaTestState)() const
{
	return m_pState->m_AlphaTestState;
}

GPU_DECL(const C_BlendState&) GPU_FUNC(GetBlendState)() const
{
	return m_pState->m_BlendState;
}

GPU_DECL(const C_ColourState&) GPU_FUNC(GetColourState)() const
{
	return m_pState->m_ColourState;
}

GPU_DECL(const C_CullState&) GPU_FUNC(GetCullState)() const
{
	return m_pState->m_CullState;
}

GPU_DECL(const C_DepthState&) GPU_FUNC(GetDepthState)() const
{
	return m_pState->m_DepthState;
}

GPU_DECL(const C_FillState&) GPU_FUNC(GetFillState)() const
{
	return m_pState->m_FillState;
}

GPU_DECL(const C_TextureState&) GPU_FUNC(GetTextureState)(const U32 i) const
{
	return m_pState->m_TextureState[i];
}

GPU_DECL(const C_SamplerState&) GPU_FUNC(GetSamplerState)(const U32 i) const
{
	return m_pState->m_SamplerState[i];
}

GPU_DECL(const C_ScissorState&) GPU_FUNC(GetScissorState)() const
{
	return m_pState->m_ScissorState;
}

GPU_DECL(const C_StencilState&) GPU_FUNC(GetStencilState)() const
{
	return m_pState->m_StencilState;
}

GPU_DECL(const C_VertexTextureState&) GPU_FUNC(GetVertexTextureState)(const U32 i) const
{
	return m_pState->m_VertexTextureState[i];
}

GPU_DECL(const C_VertexSamplerState&) GPU_FUNC(GetVertexSamplerState)(const U32 i) const
{
	return m_pState->m_VertexSamplerState[i];
}

#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL(GPU_VOID) GPU_FUNC(SetAlphaTestState)(const C_AlphaTestState& state)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendState)(const C_BlendState& state)
{

	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetColourState)(const C_ColourState& state)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetCullState)(const C_CullState& state)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetDepthState)(const C_DepthState& state)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetFillState)(const C_FillState& state)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetTextureState)(const U32 index, const C_TextureState& state)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetSamplerState)(const U32 index, const C_SamplerState& state)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetScissorState)(const C_ScissorState& state)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilState)(const C_StencilState& state)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTextureState)(const U32 index, const C_VertexTextureState& state)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexSamplerState)(const U32 index, const C_VertexSamplerState& state)
{
	GPU_RETURN_VOID;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL(GPU_VOID) GPU_FUNC(SetAlphaTestEnable)(GPU_ARGS(const E_Bool enable))
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendEnable)(GPU_ARGS(const E_Bool enable))
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilEnable)(GPU_ARGS(const E_Bool enable))
{
	GPU_RETURN_VOID;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------


GPU_DECL(GPU_VOID) GPU_FUNC(SetCullMode)(const E_CullMode cullMode)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetFillMode)(const E_FillMode fillMode)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetColourWriteEnable)(const U32 colourWriteEnable)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetDepthWriteEnable)(const E_Bool depthWriteEnable)
{
	GPU_RETURN_VOID;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL(GPU_VOID) GPU_FUNC(SetViewport)(const C_Viewport& viewport)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexDeclaration(GPU_ARGS(const C_VertexDeclaration* pVertexDeclaration)))
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexBuffer(GPU_ARGS(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride)))
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetIndexBuffer(GPU_ARGS(const C_IndexBuffer* pIndexBuffer)))
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetTexture)(const U32 index, const C_Texture* pTexture)
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexShader(GPU_ARGS(const C_VertexShader* pVertexShader)))
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexShaderConstantF(GPU_ARGS(const U32 startRegister, const F32* pData, const U32 registerCount)))
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexShaderParamF(GPU_ARGS(const C_VertexShaderParam& shaderParam, const F32* pData)))
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetPixelShader(GPU_ARGS(const C_PixelShader* pPixelShader)))
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(SetPixelShaderParamF(GPU_ARGS(const C_PixelShaderParam& shaderParam, const F32* pData, const U32 registerCount)))
{
	GPU_RETURN_VOID;
}

GPU_DECL(GPU_VOID) GPU_FUNC(DrawIndexedPrimitive(GPU_ARGS(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount)))
{
	GPU_RETURN_VOID;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

#undef GPU_DECL
#undef GPU_FUNC
