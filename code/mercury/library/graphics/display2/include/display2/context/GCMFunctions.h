#define GPU_DECL
#define GPU_FUNC(_func) _func

//------------------------------------------------------------------------------------------------------------------------------------------------------

#if GPU_CHECK || GPU_TRACK
GPU_DECL const C_AlphaTestState& GPU_FUNC(GetAlphaTestState)() const;
GPU_DECL const C_BlendState& GPU_FUNC(GetBlendState)() const;
GPU_DECL const C_ColourState& GPU_FUNC(GetColourState)() const;
GPU_DECL const C_CullState& GPU_FUNC(GetCullState)() const;
GPU_DECL const C_DepthState& GPU_FUNC(GetDepthState)() const;
GPU_DECL const C_FillState& GPU_FUNC(GetFillState)() const;
GPU_DECL const C_SamplerState& GPU_FUNC(GetSamplerState)(const U32 i) const;
GPU_DECL const C_ScissorState& GPU_FUNC(GetScissorState)() const;
GPU_DECL const C_StencilState& GPU_FUNC(GetStencilState)() const;
GPU_DECL const C_VertexSamplerState& GPU_FUNC(GetVertexSamplerState)(const U32 i) const;
#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetAlphaTestState)(const C_AlphaTestState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetBlendState)(const C_BlendState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetColourState)(const C_ColourState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetCullState)(const C_CullState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetDepthState)(const C_DepthState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetFillState)(const C_FillState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetTextureState)(const U32 index, const C_TextureState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetTextureState_TextureControl)(const U32 index, const C_TextureState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetSamplerState)(const U32 index, const C_SamplerState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetScissorState)(const C_ScissorState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetStencilState)(const C_StencilState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetVertexTextureState)(const U32 index, const C_VertexTextureState& state);
GPU_DECL GPU_VOID GPU_FUNC(SetVertexSamplerState)(const U32 index, const C_VertexSamplerState& state);

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetAlphaTestEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetBlendEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetStencilEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetPointSpriteEnable)(const uint32_t enable);

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetDepthWriteEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetDepthEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetDepthFunc)(GPU_ARGS(const uint32_t zf));
GPU_DECL GPU_VOID GPU_FUNC(SetFillMode)(GPU_ARGS(const uint32_t mode));

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(DrawPrimitive(GPU_ARGS(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount)));
GPU_DECL GPU_VOID GPU_FUNC(DrawIndexedPrimitive(GPU_ARGS(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount)));
GPU_DECL GPU_VOID GPU_FUNC(DrawIndexedPrimitive(GPU_ARGS(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount, const C_IndexBuffer* pIndexBuffer)));
GPU_DECL GPU_VOID GPU_FUNC(DrawIndexedPrimitive(GPU_ARGS(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount, const U32 indexFormat, const U32 indexLocation, const U32 indexOffset)));
GPU_DECL GPU_VOID GPU_FUNC(SetIndexBuffer(GPU_ARGS(const C_IndexBuffer* pIndexBuffer)));
GPU_DECL GPU_VOID GPU_FUNC(SetPixelShader(GPU_ARGS(const C_PixelShader* pPixelShader)));
GPU_DECL GPU_VOID GPU_FUNC(SetPixelShader(GPU_ARGS(const C_PixelShader* pPixelShader, U32 offset)));
GPU_DECL GPU_VOID GPU_FUNC(SetPixelShaderEmbeddedParamF(GPU_ARGS(const uint32_t dstOffset, const float* pData, const uint32_t sizeInWords, const uint32_t location)));
GPU_DECL GPU_VOID GPU_FUNC(SetPixelShaderParamF(GPU_ARGS(const C_PixelShaderParam& shaderParam, const F32* pData, const U32 registerCount = 1)));
GPU_DECL GPU_VOID GPU_FUNC(SetPixelShaderParamArrayF(GPU_ARGS(const C_PixelShaderParamArray& shaderParamArray, const F32* pData, const U32 registerCount = 1)));
GPU_DECL GPU_VOID GPU_FUNC(SetPixelShaderParams(GPU_ARGS(const C_PixelShader* pPixelShader)));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexBuffer(GPU_ARGS(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride)));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexBuffer(GPU_ARGS(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride, const C_VertexDeclaration* pVertexDeclaration, U32 iUsageFlags = UINT_MAX)));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexDeclaration(GPU_ARGS(const C_VertexDeclaration* pVertexDeclaration, U32* pVertexElementsBitfield = NULL, U32 iUsageFlags = UINT_MAX)));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexShader(GPU_ARGS(const C_VertexShader* pVertexShader)));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexShaderConstantF(GPU_ARGS(const U32 startRegister, const F32* pData, const U32 registerCount)));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexShaderParamF(GPU_ARGS(const C_VertexShaderParam& shaderParam, const F32* pData)));

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetReferenceCommand)(GPU_ARGS(const uint32_t ref));
GPU_DECL GPU_VOID GPU_FUNC(SetJumpCommand)(GPU_ARGS(const uint32_t offset));
GPU_DECL GPU_VOID GPU_FUNC(SetCallCommand)(GPU_ARGS(const uint32_t offset));
GPU_DECL GPU_VOID GPU_FUNC(SetReturnCommand)(GPU_NO_ARGS());
GPU_DECL GPU_VOID GPU_FUNC(SetNopCommand)(GPU_ARGS(const uint32_t count));

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetUpdateFragmentProgramParameter)(GPU_ARGS(const uint32_t offset));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramParameterBlock)(GPU_ARGS(const uint32_t baseConst, uint32_t constCount, const float * __restrict value));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramLoad)(GPU_ARGS(const CellCgbVertexProgramConfiguration *conf, const void *ucode));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramLoadSlot)(GPU_ARGS(const uint32_t loadSlot, const uint32_t instCount, const void *ucode));
GPU_DECL GPU_VOID GPU_FUNC(SetAntiAliasingControl)(GPU_ARGS(const uint32_t enable, const uint32_t alphaToCoverage, const uint32_t alphaToOne, const uint32_t sampleMask));

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetInvalidateVertexCache)(GPU_NO_ARGS());
GPU_DECL GPU_VOID GPU_FUNC(SetInvalidateTextureCache)(GPU_ARGS(const uint32_t value));
GPU_DECL GPU_VOID GPU_FUNC(SetTexture)(GPU_ARGS(const uint8_t index, const CellGcmTexture *texture));
GPU_DECL GPU_VOID GPU_FUNC(SetTextureControl)(GPU_ARGS(const uint8_t index, const uint32_t enable, const uint16_t minlod, const uint16_t maxlod, const uint8_t maxaniso));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexTexture)(GPU_ARGS(const uint8_t index, const CellGcmTexture *texture));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexTextureControl)(GPU_ARGS(const uint8_t index, const uint32_t enable, const uint16_t minLod, const uint16_t maxLod));
GPU_DECL GPU_VOID GPU_FUNC(SetAlphaFunc)(GPU_ARGS(const uint32_t af, const uint32_t ref));
GPU_DECL GPU_VOID GPU_FUNC(SetColourWriteEnable)(GPU_ARGS(const uint32_t mask));
GPU_DECL GPU_VOID GPU_FUNC(SetWriteBackEndLabel)(GPU_ARGS(const uint8_t index, const uint32_t value));
GPU_DECL GPU_VOID GPU_FUNC(SetTimeStamp)(GPU_ARGS(const uint32_t index));

GPU_DECL GPU_VOID GPU_FUNC(SetTextureRemap)(GPU_ARGS(const uint8_t index, const uint32_t remap));

#if 0

GPU_DECL GPU_VOID GPU_FUNC(SetSurface)(GPU_ARGS(const CellGcmSurface *surface));
GPU_DECL GPU_VOID GPU_FUNC(SetSurfaceWindow)(GPU_ARGS(const CellGcmSurface *surface, const uint32_t origin, const uint32_t pixelCenter));
GPU_DECL GPU_VOID GPU_FUNC(SetWaitLabel)(GPU_ARGS(const uint8_t index, const uint32_t value));
GPU_DECL GPU_VOID GPU_FUNC(SetWriteCommandLabel)(GPU_ARGS(const uint8_t index, const uint32_t value));
GPU_DECL GPU_VOID GPU_FUNC(SetWriteTextureLabel)(GPU_ARGS(const uint8_t index, const uint32_t value));
GPU_DECL GPU_VOID GPU_FUNC(SetInvalidateZcull)(GPU_NO_ARGS());
GPU_DECL GPU_VOID GPU_FUNC(SetBlendColor)(GPU_ARGS(const uint32_t color, const uint32_t color2));
GPU_DECL GPU_VOID GPU_FUNC(SetBlendEquation)(GPU_ARGS(const uint16_t color, const uint16_t alpha));
GPU_DECL GPU_VOID GPU_FUNC(SetBlendFunc)(GPU_ARGS(const uint16_t sfcolor, const uint16_t dfcolor, const uint16_t sfalpha, const uint16_t dfalpha));
GPU_DECL GPU_VOID GPU_FUNC(SetClearSurface)(GPU_ARGS(const uint32_t mask));
GPU_DECL GPU_VOID GPU_FUNC(SetClearColor)(GPU_ARGS(const uint32_t color));
GPU_DECL GPU_VOID GPU_FUNC(SetClearDepthStencil)(GPU_ARGS(const uint32_t value));
GPU_DECL GPU_VOID GPU_FUNC(SetColorMaskMrt)(GPU_ARGS(const uint32_t mask));
GPU_DECL GPU_VOID GPU_FUNC(SetCullFace)(GPU_ARGS(const uint32_t cfm));
GPU_DECL GPU_VOID GPU_FUNC(SetDepthBounds)(GPU_ARGS(const float zmin, const float zmax));
GPU_DECL GPU_VOID GPU_FUNC(SetFrontFace)(GPU_ARGS(const uint32_t dir));
GPU_DECL GPU_VOID GPU_FUNC(SetLineWidth)(GPU_ARGS(const uint32_t width));
GPU_DECL GPU_VOID GPU_FUNC(SetLineSmoothEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetLineStippleEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetLineStipplePattern)(GPU_ARGS(const uint16_t* pattern, const uint8_t factor));
GPU_DECL GPU_VOID GPU_FUNC(SetLogicOp)(GPU_ARGS(const uint32_t op));
GPU_DECL GPU_VOID GPU_FUNC(SetPointSize)(GPU_ARGS(const float size));
GPU_DECL GPU_VOID GPU_FUNC(SetPolygonOffset)(GPU_ARGS(const float factor, const float units));
GPU_DECL GPU_VOID GPU_FUNC(SetPolySmoothEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetPolygonStippleEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetPolygonStipplePattern)(GPU_ARGS(const uint32_t* pattern));
GPU_DECL GPU_VOID GPU_FUNC(SetFrontPolygonMode)(GPU_ARGS(const uint32_t mode));
GPU_DECL GPU_VOID GPU_FUNC(SetBackPolygonMode)(GPU_ARGS(const uint32_t mode));
GPU_DECL GPU_VOID GPU_FUNC(SetScissor)(GPU_ARGS(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h));
GPU_DECL GPU_VOID GPU_FUNC(SetShadeMode)(GPU_ARGS(const uint32_t sm));
GPU_DECL GPU_VOID GPU_FUNC(SetTwoSideLightEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetStencilFunc)(GPU_ARGS(const uint32_t func, const int32_t ref, const uint32_t mask));
GPU_DECL GPU_VOID GPU_FUNC(SetBackStencilFunc)(GPU_ARGS(const uint32_t func, const int32_t ref, const uint32_t mask));
GPU_DECL GPU_VOID GPU_FUNC(SetStencilMask)(GPU_ARGS(const uint32_t sm));
GPU_DECL GPU_VOID GPU_FUNC(SetBackStencilMask)(GPU_ARGS(const uint32_t sm));
GPU_DECL GPU_VOID GPU_FUNC(SetStencilOp)(GPU_ARGS(const uint32_t fail, const uint32_t depthFail, const uint32_t depthPass));
GPU_DECL GPU_VOID GPU_FUNC(SetBackStencilOp)(GPU_ARGS(const uint32_t fail, const uint32_t depthFail, const uint32_t depthPass));
GPU_DECL GPU_VOID GPU_FUNC(SetViewport)(GPU_ARGS(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const float min, const float max, const float scale[4], const float offset[4]));
GPU_DECL GPU_VOID GPU_FUNC(SetZMinMaxControl)(GPU_ARGS(const uint32_t cullNearFarEnable, const uint32_t zclampEnable, const uint32_t cullIgnoreW));
GPU_DECL GPU_VOID GPU_FUNC(SetBlendEnableMrt)(GPU_ARGS(const uint32_t mrt1, const uint32_t mrt2, const uint32_t mrt3));
GPU_DECL GPU_VOID GPU_FUNC(SetLogicOpEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetCullFaceEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetDepthBoundsTestEnable)(GPU_ARGS(const uint32_t enable));

GPU_DECL GPU_VOID GPU_FUNC(SetDitherEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetTwoSidedStencilTestEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetPolygonOffsetFillEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetRestartIndexEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetTextureBorderColor)(GPU_ARGS(const uint8_t index, const uint32_t color));
GPU_DECL GPU_VOID GPU_FUNC(SetTextureFilter)(GPU_ARGS(const uint8_t index, const uint16_t bias, const uint8_t min, const uint8_t mag, const uint8_t conv));
GPU_DECL GPU_VOID GPU_FUNC(SetTextureAddress)(GPU_ARGS(const uint8_t index, const uint8_t wraps, const uint8_t wrapt, const uint8_t wrapr, const uint8_t unsignedRemap, const uint8_t zfunc, const uint8_t gamma));
GPU_DECL GPU_VOID GPU_FUNC(SetTextureOptimization)(GPU_ARGS(const uint8_t index, const uint8_t slope, const uint8_t iso, const uint8_t aniso));
GPU_DECL GPU_VOID GPU_FUNC(SetCylindricalWrap)(GPU_ARGS(const uint32_t enable, const uint32_t reserved));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawArrays)(GPU_ARGS(const uint8_t mode, uint32_t first, uint32_t count));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawIndexArray)(GPU_ARGS(const uint8_t mode, uint32_t count, const uint8_t type, const uint8_t location, const uint32_t indicies));
GPU_DECL GPU_VOID GPU_FUNC(SetRestartIndex)(GPU_ARGS(const uint32_t index));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexData4f)(GPU_ARGS(const uint8_t index, const float v[4]));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexDataArray)(GPU_ARGS(const uint8_t index, const uint16_t frequency, const uint8_t stride, const uint8_t size, const uint8_t type, const uint8_t location, const uint32_t offset));
GPU_DECL GPU_VOID GPU_FUNC(SetFrequencyDividerOperation)(GPU_ARGS(const uint16_t operation));
GPU_DECL GPU_VOID GPU_FUNC(SetTransformBranchBits)(GPU_ARGS(const uint32_t branchBits));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexAttribInputMask)(GPU_ARGS(const uint16_t mask));
GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramGammaEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetUserClipPlaneControl)(GPU_ARGS(const uint32_t plane0, const uint32_t plane1, const uint32_t plane2, const uint32_t plane3, const uint32_t plane4, const uint32_t plane5));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramConstants)(GPU_ARGS(const uint32_t start, const uint32_t count, const float *data));
GPU_DECL GPU_VOID GPU_FUNC(SetInlineTransfer)(GPU_ARGS(const uint32_t dstOffset, const void *srcAdr, const uint32_t sizeInWords));
GPU_DECL GPU_VOID GPU_FUNC(InlineTransfer)(GPU_ARGS(const uint32_t dstOffset, const void *srcAdr, const uint32_t sizeInWords, const uint8_t location));
GPU_DECL uint32_t GPU_FUNC(SetTransferImage)(GPU_ARGS(const uint8_t mode, const uint32_t dstOffset, const uint32_t dstPitch, const uint32_t dstX, const uint32_t dstY, const uint32_t srcOffset, const uint32_t srcPitch, const uint32_t srcX, const uint32_t srcY, const uint32_t width, const uint32_t height, const uint32_t bytesPerPixel));
GPU_DECL GPU_VOID GPU_FUNC(TransferData)(GPU_ARGS(uint32_t dstOffset, int32_t dstPitch, uint32_t srcOffset, int32_t srcPitch, int32_t bytesPerRow, int32_t rowCount));
GPU_DECL uint32_t GPU_FUNC(SetTransferData)(GPU_ARGS(const uint8_t mode, const uint32_t dstOffset, const uint32_t dstPitch, const uint32_t srcOffset, const uint32_t srcPitch, const uint32_t bytesPerRow, const uint32_t rowCount));
GPU_DECL GPU_VOID GPU_FUNC(SetRenderEnable)(GPU_ARGS(const uint8_t mode, const uint32_t index));
GPU_DECL GPU_VOID GPU_FUNC(SetZpassPixelCountEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetClearReport)(GPU_ARGS(const uint32_t type));
GPU_DECL GPU_VOID GPU_FUNC(SetReport)(GPU_ARGS(const uint32_t type, const uint32_t index));
GPU_DECL GPU_VOID GPU_FUNC(SetZcullStatsEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetZcullControl)(GPU_ARGS(const uint8_t zCullDir, const uint8_t zCullFormat));
GPU_DECL GPU_VOID GPU_FUNC(SetZcullLimit)(GPU_ARGS(const uint16_t moveForwardLimit, const uint16_t pushBackLimit));
GPU_DECL GPU_VOID GPU_FUNC(SetScullControl)(GPU_ARGS(const uint8_t sFunc, const uint8_t sRef, const uint8_t sMask));
GPU_DECL GPU_VOID GPU_FUNC(SetConvertSwizzleFormat)(GPU_ARGS(uint32_t dstOffset, uint32_t dstWidth, uint32_t dstHeight, uint32_t dstX, uint32_t dstY, uint32_t srcOffset, uint32_t srcPitch, uint32_t srcX, uint32_t srcY, uint32_t width, uint32_t height, uint32_t bytesPerPixel, uint8_t mode));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexTextureAddress)(GPU_ARGS(const uint8_t index, const uint8_t wraps, const uint8_t wrapt));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexTextureFilter)(GPU_ARGS(const uint8_t index, const uint16_t bias));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexTextureBorderColor)(GPU_ARGS(const uint8_t index, const uint32_t color));
GPU_DECL GPU_VOID GPU_FUNC(SetPerfMonTrigger)(GPU_NO_ARGS());
GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineArray)(GPU_ARGS(const uint8_t mode, const uint32_t count, const void *data));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray32)(GPU_ARGS(const uint8_t mode, const uint32_t start, const uint32_t count, const uint32_t *data));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray16)(GPU_ARGS(const uint8_t mode, const uint32_t start, const uint32_t count, const uint16_t *data));
GPU_DECL GPU_VOID GPU_FUNC(SetFogMode)(GPU_ARGS(const uint32_t mode));
GPU_DECL GPU_VOID GPU_FUNC(SetFogParams)(GPU_ARGS(const float p0, const float p1));
GPU_DECL GPU_VOID GPU_FUNC(SetTransferLocation)(GPU_ARGS(const uint32_t location));
GPU_DECL GPU_VOID GPU_FUNC(SetDepthFormat)(GPU_ARGS(const uint32_t format));
GPU_DECL GPU_VOID GPU_FUNC(SetBlendOptimization)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetPolygonOffsetLineEnable)(GPU_ARGS(const uint32_t enable));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexAttribOutputMask)(GPU_ARGS(const uint32_t mask));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramStartSlot)(GPU_ARGS(const uint32_t startSlot));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramRegisterCount)(GPU_ARGS(const uint32_t registerCount));
GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramLoad)(GPU_ARGS(const CellCgbFragmentProgramConfiguration *conf));
GPU_DECL GPU_VOID GPU_FUNC(SetInlineTransferPointer)(GPU_ARGS(const uint32_t offset, const uint32_t count, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetTransferDataMode)(GPU_ARGS(const uint8_t mode));
GPU_DECL GPU_VOID GPU_FUNC(SetTransferDataFormat)(GPU_ARGS(const int32_t inPitch, const int32_t outPitch, const uint32_t lineLength, const uint32_t lineCount, const uint8_t inFormat, const uint8_t outFormat));
GPU_DECL GPU_VOID GPU_FUNC(SetTransferDataOffset)(GPU_ARGS(const uint32_t dstOffset, const uint32_t srcOffset));
GPU_DECL GPU_VOID GPU_FUNC(SetTransferScaleMode)(GPU_ARGS(const uint8_t mode, const uint8_t surface));
GPU_DECL GPU_VOID GPU_FUNC(SetTransferScaleSurface)(GPU_ARGS(const CellGcmTransferScale *scale, CellGcmTransferSurface *surface));
GPU_DECL GPU_VOID GPU_FUNC(SetTransferScaleSwizzle)(GPU_ARGS(const CellGcmTransferScale *scale, CellGcmTransferSwizzle *swizzle));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawBegin)(GPU_ARGS(const uint8_t mode));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawEnd)(GPU_NO_ARGS());
GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineArrayPointer)(GPU_ARGS(const uint32_t count, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramConstantsPointer)(GPU_ARGS(uint32_t first, uint32_t count, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray32Pointer)(GPU_ARGS(const uint32_t count, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray16Pointer)(GPU_ARGS(const uint32_t count, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetAnisoSpread)(GPU_ARGS(const uint8_t index, const uint8_t reduceSamplesEnable, const uint8_t hReduceSamplesEnable, const uint8_t vReduceSamplesEnable, const uint8_t spacingSelect, const uint8_t hSpacingSelect, const uint8_t vSpacingSelect));
GPU_DECL GPU_VOID GPU_FUNC(SetTextureAddressAnisoBias)(GPU_ARGS(const uint8_t index, const uint8_t wraps, const uint8_t wrapt, const uint8_t wrapr, const uint8_t unsignedRemap, const uint8_t zfunc, const uint8_t gamma, const uint8_t anisoBias));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramParameterBlockPointer)(GPU_ARGS(uint32_t baseConst, uint32_t constCount, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexDataArrayFormat)(GPU_ARGS(const uint8_t index, const uint16_t frequency, const uint8_t stride, const uint8_t size, const uint8_t type));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexDataArrayOffset)(GPU_ARGS(const uint8_t index, const uint8_t location, const uint32_t offset));
GPU_DECL GPU_VOID GPU_FUNC(SetInlineTransferAlignedPointer)(GPU_ARGS(const uint32_t offset, const uint32_t count, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramConstantsAlignedPointer)(GPU_ARGS(uint32_t first, uint32_t count, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramParameterBlockAlignedPointer)(GPU_ARGS(const uint32_t baseConst, const uint32_t constCount, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineArrayAlignedPointer)(GPU_ARGS(const uint32_t count, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray32AlignedPointer)(GPU_ARGS(const uint32_t count, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray16AlignedPointer)(GPU_ARGS(const uint32_t count, void **pointer));
GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramLoadLocation)(GPU_ARGS(const CellCgbFragmentProgramConfiguration *conf, const uint32_t location));
GPU_DECL GPU_VOID GPU_FUNC(SetUpdateFragmentProgramParameterLocation)(GPU_ARGS(const uint32_t offset, const uint32_t location));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexDataBase)(GPU_ARGS(const uint32_t baseOffset, const uint32_t baseIndex));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramParameter)(GPU_ARGS(const CGparameter param, const float *value));
GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgram)(GPU_ARGS(const CGprogram prog, const void *ucode));
GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramOffset)(GPU_ARGS(const CGprogram prog, const uint32_t offset, const uint32_t location));
GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramControl)(GPU_ARGS(const CGprogram prog, const uint8_t controlTxp, const uint8_t reserved0, const uint8_t reserved1));
GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgram)(GPU_ARGS(const CGprogram prog, uint32_t offset));
GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramParameter)(GPU_ARGS(const CGprogram prog, const CGparameter param, const float *value, const uint32_t offset));
GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramParameterPointer)(GPU_ARGS(const CGprogram prog, const CGparameter param, const uint32_t offset, const uint32_t srcOffset));
GPU_DECL GPU_VOID GPU_FUNC(SetClearZcullSurface)(GPU_ARGS(const uint32_t depth, const uint32_t stencil));
GPU_DECL GPU_VOID GPU_FUNC(SetZcullEnable)(GPU_ARGS(const uint32_t depth, const uint32_t stencil));
GPU_DECL GPU_VOID GPU_FUNC(SetTransferReportData)(GPU_ARGS(const uint32_t offset, const uint32_t index, const uint32_t count));
GPU_DECL GPU_VOID GPU_FUNC(SetWriteBackEndLabelForConditional)(GPU_ARGS(uint8_t index, uint32_t value));
GPU_DECL GPU_VOID GPU_FUNC(SetWriteTextureLabelForConditional)(GPU_ARGS(uint8_t index, uint32_t value));
GPU_DECL GPU_VOID GPU_FUNC(SetUserCommand)(GPU_ARGS(const uint32_t cause));
GPU_DECL GPU_VOID GPU_FUNC(SetWaitFlip)(GPU_NO_ARGS());
GPU_DECL int32_t GPU_FUNC(SetFlip)(GPU_ARGS(const uint8_t id));
GPU_DECL GPU_VOID GPU_FUNC(ReserveMethodSize)(GPU_ARGS(uint32_t size));

#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------

#undef GPU_FUNC
#undef GPU_DECL
