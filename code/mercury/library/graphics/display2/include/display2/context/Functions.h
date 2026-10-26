//------------------------------------------------------------------------------------------------------------------------------------------------------

#if GPU_CHECK || GPU_TRACK

GPU_DECL(const C_AlphaTestState&)		GPU_FUNC(GetAlphaTestState)() const;
GPU_DECL(const C_BlendState&)			GPU_FUNC(GetBlendState)() const;
GPU_DECL(const C_ColourState&)			GPU_FUNC(GetColourState)() const;
GPU_DECL(const C_CullState&)			GPU_FUNC(GetCullState)() const;
GPU_DECL(const C_DepthState&)			GPU_FUNC(GetDepthState)() const;
GPU_DECL(const C_FillState&)			GPU_FUNC(GetFillState)() const;
GPU_DECL(const C_TextureState&)			GPU_FUNC(GetTextureState)(const U32 i) const;
GPU_DECL(const C_SamplerState&)			GPU_FUNC(GetSamplerState)(const U32 i) const;
GPU_DECL(const C_ScissorState&)			GPU_FUNC(GetScissorState)() const;
GPU_DECL(const C_StencilState&)			GPU_FUNC(GetStencilState)() const;
GPU_DECL(const C_VertexTextureState&)	GPU_FUNC(GetVertexTextureState)(const U32 i) const;
GPU_DECL(const C_VertexSamplerState&)	GPU_FUNC(GetVertexSamplerState)(const U32 i) const;

#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL(GPU_VOID) GPU_FUNC(SetAlphaTestState)(const C_AlphaTestState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendState)(const C_BlendState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetColourState)(const C_ColourState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetCullState)(const C_CullState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetDepthState)(const C_DepthState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetFillState)(const C_FillState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTextureState)(const U32 index, const C_TextureState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetSamplerState)(const U32 index, const C_SamplerState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetScissorState)(const C_ScissorState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilState)(const C_StencilState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTextureState)(const U32 index, const C_VertexTextureState& state);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexSamplerState)(const U32 index, const C_VertexSamplerState& state);

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL(GPU_VOID) GPU_FUNC(SetAlphaTestEnable)(const E_Bool enable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendEnable)(const E_Bool enable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilEnable)(const E_Bool enable);

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL(GPU_VOID) GPU_FUNC(SetCullMode)(const E_CullMode cullMode);
GPU_DECL(GPU_VOID) GPU_FUNC(SetFillMode)(const E_FillMode fillMode);
GPU_DECL(GPU_VOID) GPU_FUNC(SetColourWriteEnable)(const U32 colourWriteEnable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetDepthWriteEnable)(const E_Bool depthWriteEnable);

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL(GPU_VOID) GPU_FUNC(SetViewport)(const C_Viewport& viewport);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexDeclaration)(const C_VertexDeclaration* pVertexDeclaration);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexBuffer)(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride, const C_VertexDeclaration* pVertexDeclaration);
GPU_DECL(GPU_VOID) GPU_FUNC(SetIndexBuffer)(const C_IndexBuffer* pIndexBuffer);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTexture)(const U32 index, const C_Texture* pTexture);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexShader)(const C_VertexShader* pVertexShader);
GPU_DECL(GPU_VOID) GPU_FUNC(SetPixelShader)(const C_PixelShader* pPixelShader);
GPU_DECL(GPU_VOID) GPU_FUNC(DrawIndexedPrimitive)(const U32 primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount, const C_IndexBuffer* pIndexBuffer);

//------------------------------------------------------------------------------------------------------------------------------------------------------
