#define GPU_FUNC(_fnc) _fnc
#define GPU_DECL(_rtn) _rtn

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

GPU_DECL(GPU_VOID) GPU_FUNC(SetViewport)(const C_Viewport& viewport);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexDeclaration)(const C_VertexDeclaration* pVertexDeclaration);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexBuffer)(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride, const C_VertexDeclaration* pVertexDeclaration);
GPU_DECL(GPU_VOID) GPU_FUNC(SetIndexBuffer)(const C_IndexBuffer* pIndexBuffer);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTexture)(const U32 index, const C_Texture* pTexture);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexShader)(const C_VertexShader* pVertexShader);
GPU_DECL(GPU_VOID) GPU_FUNC(SetPixelShader)(const C_PixelShader* pPixelShader);
GPU_DECL(GPU_VOID) GPU_FUNC(DrawIndexedPrimitive)(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount, const C_IndexBuffer* pIndexBuffer);

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL(GPU_VOID) GPU_FUNC(BeginScene)();
GPU_DECL(GPU_VOID) GPU_FUNC(Clear)(const U32 flags, const U32 colour, const F32 depth, const U8 stencil);
GPU_DECL(GPU_VOID) GPU_FUNC(DrawPrimitive)(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount);
GPU_DECL(GPU_VOID) GPU_FUNC(DrawIndexedPrimitive)(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount);
GPU_DECL(GPU_VOID) GPU_FUNC(EndScene)();
GPU_DECL(GPU_VOID) GPU_FUNC(Present)();
GPU_DECL(GPU_VOID) GPU_FUNC(SetAlphaTestEnable)(const E_Bool alphaTestEnable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetAlphaTestFunc)(const E_CmpFunc alphaTestFunc);
GPU_DECL(GPU_VOID) GPU_FUNC(SetAlphaTestRef)(const U32 alphaTestRef);
GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendSrc)(const E_Blend blendSrc);
GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendEnable)(const E_Bool blendEnable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendDst)(const E_Blend blendDst);
GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendOp)(const E_BlendOp blendOp);
GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendSrcAlpha)(const E_Blend blendSrcAlpha);
GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendDstAlpha)(const E_Blend blendDstAlpha);
GPU_DECL(GPU_VOID) GPU_FUNC(SetBlendOpAlpha)(const E_BlendOp blendOpAlpha);
GPU_DECL(GPU_VOID) GPU_FUNC(SetColourSurface)(const U32 index, const C_Surface* pSurface);
GPU_DECL(GPU_VOID) GPU_FUNC(SetColourWriteEnable)(const U32 colourWriteEnable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetCullMode)(const E_CullMode cullMode);
GPU_DECL(GPU_VOID) GPU_FUNC(SetDepthBias)(const F32 depthBias);
GPU_DECL(GPU_VOID) GPU_FUNC(SetDepthEnable)(const E_Bool depthEnable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetDepthFunc)(const E_CmpFunc depthFunc);
GPU_DECL(GPU_VOID) GPU_FUNC(SetDepthWriteEnable)(const E_Bool depthWriteEnable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetDepthStencilSurface)(const C_Surface* pSurface);
GPU_DECL(GPU_VOID) GPU_FUNC(SetFillMode)(const E_FillMode fillMode);
GPU_DECL(GPU_VOID) GPU_FUNC(SetPixelShaderConstantB)(const U32 startRegister, const BOOL* pData, const U32 registerCount);
GPU_DECL(GPU_VOID) GPU_FUNC(SetPixelShaderConstantI)(const U32 startRegister, const S32* pData, const U32 registerCount);
GPU_DECL(GPU_VOID) GPU_FUNC(SetPixelShaderConstantF)(const U32 startRegister, const F32* pData, const U32 registerCount);
GPU_DECL(GPU_VOID) GPU_FUNC(SetPixelShaderParamF)(const C_PixelShaderParam& shaderParam, const F32* pData, const U32 registerCount = 1);
GPU_DECL(GPU_VOID) GPU_FUNC(SetPixelShaderParamArrayF)(const C_PixelShaderParam& shaderParam, const F32* pData, const U32 registerCount = 1);
GPU_DECL(GPU_VOID) GPU_FUNC(SetPointSpriteEnable)(const E_Bool pointSpriteEnable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetRenderState)(const U32 renderState, const U32 value);
GPU_DECL(GPU_VOID) GPU_FUNC(SetSamplerState)(const U32 index, const U32 samplerState, const U32 value);
GPU_DECL(GPU_VOID) GPU_FUNC(SetScissorEnable)(const E_Bool scissorTestEnable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetScissorRect)(const RECT& rect);
GPU_DECL(GPU_VOID) GPU_FUNC(SetSlopeScaledBias)(const F32 slopeScaledBias);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilEnable)(const E_Bool stencilEnable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilFail)(const E_StencilOp stencilFail);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilDepthFail)(const E_StencilOp stencilDepthFail);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilPass)(const E_StencilOp stencilPass);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilFunc)(const E_CmpFunc stencilFunc);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilRef)(const U32 stencilRef);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilMask)(const U32 stencilMask);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilWriteMask)(const U32 stencilWriteMask);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilCcwEnable)(const E_Bool stencilCcwEnable);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilCcwFail)(const E_StencilOp stencilFail);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilCcwDepthFail)(const E_StencilOp stencilDepthFail);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilCcwPass)(const E_StencilOp stencilPass);
GPU_DECL(GPU_VOID) GPU_FUNC(SetStencilCcwFunc)(const E_CmpFunc stencilFunc);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTextureAddressU)(const U32 index, const E_TextureAddress addressU);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTextureAddressV)(const U32 index, const E_TextureAddress addressV);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTextureAddressW)(const U32 index, const E_TextureAddress addressW);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTextureFilterMag)(const U32 index, const E_TextureFilter filterMag);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTextureFilterMin)(const U32 index, const E_TextureFilter filterMin);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTextureFilterMip)(const U32 index, const E_TextureFilter filterMip);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTextureBorderColour)(const U32 index, const U32 borderColour);
GPU_DECL(GPU_VOID) GPU_FUNC(SetTextureLodBias)(const U32 index, const S8 lodBias);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexBuffer)(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexShaderConstantB)(const U32 startRegister, const BOOL* pData, const U32 registerCount);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexShaderConstantI)(const U32 startRegister, const S32* pData, const U32 registerCount);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexShaderConstantF)(const U32 startRegister, const F32* pData, const U32 registerCount);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexShaderParamF)(const C_VertexShaderParam& shaderParam, const F32* pData);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTexture)(const U32 index, const C_Texture* pTexture);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTextureAddressU)(const U32 index, const E_TextureAddress addressU);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTextureAddressV)(const U32 index, const E_TextureAddress addressV);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTextureAddressW)(const U32 index, const E_TextureAddress addressW);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTextureFilterMag)(const U32 index, const E_TextureFilter filterMag);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTextureFilterMin)(const U32 index, const E_TextureFilter filterMin);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTextureFilterMip)(const U32 index, const E_TextureFilter filterMip);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTextureBorderColour)(const U32 index, const U32 borderColour);
GPU_DECL(GPU_VOID) GPU_FUNC(SetVertexTextureLodBias)(const U32 index, const S8 lodBias);

//------------------------------------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------------------------------------------

#undef GPU_DECL
#undef GPU_FUNC
