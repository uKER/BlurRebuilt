#if CFG_DEBUG
	#define GPU_DECL				INLINE
#else
	#define GPU_DECL				FORCE_INLINE
#endif

#define GPU_FUNC(_func)				GPU_CLASS::_func
#define GCM_ARGS(...)				GetData(), __VA_ARGS__
#define GCM_NO_ARGS()				GetData()

#if GPU_INLINE
	#if GPU_MEASURE
		#define GCM_FUNC(_func)		cellGcm ## _func ## MeasureInline
	#elif GPU_UNSAFE
		#define GCM_FUNC(_func)		cellGcm ## _func ## UnsafeInline
	#else
		#define GCM_FUNC(_func)		cellGcm ## _func ## Inline
	#endif
#else
	#if GPU_MEASURE
		#define GCM_FUNC(_func) 	cellGcm ## _func ## Measure
	#elif GPU_UNSAFE
		#define GCM_FUNC(_func) 	cellGcm ## _func ## Unsafe
	#else
		#define GCM_FUNC(_func) 	cell::Gcm::Inline::cellGcm ## _func
	#endif
#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------

#if GPU_CHECK || GPU_TRACK
GPU_DECL const C_AlphaTestState& GPU_FUNC(GetAlphaTestState)() const
{
	return m_pState->m_AlphaTestState;
}

GPU_DECL const C_BlendState& GPU_FUNC(GetBlendState)() const
{
	return m_pState->m_BlendState;
}

GPU_DECL const C_ColourState& GPU_FUNC(GetColourState)() const
{
	return m_pState->m_ColourState;
}

GPU_DECL const C_CullState& GPU_FUNC(GetCullState)() const
{
	return m_pState->m_CullState;
}

GPU_DECL const C_DepthState& GPU_FUNC(GetDepthState)() const
{
	return m_pState->m_DepthState;
}

GPU_DECL const C_FillState& GPU_FUNC(GetFillState)() const
{
	return m_pState->m_FillState;
}

GPU_DECL const C_SamplerState& GPU_FUNC(GetSamplerState)(const U32 i) const
{
	return m_pState->m_SamplerState[i];
}

GPU_DECL const C_ScissorState& GPU_FUNC(GetScissorState)() const
{
	return m_pState->m_ScissorState;
}

GPU_DECL const C_StencilState& GPU_FUNC(GetStencilState)() const
{
	return m_pState->m_StencilState;
}

GPU_DECL const C_VertexSamplerState& GPU_FUNC(GetVertexSamplerState)(const U32 i) const
{
	return m_pState->m_VertexSamplerState[i];
}
#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetAlphaTestState)(const C_AlphaTestState& state)
{
	#if GPU_CHECK
	if(state.m_AlphaTestBits != m_pState->m_AlphaTestState.m_AlphaTestBits)
	#endif
	{
		GPU_ASSERT_CMP_FUNC(state.m_AlphaTestCmpFunc);

		GCM_FUNC(SetAlphaFunc)(GCM_ARGS(
			state.m_AlphaTestCmpFunc,
			state.m_AlphaTestRef));

		#if GPU_TRACK
		m_pState->m_AlphaTestState.m_AlphaTestBits = state.m_AlphaTestBits;
		#endif
	}
	
	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBlendState)(const C_BlendState& state)
{
	#if GPU_CHECK
	if(state.m_BlendColourSrc != m_pState->m_BlendState.m_BlendColourSrc ||
		state.m_BlendColourDst != m_pState->m_BlendState.m_BlendColourDst ||
		state.m_BlendAlphaSrc != m_pState->m_BlendState.m_BlendAlphaSrc ||
		state.m_BlendAlphaDst != m_pState->m_BlendState.m_BlendAlphaDst)
	#endif
	{
		GPU_ASSERT_BLEND(state.m_BlendColourSrc);
		GPU_ASSERT_BLEND(state.m_BlendColourDst);
		GPU_ASSERT_BLEND(state.m_BlendAlphaSrc);
		GPU_ASSERT_BLEND(state.m_BlendAlphaDst);

		GCM_FUNC(SetBlendFunc)(GCM_ARGS(
			state.m_BlendColourSrc,
			state.m_BlendColourDst,
			state.m_BlendAlphaSrc,
			state.m_BlendAlphaDst));

		#if GPU_TRACK
		m_pState->m_BlendState.m_BlendColourSrc = state.m_BlendColourSrc;
		m_pState->m_BlendState.m_BlendColourDst = state.m_BlendColourDst;
		m_pState->m_BlendState.m_BlendAlphaSrc = state.m_BlendAlphaSrc;
		m_pState->m_BlendState.m_BlendAlphaDst = state.m_BlendAlphaDst;
		#endif
	}

	#if GPU_CHECK
	if(state.m_BlendColourOp != m_pState->m_BlendState.m_BlendColourOp ||
		state.m_BlendAlphaOp != m_pState->m_BlendState.m_BlendAlphaOp)
	#endif
	{
		GPU_ASSERT_BLEND_OP(state.m_BlendColourOp);
		GPU_ASSERT_BLEND_OP(state.m_BlendAlphaOp);

		GCM_FUNC(SetBlendEquation)(GCM_ARGS(
			state.m_BlendColourOp,
			state.m_BlendAlphaOp));
		
		#if GPU_TRACK
		m_pState->m_BlendState.m_BlendColourOp = state.m_BlendColourOp;
		m_pState->m_BlendState.m_BlendAlphaOp = state.m_BlendAlphaOp;
		#endif
	}
	
	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetColourState)(const C_ColourState& state)
{
	#if GPU_CHECK
	if(state.m_ColourWriteEnable != m_pState->m_ColourState.m_ColourWriteEnable)
	#endif
	{
		GCM_FUNC(SetColorMask)(GCM_ARGS(
			state.m_ColourWriteEnable));

		#if GPU_TRACK
		m_pState->m_ColourState.m_ColourWriteEnable = state.m_ColourWriteEnable;
		#endif
	}

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetCullState)(const C_CullState& state)
{
	#if GPU_CHECK
	if(state.m_CullBits != m_pState->m_CullState.m_CullBits)
	#endif
	{
		#if GPU_CHECK
		if(state.m_CullEnable != m_pState->m_CullState.m_CullEnable)
		#endif
		{
			GPU_ASSERT_BOOL(state.m_CullEnable);

			GCM_FUNC(SetCullFaceEnable)(GCM_ARGS(
				state.m_CullEnable));
		}

		#if GPU_CHECK
		if(state.m_CullMode != E_CullMode_None && state.m_CullMode != m_pState->m_CullState.m_CullMode)
		#else
		if(state.m_CullMode != E_CullMode_None)
		#endif
		{
			GPU_ASSERT_CULL_MODE(state.m_CullMode);

			GCM_FUNC(SetFrontFace)(GCM_ARGS(
				state.m_CullMode));
		}

		#if GPU_TRACK
		m_pState->m_CullState.m_CullBits = state.m_CullBits;
		#endif
	}

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDepthState)(const C_DepthState& state)
{
	#if GPU_CHECK
	if(state.m_DepthBits != m_pState->m_DepthState.m_DepthBits)
	#endif
	{
		#if GPU_CHECK
		if(state.m_DepthFunc != m_pState->m_DepthState.m_DepthFunc)
		#endif
		{
			GPU_ASSERT_CMP_FUNC(state.m_DepthFunc);

			GCM_FUNC(SetDepthFunc)(GCM_ARGS(
				state.m_DepthFunc));
		}

		#if GPU_CHECK
		if(state.m_DepthEnable != m_pState->m_DepthState.m_DepthEnable)
		#endif
		{
			GPU_ASSERT_BOOL(state.m_DepthEnable);

			GCM_FUNC(SetDepthTestEnable)(GCM_ARGS(
				state.m_DepthEnable));
		}

		#if GPU_CHECK
		if(state.m_DepthWriteEnable != m_pState->m_DepthState.m_DepthWriteEnable)
		#endif
		{
			GPU_ASSERT_BOOL(state.m_DepthWriteEnable);

			GCM_FUNC(SetDepthMask)(GCM_ARGS(
				state.m_DepthWriteEnable));
		}

		#if GPU_TRACK
		m_pState->m_DepthState.m_DepthBits = state.m_DepthBits;
		#endif
	}

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFillState)(const C_FillState& state)
{
	#if GPU_CHECK
	if(state.m_FillMode != m_pState->m_FillState.m_FillMode)
	#endif
	{
		GPU_ASSERT_FILL_MODE(state.m_FillMode);

		GCM_FUNC(SetFrontPolygonMode)(GCM_ARGS(
			state.m_FillMode));

		#if GPU_TRACK
		m_pState->m_FillState.m_FillMode = state.m_FillMode;
		#endif
	}

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTextureState)(const U32 index, const C_TextureState& state)
{
	BIZ_ASSERT(index < E_Sampler_Count && index < 16);

	#if GPU_CHECK
	if(state.m_pTexture != m_pState->m_TextureState[index].m_pTexture)
	#endif
	{
		if(state.m_pTexture)
		{
			//GCM_FUNC(SetInvalidateTextureCache)(
				//GCM_ARGS(CELL_GCM_INVALIDATE_TEXTURE));

#if 1
			SetTexture(
				index,
				&state.m_pTexture->m_gcmTexture);
#else
			GCM_FUNC(SetTexture)(GCM_ARGS(
				index,
				&state.m_pTexture->m_gcmTexture));
#endif
		}

		#if GPU_TRACK
		m_pState->m_TextureState[index].m_pTexture = state.m_pTexture;
		#endif
	}

	#if GPU_CHECK
	if(state.m_SetTextureControlBits != m_pState->m_TextureState[index].m_SetTextureControlBits)
	#endif
	{
		GPU_ASSERT_BOOL(state.m_TextureControlEnable);
		GPU_ASSERT(state.m_TextureControlMinLod <= 12);
		GPU_ASSERT(state.m_TextureControlMaxLod <= 12);
		GPU_ASSERT(state.m_TextureControlMaxAniso <= 8);

		GCM_FUNC(SetTextureControl)(GCM_ARGS(
			index,
			state.m_TextureControlEnable,
			state.m_TextureControlMinLod<<8,
			state.m_TextureControlMaxLod<<8,
			state.m_TextureControlMaxAniso));

		#if GPU_TRACK
		m_pState->m_TextureState[index].m_SetTextureControlBits = state.m_SetTextureControlBits;
		#endif
	}
	
	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTextureState_TextureControl)(const U32 index, const C_TextureState& state)
{
	BIZ_ASSERT(index < E_Sampler_Count && index < 16);

	#if GPU_CHECK
	if(state.m_SetTextureControlBits != m_pState->m_TextureState[index].m_SetTextureControlBits)
	#endif
	{
		GPU_ASSERT_BOOL(state.m_TextureControlEnable);
		GPU_ASSERT(state.m_TextureControlMinLod <= 12);
		GPU_ASSERT(state.m_TextureControlMaxLod <= 12);
		GPU_ASSERT(state.m_TextureControlMaxAniso <= 8);

		GCM_FUNC(SetTextureControl)(GCM_ARGS(
			index,
			state.m_TextureControlEnable,
			state.m_TextureControlMinLod<<8,
			state.m_TextureControlMaxLod<<8,
			state.m_TextureControlMaxAniso));

		#if GPU_TRACK
		m_pState->m_TextureState[index].m_SetTextureControlBits = state.m_SetTextureControlBits;
		#endif
	}
	
	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetSamplerState)(const U32 index, const C_SamplerState& state)
{
	BIZ_ASSERT(index < E_Sampler_Count && index < 16);

	#if GPU_CHECK
	if(state.m_SetTextureAddressBits != m_pState->m_SamplerState[index].m_SetTextureAddressBits)
	#endif
	{
		GPU_ASSERT_TEXTURE_ADDRESS(state.m_TextureAddressU);
		GPU_ASSERT_TEXTURE_ADDRESS(state.m_TextureAddressV);
		GPU_ASSERT_TEXTURE_ADDRESS(state.m_TextureAddressW);
		GPU_ASSERT_TEXTURE_Z_FUNC(state.m_TextureZFunc);

		GCM_FUNC(SetTextureAddress)(GCM_ARGS(
			index,
			state.m_TextureAddressU,
			state.m_TextureAddressV,
			state.m_TextureAddressW,
			state.m_TextureUnsignedRemap,
			state.m_TextureZFunc,
			state.m_TextureGamma));

		#if GPU_TRACK
		m_pState->m_SamplerState[index].m_SetTextureAddressBits = state.m_SetTextureAddressBits;
		#endif
	}

	#if GPU_CHECK
	if(state.m_TextureBorderColour != m_pState->m_SamplerState[index].m_TextureBorderColour)
	#endif
	{
		GCM_FUNC(SetTextureBorderColor)(GCM_ARGS(
			index,
			state.m_TextureBorderColour));

		#if GPU_TRACK
		m_pState->m_SamplerState[index].m_TextureBorderColour = state.m_TextureBorderColour;
		#endif
	}

	#if GPU_CHECK
	if(state.m_SetTextureFilterBits != m_pState->m_SamplerState[index].m_SetTextureFilterBits)
	#endif
	{
		GPU_ASSERT_TEXTURE_FILTER_MIN(state.m_TextureFilterMin);
		GPU_ASSERT_TEXTURE_FILTER_MAG(state.m_TextureFilterMag);
		GPU_ASSERT_TEXTURE_FILTER_CONV(state.m_TextureFilterConv);

		//BIZ_ASSERT((S32)state.m_TextureFilterBias >= SCHAR_MIN && (S32)state.m_TextureFilterBias <= SCHAR_MAX);

		GCM_FUNC(SetTextureFilter)(GCM_ARGS(
			index,
			(S16)((((F32)state.m_TextureFilterBias/128.0f)*8.0f)*(F32)(1<<8)) & 0x1fff,
			state.m_TextureFilterMin,
			state.m_TextureFilterMag,
			state.m_TextureFilterConv));

		#if GPU_TRACK
		m_pState->m_SamplerState[index].m_SetTextureFilterBits = state.m_SetTextureFilterBits;
		#endif
	}
	
	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetScissorState)(const C_ScissorState& state)
{
	GCM_FUNC(SetScissor)(GCM_ARGS(
		state.m_ScissorRectX,
		state.m_ScissorRectY,
		state.m_ScissorRectW,
		state.m_ScissorRectH));
	
	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetStencilState)(const C_StencilState& state)
{
	#if GPU_CHECK
	if(state.m_StencilFunc != m_pState->m_StencilState.m_StencilFunc ||
		state.m_StencilRef != m_pState->m_StencilState.m_StencilRef ||
		state.m_StencilMask != m_pState->m_StencilState.m_StencilMask)
	#endif
	{
		GPU_ASSERT_CMP_FUNC(state.m_StencilFunc);
		
		GCM_FUNC(SetStencilFunc)(GCM_ARGS(
			state.m_StencilFunc,
			state.m_StencilRef,
			state.m_StencilMask));

		#if GPU_TRACK
		m_pState->m_StencilState.m_StencilFunc = state.m_StencilFunc;
		m_pState->m_StencilState.m_StencilRef = state.m_StencilRef;
		m_pState->m_StencilState.m_StencilMask = state.m_StencilMask;
		#endif
	}

	#if GPU_CHECK
	if(state.m_StencilWriteMask != m_pState->m_StencilState.m_StencilWriteMask)
	#endif
	{
		GCM_FUNC(SetStencilMask)(GCM_ARGS(
			state.m_StencilWriteMask));

		#if GPU_TRACK
		m_pState->m_StencilState.m_StencilWriteMask = state.m_StencilWriteMask;
		#endif
	}

	#if GPU_CHECK
	if(state.m_StencilFail != m_pState->m_StencilState.m_StencilFail ||
		state.m_StencilDepthFail != m_pState->m_StencilState.m_StencilDepthFail ||
		state.m_StencilDepthPass != m_pState->m_StencilState.m_StencilDepthPass)
	#endif
	{
		GPU_ASSERT_STENCIL_OP(state.m_StencilFail);
		GPU_ASSERT_STENCIL_OP(state.m_StencilDepthFail);
		GPU_ASSERT_STENCIL_OP(state.m_StencilDepthPass);
		
		GCM_FUNC(SetStencilOp)(GCM_ARGS(
			state.m_StencilFail,
			state.m_StencilDepthFail,
			state.m_StencilDepthPass));

		#if GPU_TRACK
		m_pState->m_StencilState.m_StencilFail = state.m_StencilFail;
		m_pState->m_StencilState.m_StencilDepthFail = state.m_StencilDepthFail;
		m_pState->m_StencilState.m_StencilDepthPass = state.m_StencilDepthPass;
		#endif
	}

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexTextureState)(const U32 index, const C_VertexTextureState& state)
{
	BIZ_ASSERT(index < E_VertexSampler_Count && index <= 3);

	#if GPU_CHECK
	if(state.m_pTexture != m_pState->m_VertexTextureState[index].m_pTexture)
	#endif
	{
//		GCM_FUNC(SetInvalidateTextureCache)(
//			GCM_ARGS(CELL_GCM_INVALIDATE_TEXTURE));

		GCM_FUNC(SetVertexTexture)(GCM_ARGS(
			index,
			&state.m_pTexture->m_gcmTexture));

		#if GPU_TRACK
		m_pState->m_VertexTextureState[index].m_pTexture = state.m_pTexture;
		#endif
	}

	#if GPU_CHECK
	if(state.m_SetTextureControlBits != m_pState->m_VertexTextureState[index].m_SetTextureControlBits)
	#endif
	{
		GPU_ASSERT_BOOL(state.m_TextureControlEnable);
		GPU_ASSERT(state.m_TextureControlMinLod <= 12);
		GPU_ASSERT(state.m_TextureControlMaxLod <= 12);

		GCM_FUNC(SetVertexTextureControl)(GCM_ARGS(
			index,
			state.m_TextureControlEnable,
			state.m_TextureControlMinLod<<8,
			state.m_TextureControlMaxLod<<8));

		#if GPU_TRACK
		m_pState->m_VertexTextureState[index].m_SetTextureControlBits = state.m_SetTextureControlBits;
		#endif
	}
	
	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexSamplerState)(const U32 index, const C_VertexSamplerState& state)
{
	BIZ_ASSERT(index < E_VertexSampler_Count && index <= 3);

	#if GPU_CHECK
	if(state.m_SetTextureAddressBits != m_pState->m_VertexSamplerState[index].m_SetTextureAddressBits)
	#endif
	{
		GPU_ASSERT_TEXTURE_ADDRESS(state.m_TextureAddressU);
		GPU_ASSERT_TEXTURE_ADDRESS(state.m_TextureAddressV);

		GCM_FUNC(SetVertexTextureAddress)(GCM_ARGS(
			index,
			state.m_TextureAddressU,
			state.m_TextureAddressV));

		#if GPU_TRACK
		m_pState->m_VertexSamplerState[index].m_SetTextureAddressBits = state.m_SetTextureAddressBits;
		#endif
	}

	#if GPU_CHECK
	if(state.m_TextureBorderColour != m_pState->m_VertexSamplerState[index].m_TextureBorderColour)
	#endif
	{
		GCM_FUNC(SetVertexTextureBorderColor)(GCM_ARGS(
			index,
			state.m_TextureBorderColour));

		#if GPU_TRACK
		m_pState->m_VertexSamplerState[index].m_TextureBorderColour = state.m_TextureBorderColour;
		#endif
	}

	#if GPU_CHECK
	if(state.m_SetTextureFilterBits != m_pState->m_VertexSamplerState[index].m_SetTextureFilterBits)
	#endif
	{
		GCM_FUNC(SetVertexTextureFilter)(GCM_ARGS(
			index,
			(S16)((((F32)state.m_TextureFilterBias/128.0f)*8.0f)*(F32)(1<<8))));

		#if GPU_TRACK
		m_pState->m_VertexSamplerState[index].m_SetTextureFilterBits = state.m_SetTextureFilterBits;
		#endif
	}
	
	GPU_RETURN_VOID;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetAlphaTestEnable)(GPU_ARGS(const uint32_t enable))
{
	#if GPU_CHECK
	if(enable != m_pState->m_AlphaTestEnable)
	#endif
	{
		GPU_ASSERT_BOOL(enable);

		GCM_FUNC(SetAlphaTestEnable)(GCM_ARGS(
			enable));

		#if GPU_TRACK
		m_pState->m_AlphaTestEnable = enable;
		#endif
	}

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBlendEnable)(GPU_ARGS(const uint32_t enable))
{
	#if GPU_CHECK
	if(enable != m_pState->m_BlendEnable)
	#endif
	{
		GPU_ASSERT_BOOL(enable);

		GCM_FUNC(SetBlendEnable)(GCM_ARGS(
			enable));

		#if GPU_TRACK
		m_pState->m_BlendEnable = enable;
		#endif
	}
	
	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetStencilEnable)(GPU_ARGS(const uint32_t enable))
{
	#if GPU_CHECK
	if(enable != m_pState->m_StencilEnable)
	#endif
	{
		GPU_ASSERT_BOOL(enable);

		GCM_FUNC(SetStencilTestEnable)(GCM_ARGS(
			enable));

		#if GPU_TRACK
		m_pState->m_StencilEnable = enable;
		#endif
	}
	
	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetPointSpriteEnable)(GPU_ARGS(const uint32_t enable))
{
	#if GPU_CHECK
	if(enable != m_pState->m_PointSpriteEnable)
	#endif
	{
		GPU_ASSERT_BOOL(enable);

		GCM_FUNC(SetPointSpriteControl)(GCM_ARGS(enable, CELL_GCM_POINT_SPRITE_RMODE_ZERO, CELL_GCM_POINT_SPRITE_TEX0));

		#if GPU_TRACK
		m_pState->m_PointSpriteEnable = enable;
		#endif
	}
	
	GPU_RETURN_VOID;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetDepthWriteEnable)(GPU_ARGS(const uint32_t enable))
{
	GPU_ASSERT_BOOL(enable);

	GCM_FUNC(SetDepthMask)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDepthFunc)(GPU_ARGS(const uint32_t zf))
{
	GCM_FUNC(SetDepthFunc)(GCM_ARGS(zf));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDepthEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetDepthTestEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFillMode)(GPU_ARGS(const uint32_t mode))
{
	GPU_ASSERT_FILL_MODE(mode);

	GCM_FUNC(SetFrontPolygonMode)(GCM_ARGS(mode));

	GPU_RETURN_VOID;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(DrawPrimitive(GPU_ARGS(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount)))
{
	GCM_FUNC(SetDrawArrays)(GCM_ARGS(primitiveType, startVertex, vertexCount));
}

GPU_DECL GPU_VOID GPU_FUNC(DrawIndexedPrimitive(GPU_ARGS(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount)))
{
	#if GPU_CHECK || GPU_TRACK
	BIZ_ASSERT(m_pState->m_pIndexBuffer);
	DrawIndexedPrimitive(
		primitiveType,
		baseVertex,
		minIndex,
		vertexCount,
		startIndex,
		indexCount,
		m_pState->m_pIndexBuffer->m_Format,
		FlagsToLocation(m_pState->m_pIndexBuffer->m_Flags),
		m_pState->m_pIndexBuffer->m_Offset);
	#else
	BIZ_ASSERT(0 && "Funcionality not available on a context with no checking or tracking");
	#endif
}

GPU_DECL GPU_VOID GPU_FUNC(DrawIndexedPrimitive(GPU_ARGS(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount, const C_IndexBuffer* pIndexBuffer)))
{
	BIZ_ASSERT(pIndexBuffer);
	DrawIndexedPrimitive(
		primitiveType,
		baseVertex,
		minIndex,
		vertexCount,
		startIndex,
		indexCount,
		pIndexBuffer->m_Format,
		FlagsToLocation(pIndexBuffer->m_Flags),
		pIndexBuffer->m_Offset);
}

GPU_DECL GPU_VOID GPU_FUNC(DrawIndexedPrimitive(GPU_ARGS(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount, const U32 indexFormat, const U32 indexLocation, const U32 indexOffset)))
{
	GPU_ASSERT_PRIMITIVE_TYPE(primitiveType);
	GPU_ASSERT_INDEX_FORMAT(indexFormat);
	GPU_ASSERT_LOCATION(indexLocation);

	const U32 actualIndexOffset = indexOffset + (startIndex * (indexFormat == E_IndexFormat_16 ? sizeof(U16) : sizeof(U32)));

	GPU_ASSERT(actualIndexOffset % 2 == 0);

	// TODO: Check upper 3 bits of actualIndexOffset are 0?
	// TODO: Check index value it actually points to must be up until 0xfffff?

	GCM_FUNC(SetDrawIndexArray)(GCM_ARGS(primitiveType, indexCount, indexFormat, indexLocation, actualIndexOffset));
}

GPU_DECL GPU_VOID GPU_FUNC(SetIndexBuffer(GPU_ARGS(const C_IndexBuffer* pIndexBuffer)))
{
	#if GPU_CHECK
	if(pIndexBuffer == m_pState->m_pIndexBuffer)
		GPU_RETURN_OK;
	#endif

	#if GPU_TRACK
	m_pState->m_pIndexBuffer = pIndexBuffer;
	#endif
}

GPU_DECL GPU_VOID GPU_FUNC(SetPixelShader(GPU_ARGS(const C_PixelShader* pPixelShader)))
{
	#if GPU_CHECK
	if(pPixelShader == m_pState->m_pPixelShader)
		GPU_RETURN_OK;
	#endif

	#if GPU_TRACK
	m_pState->m_pPixelShader = pPixelShader;
	#endif

	if(!pPixelShader)
		return;

	BIZ_ASSERT(pPixelShader->IsCreated());

	//void* pAddress = OffsetToAddress(FlagsToLocation(pPixelShader->m_Flags), pPixelShader->m_ProgramConfig.offset);
	
	//if(IsMainAddress(pAddress))
		//GCM_FUNC(SetFragmentProgramLoadLocation(GCM_ARGS(&pPixelShader->m_ProgramConfig, CELL_GCM_LOCATION_MAIN)));
	//else
		GCM_FUNC(SetFragmentProgramLoadLocation)(GCM_ARGS(&pPixelShader->m_ProgramConfig, CELL_GCM_LOCATION_LOCAL));

	//GCM_FUNC(SetUpdateFragmentProgramParameter)(GCM_ARGS(pPixelShader->m_ProgramConfig.offset));

	//GCM_FUNC(SetInvalidateTextureCache)(GCM_ARGS(CELL_GCM_INVALIDATE_TEXTURE));
}

GPU_DECL GPU_VOID GPU_FUNC(SetPixelShader(GPU_ARGS(const C_PixelShader* pPixelShader, U32 offset)))
{
	#if GPU_CHECK
	if(pPixelShader == m_pState->m_pPixelShader)
		GPU_RETURN_OK;
	#endif

	#if GPU_TRACK
	m_pState->m_pPixelShader = pPixelShader;
	#endif

	if(!pPixelShader)
		return;

	BIZ_ASSERT(pPixelShader->IsCreated());

	CellCgbFragmentProgramConfiguration cgbProgramConfig;

	cgbProgramConfig.offset					= offset;
	cgbProgramConfig.attributeInputMask 	= pPixelShader->m_ProgramConfig.attributeInputMask;
	cgbProgramConfig.texCoordsInputMask 	= pPixelShader->m_ProgramConfig.texCoordsInputMask;
	cgbProgramConfig.texCoords2D			= pPixelShader->m_ProgramConfig.texCoords2D;
	cgbProgramConfig.texCoordsCentroid		= pPixelShader->m_ProgramConfig.texCoordsCentroid;
	cgbProgramConfig.fragmentControl		= pPixelShader->m_ProgramConfig.fragmentControl;
	cgbProgramConfig.registerCount			= pPixelShader->m_ProgramConfig.registerCount;
		
	GCM_FUNC(SetFragmentProgramLoadLocation)(GCM_ARGS(&cgbProgramConfig, CELL_GCM_LOCATION_LOCAL));

//	GCM_FUNC(SetInvalidateTextureCache)(GCM_ARGS(CELL_GCM_INVALIDATE_TEXTURE));
}

GPU_DECL GPU_VOID GPU_FUNC(SetPixelShaderEmbeddedParamF(GPU_ARGS(const uint32_t dstOffset, const float* pData, const uint32_t sizeInWords, const uint32_t location)))
{
	GCM_FUNC(SetTransferLocation)(GCM_ARGS(location));
	
	void* pointer = NULL;
	GCM_FUNC(SetInlineTransferPointer)(GCM_ARGS(dstOffset, sizeInWords, &pointer));
	
	float* fp = (float*)pointer;
	float* src = (float*)pData;
	for(uint32_t j=0; j<sizeInWords; j++)
	{
		*fp = cellGcmSwap16Float32(*src);
		fp++; src++;
	}
}

GPU_DECL GPU_VOID GPU_FUNC(SetPixelShaderParamF(GPU_ARGS(const C_PixelShaderParam& shaderParam, const F32* pData, const U32 registerCount)))
{
	BIZ_ASSERT(pData);
	
	if(!shaderParam)
		return;

	BIZ_ASSERT(shaderParam->m_pPixelShader);
	
	for(S32 i=0; i<shaderParam->m_Offsets.Size(); i++)
	{
		SetPixelShaderEmbeddedParamF(
			shaderParam->m_pPixelShader->m_ProgramConfig.offset+shaderParam->m_Offsets[i],
			pData,
			4,
			FlagsToLocation(shaderParam->m_pPixelShader->m_Flags));
	}
}

GPU_DECL GPU_VOID GPU_FUNC(SetPixelShaderParamArrayF(GPU_ARGS(const C_PixelShaderParamArray& shaderParamArray, const F32* pData, const U32 registerCount)))
{
	BIZ_ASSERT(pData);

	for(S32 i=0; i<shaderParamArray.Size(); i++)
	{
		SetPixelShaderParamF(shaderParamArray[i], &pData[i*4], registerCount);
	}
}

GPU_DECL GPU_VOID GPU_FUNC(SetPixelShaderParams(GPU_ARGS(const C_PixelShader* pPixelShader)))
{
	BIZ_ASSERT(pPixelShader);
	BIZ_ASSERT(pPixelShader->IsCreated());

	GCM_FUNC(SetUpdateFragmentProgramParameter)(GCM_ARGS(pPixelShader->GetOffset()));
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexBuffer(GPU_ARGS(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride)))
{
	#if GPU_CHECK || GPU_TRACK
	BIZ_ASSERT(m_pState->m_pVertexDeclaration);
	SetVertexBuffer(
		stream,
		pVertexBuffer,
		offset,
		stride,
		m_pState->m_pVertexDeclaration);
	#else
	BIZ_ASSERT(0 && "Funcionality not available on a context with no checking or tracking");
	#endif
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexBuffer(GPU_ARGS(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride, const C_VertexDeclaration* pVertexDeclaration, const U32 iUsageFlags)))
{
	BIZ_ASSERT(stream < E_Stream_Count);

	#if GPU_CHECK
	if(pVertexBuffer == m_pState->m_pVertexBuffer[stream] &&
		offset == m_pState->m_VertexOffset[stream] &&
		stride == m_pState->m_VertexStride[stream])
		GPU_RETURN_OK;
	#endif

	#if GPU_TRACK
	m_pState->m_pVertexBuffer[stream] = pVertexBuffer;
	m_pState->m_VertexOffset[stream] = offset;
	m_pState->m_VertexStride[stream] = stride;
	#endif

	GPU_ASSERT(pVertexDeclaration);
	GPU_ASSERT(pVertexDeclaration->m_ElementCount <= C_VertexDeclaration::m_MaxElementCount);

	for(U32 i=0; i<pVertexDeclaration->m_ElementCount; i++)
	{
		const C_VertexElement& element = pVertexDeclaration->m_ElementArray[i];

		if(stream == element.m_Stream)
		{
			GPU_ASSERT(element.m_Usage+element.m_UsageIndex <= 15);
			GPU_ASSERT_LOCATION(FlagsToLocation(pVertexBuffer->m_Flags));
			//GPU_ASSERT(pVertexBuffer->m_Offset+offset+element.m_Offset % 2 == 0);

			if (iUsageFlags&(1<<(element.m_Usage+element.m_UsageIndex)))
			{
				GCM_FUNC(SetVertexDataArrayOffset)(GCM_ARGS(
					element.m_Usage+element.m_UsageIndex,
					FlagsToLocation(pVertexBuffer->m_Flags),
					pVertexBuffer->m_Offset+offset+element.m_Offset));
			}
		}
	}
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexDeclaration(GPU_ARGS(const C_VertexDeclaration* pVertexDeclaration, U32* pVertexElementsBitfield, U32 iUsageFlags)))
{
	#if GPU_CHECK
	if(pVertexDeclaration == m_pState->m_pVertexDeclaration)
		GPU_RETURN_OK;
	#endif

	#if GPU_TRACK
	m_pState->m_pVertexDeclaration = pVertexDeclaration;
	#endif

	if(!pVertexDeclaration)
		return;

	GPU_ASSERT(pVertexDeclaration->m_ElementCount <= C_VertexDeclaration::m_MaxElementCount);

	U32 vertexElementsBitfield = 0;

	for(U32 i=0; i<pVertexDeclaration->m_ElementCount; i++)
	{
		const C_VertexElement& element = pVertexDeclaration->m_ElementArray[i];
		U32 iBit = (1<<(element.m_Usage+element.m_UsageIndex));
		if (!(iUsageFlags & iBit))
			continue;

		GPU_ASSERT(element.m_Usage+element.m_UsageIndex <= 15);
		GPU_ASSERT(element.m_gcmSize <= 4);
		GPU_ASSERT_VERTEX_ELEMENT_GCM_TYPE(element.m_gcmType);

		GPU_ASSERT(element.m_gcmType == CELL_GCM_VERTEX_UB256 ? element.m_gcmSize == 4 : 1);
		GPU_ASSERT(element.m_gcmType == CELL_GCM_VERTEX_CMP ? element.m_gcmSize == 1 : 1);
		
		GCM_FUNC(SetVertexDataArrayFormat)(GCM_ARGS(
			element.m_Usage+element.m_UsageIndex,
			0,
			element.m_gcmStride,
			element.m_gcmSize,
			element.m_gcmType));

		vertexElementsBitfield |= iBit;
	}

	if(pVertexElementsBitfield && (*pVertexElementsBitfield != vertexElementsBitfield))
	{
		#define IsBitSet(_bit, _bitfield) ((_bitfield)&(1<<i))

		for(U32 i=0; i<16; i++)
		{
			// Vertex element not used by vertex declaration and either tracking says vertex element is enabled or has been dirtied
			if(!IsBitSet(i, vertexElementsBitfield) && (IsBitSet(i, *pVertexElementsBitfield) || IsBitSet(31, *pVertexElementsBitfield)))
			{
				GCM_FUNC(SetVertexDataArrayFormat)(GCM_ARGS(i, 0, 0, 0, CELL_GCM_VERTEX_F)); // Disable vertex element
			}
		}

		*pVertexElementsBitfield = vertexElementsBitfield;
	}
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexShader(GPU_ARGS(const C_VertexShader* pVertexShader)))
{
	#if GPU_CHECK
	if(pVertexShader == m_pState->m_pVertexShader)
		GPU_RETURN_OK;
	#endif

	#if GPU_TRACK
	m_pState->m_pVertexShader = pVertexShader;
	#endif

	if(!pVertexShader)
		return;

	BIZ_ASSERT(pVertexShader->IsCreated());

#if PLATFORM_SPU
	SetCallCommand(pVertexShader->m_CommandChain.m_Offset);
#else
	const U32 count = cellCgbGetVertexConstantCount(&pVertexShader->m_Program);
	const F32* value = 0;
	U16 reg = 0;
	U32 i = 0;
	while(i < count)
	{
		cellCgbGetVertexConstantValues(&pVertexShader->m_Program, i, &reg, &value);
		GCM_FUNC(SetVertexProgramParameterBlock)(GCM_ARGS(reg, 1, value));
		i++;
	}

	const void* ucode = cellCgbGetUCode(&pVertexShader->m_Program);
	GCM_FUNC(SetVertexProgramLoad)(GCM_ARGS(&pVertexShader->m_ProgramConfig, ucode));
#endif
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexShaderConstantF(GPU_ARGS(const U32 startRegister, const F32* pData, const U32 registerCount)))
{
	GCM_FUNC(SetVertexProgramConstants)(GCM_ARGS(
		startRegister,
		registerCount*4,
		pData));
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexShaderParamF(GPU_ARGS(const C_VertexShaderParam& shaderParam, const F32* pData)))
{
	GCM_FUNC(SetVertexProgramConstants)(GCM_ARGS(
		shaderParam.m_RegisterIndex,
		shaderParam.m_RegisterCount*4,
		pData));
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetReferenceCommand)(GPU_ARGS(const uint32_t ref))
{
	GCM_FUNC(SetReferenceCommand)(GCM_ARGS(ref));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetJumpCommand)(GPU_ARGS(const uint32_t offset))
{
	GCM_FUNC(SetJumpCommand)(GCM_ARGS(offset));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetCallCommand)(GPU_ARGS(const uint32_t offset))
{
	GCM_FUNC(SetCallCommand)(GCM_ARGS(offset));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetReturnCommand)(GPU_NO_ARGS())
{
	GCM_FUNC(SetReturnCommand)(GCM_NO_ARGS());

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetNopCommand)(GPU_ARGS(const uint32_t count))
{
	GCM_FUNC(SetNopCommand)(GCM_ARGS(count));

	GPU_RETURN_VOID;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetUpdateFragmentProgramParameter)(GPU_ARGS(const uint32_t offset))
{
	GCM_FUNC(SetUpdateFragmentProgramParameter)(GCM_ARGS(offset));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramParameterBlock)(GPU_ARGS(const uint32_t baseConst, uint32_t constCount, const float * __restrict value))
{
	GCM_FUNC(SetVertexProgramParameterBlock)(GCM_ARGS(baseConst, constCount, value));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramLoad)(GPU_ARGS(const CellCgbVertexProgramConfiguration *conf, const void *ucode))
{
	GCM_FUNC(SetVertexProgramLoad)(GCM_ARGS(conf, ucode));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramLoadSlot)(GPU_ARGS(const uint32_t loadSlot, const uint32_t instCount, const void *ucode))
{
	GCM_FUNC(SetVertexProgramLoadSlot)(GCM_ARGS(loadSlot, instCount, ucode));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetAntiAliasingControl)(GPU_ARGS(const uint32_t enable, const uint32_t alphaToCoverage, const uint32_t alphaToOne, const uint32_t sampleMask))
{
	GCM_FUNC(SetAntiAliasingControl)(GCM_ARGS(enable, alphaToCoverage, alphaToOne, sampleMask));

	GPU_RETURN_VOID;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

GPU_DECL GPU_VOID GPU_FUNC(SetInvalidateVertexCache)(GPU_NO_ARGS())
{
	GCM_FUNC(SetInvalidateVertexCache)(GCM_NO_ARGS());

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetInvalidateTextureCache)(GPU_ARGS(const uint32_t value))
{
	GCM_FUNC(SetInvalidateTextureCache)(GCM_ARGS(value));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTexture)(GPU_ARGS(const uint8_t index, const CellGcmTexture *texture))
{
	GPU_ASSERT(index < 16);
	GPU_ASSERT(DebugCheckTexture(texture));

	GCM_FUNC(SetTexture)(GCM_ARGS(index, texture));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTextureControl)(GPU_ARGS(const uint8_t index, const uint32_t enable, const uint16_t minlod, const uint16_t maxlod, const uint8_t maxaniso))
{
	GCM_FUNC(SetTextureControl)(GCM_ARGS(index, enable, minlod, maxlod, maxaniso));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexTexture)(GPU_ARGS(const uint8_t index, const CellGcmTexture *texture))
{
	GCM_FUNC(SetVertexTexture)(GCM_ARGS(index, texture));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexTextureControl)(GPU_ARGS(const uint8_t index, const uint32_t enable, const uint16_t minLod, const uint16_t maxLod))
{
	GCM_FUNC(SetVertexTextureControl)(GCM_ARGS(index, enable, minLod, maxLod));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetAlphaFunc)(GPU_ARGS(const uint32_t af, const uint32_t ref))
{
	GCM_FUNC(SetAlphaFunc)(GCM_ARGS(af, ref));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetColourWriteEnable)(GPU_ARGS(const uint32_t mask))
{
	GCM_FUNC(SetColorMask)(GCM_ARGS(mask));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetWriteBackEndLabel)(GPU_ARGS(const uint8_t index, const uint32_t value))
{
	GCM_FUNC(SetWriteBackEndLabel)(GCM_ARGS(index, value));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTimeStamp)(GPU_ARGS(const uint32_t index))
{
	GPU_ASSERT(index <= 2047);

	GCM_FUNC(SetTimeStamp)(GCM_ARGS(index));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTextureRemap)(GPU_ARGS(const uint8_t index, const uint32_t remap))
{
	GCM_FUNC(SetTextureRemap)(GCM_ARGS(index, remap));

	GPU_RETURN_VOID;
}


#if 0

GPU_DECL GPU_VOID GPU_FUNC(SetSurface)(GPU_ARGS(const CellGcmSurface *surface))
{
	GCM_FUNC(SetSurface)(GCM_ARGS(surface));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetSurfaceWindow)(GPU_ARGS(const CellGcmSurface *surface, const uint32_t origin, const uint32_t pixelCenter))
{
	GCM_FUNC(SetSurfaceWindow)(GCM_ARGS(surface, origin, pixelCenter));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetWaitLabel)(GPU_ARGS(const uint8_t index, const uint32_t value))
{
	GCM_FUNC(SetWaitLabel)(GCM_ARGS(index, value));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetWriteCommandLabel)(GPU_ARGS(const uint8_t index, const uint32_t value))
{
	GCM_FUNC(SetWriteCommandLabel)(GCM_ARGS(index, value));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetWriteTextureLabel)(GPU_ARGS(const uint8_t index, const uint32_t value))
{
	GCM_FUNC(SetWriteTextureLabel)(GCM_ARGS(index, value));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetInvalidateZcull)(GPU_NO_ARGS())
{
	GCM_FUNC(SetInvalidateZcull)(GCM_NO_ARGS());

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBlendColor)(GPU_ARGS(const uint32_t color, const uint32_t color2))
{
	GCM_FUNC(SetBlendColor)(GCM_ARGS(color, color2));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBlendEquation)(GPU_ARGS(const uint16_t color, const uint16_t alpha))
{
	GCM_FUNC(SetBlendEquation)(GCM_ARGS(color, alpha));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBlendFunc)(GPU_ARGS(const uint16_t sfcolor, const uint16_t dfcolor, const uint16_t sfalpha, const uint16_t dfalpha))
{
	GCM_FUNC(SetBlendFunc)(GCM_ARGS(sfcolor, dfcolor, sfalpha, dfalpha));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetClearSurface)(GPU_ARGS(const uint32_t mask))
{
	GCM_FUNC(SetClearSurface)(GCM_ARGS(mask));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetClearColor)(GPU_ARGS(const uint32_t color))
{
	GCM_FUNC(SetClearColor)(GCM_ARGS(color));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetClearDepthStencil)(GPU_ARGS(const uint32_t value))
{
	GCM_FUNC(SetClearDepthStencil)(GCM_ARGS(value));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetColorMaskMrt)(GPU_ARGS(const uint32_t mask))
{
	GCM_FUNC(SetColorMaskMrt)(GCM_ARGS(mask));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetCullFace)(GPU_ARGS(const uint32_t cfm))
{
	GCM_FUNC(SetCullFace)(GCM_ARGS(cfm));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDepthBounds)(GPU_ARGS(const float zmin, const float zmax))
{
	GCM_FUNC(SetDepthBounds)(GCM_ARGS(zmin, zmax));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFrontFace)(GPU_ARGS(const uint32_t dir))
{
	GCM_FUNC(SetFrontFace)(GCM_ARGS(dir));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetLineWidth)(GPU_ARGS(const uint32_t width))
{
	GCM_FUNC(SetLineWidth)(GCM_ARGS(width));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetLineSmoothEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetLineSmoothEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetLineStippleEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetLineStippleEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetLineStipplePattern)(GPU_ARGS(const uint16_t* pattern, const uint8_t factor))
{
	GCM_FUNC(SetLineStipplePattern)(GCM_ARGS(pattern, factor));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetLogicOp)(GPU_ARGS(const uint32_t op))
{
	GCM_FUNC(SetLogicOp)(GCM_ARGS(op));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetPointSize)(GPU_ARGS(const float size))
{
	GCM_FUNC(SetPointSize)(GCM_ARGS(size));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetPolygonOffset)(GPU_ARGS(const float factor, const float units))
{
	GCM_FUNC(SetPolygonOffset)(GCM_ARGS(factor, units));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetPolySmoothEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetPolySmoothEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetPolygonStippleEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetPolygonStippleEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetPolygonStipplePattern)(GPU_ARGS(const uint32_t* pattern))
{
	GCM_FUNC(SetPolygonStipplePattern)(GCM_ARGS(pattern));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFrontPolygonMode)(GPU_ARGS(const uint32_t mode))
{
	GCM_FUNC(SetFrontPolygonMode)(GCM_ARGS(mode));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBackPolygonMode)(GPU_ARGS(const uint32_t mode))
{
	GCM_FUNC(SetBackPolygonMode)(GCM_ARGS(mode));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetScissor)(GPU_ARGS(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h))
{
	GCM_FUNC(SetScissor)(GCM_ARGS(x, y, w, h));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetShadeMode)(GPU_ARGS(const uint32_t sm))
{
	GCM_FUNC(SetShadeMode)(GCM_ARGS(sm));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTwoSideLightEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetTwoSideLightEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetStencilFunc)(GPU_ARGS(const uint32_t func, const int32_t ref, const uint32_t mask))
{
	GCM_FUNC(SetStencilFunc)(GCM_ARGS(func, ref, mask));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBackStencilFunc)(GPU_ARGS(const uint32_t func, const int32_t ref, const uint32_t mask))
{
	GCM_FUNC(SetBackStencilFunc)(GCM_ARGS(func, ref, mask));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetStencilMask)(GPU_ARGS(const uint32_t sm))
{
	GCM_FUNC(SetStencilMask)(GCM_ARGS(sm));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBackStencilMask)(GPU_ARGS(const uint32_t sm))
{
	GCM_FUNC(SetBackStencilMask)(GCM_ARGS(sm));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetStencilOp)(GPU_ARGS(const uint32_t fail, const uint32_t depthFail, const uint32_t depthPass))
{
	GCM_FUNC(SetStencilOp)(GCM_ARGS(fail, depthFail, depthPass));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBackStencilOp)(GPU_ARGS(const uint32_t fail, const uint32_t depthFail, const uint32_t depthPass))
{
	GCM_FUNC(SetBackStencilOp)(GCM_ARGS(fail, depthFail, depthPass));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetViewport)(GPU_ARGS(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const float min, const float max, const float scale[4], const float offset[4]))
{
	GCM_FUNC(SetViewport)(GCM_ARGS(x, y, w, h, min, max, scale, offset));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetZMinMaxControl)(GPU_ARGS(const uint32_t cullNearFarEnable, const uint32_t zclampEnable, const uint32_t cullIgnoreW))
{
	GCM_FUNC(SetZMinMaxControl)(GCM_ARGS(cullNearFarEnable, zclampEnable, cullIgnoreW));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBlendEnableMrt)(GPU_ARGS(const uint32_t mrt1, const uint32_t mrt2, const uint32_t mrt3))
{
	GCM_FUNC(SetBlendEnableMrt)(GCM_ARGS(mrt1, mrt2, mrt3));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetLogicOpEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetLogicOpEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetCullFaceEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetCullFaceEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDepthBoundsTestEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetDepthBoundsTestEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDitherEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetDitherEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTwoSidedStencilTestEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetTwoSidedStencilTestEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetPolygonOffsetFillEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetPolygonOffsetFillEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetRestartIndexEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetRestartIndexEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTextureBorderColor)(GPU_ARGS(const uint8_t index, const uint32_t color))
{
	GCM_FUNC(SetTextureBorderColor)(GCM_ARGS(index, color));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTextureFilter)(GPU_ARGS(const uint8_t index, const uint16_t bias, const uint8_t min, const uint8_t mag, const uint8_t conv))
{
	GCM_FUNC(SetTextureFilter)(GCM_ARGS(index, bias, min, mag, conv));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTextureAddress)(GPU_ARGS(const uint8_t index, const uint8_t wraps, const uint8_t wrapt, const uint8_t wrapr, const uint8_t unsignedRemap, const uint8_t zfunc, const uint8_t gamma))
{
	GCM_FUNC(SetTextureAddress)(GCM_ARGS(index, wraps, wrapt, wrapr, unsignedRemap, zfunc, gamma));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTextureOptimization)(GPU_ARGS(const uint8_t index, const uint8_t slope, const uint8_t iso, const uint8_t aniso))
{
	GCM_FUNC(SetTextureOptimization)(GCM_ARGS(index, slope, iso, aniso));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetCylindricalWrap)(GPU_ARGS(const uint32_t enable, const uint32_t reserved))
{
	GCM_FUNC(SetCylindricalWrap)(GCM_ARGS(enable, reserved));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawArrays)(GPU_ARGS(const uint8_t mode, uint32_t first, uint32_t count))
{
	GCM_FUNC(SetDrawArrays)(GCM_ARGS(mode, first, count));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawIndexArray)(GPU_ARGS(const uint8_t mode, uint32_t count, const uint8_t type, const uint8_t location, const uint32_t indicies))
{
	GCM_FUNC(SetDrawIndexArray)(GCM_ARGS(mode, count, type, location, indicies));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetRestartIndex)(GPU_ARGS(const uint32_t index))
{
	GCM_FUNC(SetRestartIndex)(GCM_ARGS(index));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexData4f)(GPU_ARGS(const uint8_t index, const float v[4]))
{
	GCM_FUNC(SetVertexData4f)(GCM_ARGS(index, v));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexDataArray)(GPU_ARGS(const uint8_t index, const uint16_t frequency, const uint8_t stride, const uint8_t size, const uint8_t type, const uint8_t location, const uint32_t offset))
{
	GCM_FUNC(SetVertexDataArray)(GCM_ARGS(index, frequency, stride, size, type, location, offset));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFrequencyDividerOperation)(GPU_ARGS(const uint16_t operation))
{
	GCM_FUNC(SetFrequencyDividerOperation)(GCM_ARGS(operation));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTransformBranchBits)(GPU_ARGS(const uint32_t branchBits))
{
	GCM_FUNC(SetTransformBranchBits)(GCM_ARGS(branchBits));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexAttribInputMask)(GPU_ARGS(const uint16_t mask))
{
	GCM_FUNC(SetVertexAttribInputMask)(GCM_ARGS(mask));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramGammaEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetFragmentProgramGammaEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetUserClipPlaneControl)(GPU_ARGS(const uint32_t plane0, const uint32_t plane1, const uint32_t plane2, const uint32_t plane3, const uint32_t plane4, const uint32_t plane5))
{
	GCM_FUNC(SetUserClipPlaneControl)(GCM_ARGS(plane0, plane1, plane2, plane3, plane4, plane5));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramConstants)(GPU_ARGS(const uint32_t start, const uint32_t count, const float *data))
{
	GCM_FUNC(SetVertexProgramConstants)(GCM_ARGS(start, count, data));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetInlineTransfer)(GPU_ARGS(const uint32_t dstOffset, const void *srcAdr, const uint32_t sizeInWords))
{
	GCM_FUNC(SetInlineTransfer)(GCM_ARGS(dstOffset, srcAdr, sizeInWords));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(InlineTransfer)(GPU_ARGS(const uint32_t dstOffset, const void *srcAdr, const uint32_t sizeInWords, const uint8_t location))
{
	GCM_FUNC(InlineTransfer)(GCM_ARGS(dstOffset, srcAdr, sizeInWords, location));

	GPU_RETURN_VOID;
}

GPU_DECL uint32_t GPU_FUNC(SetTransferImage)(GPU_ARGS(const uint8_t mode, const uint32_t dstOffset, const uint32_t dstPitch, const uint32_t dstX, const uint32_t dstY, const uint32_t srcOffset, const uint32_t srcPitch, const uint32_t srcX, const uint32_t srcY, const uint32_t width, const uint32_t height, const uint32_t bytesPerPixel))
{
	const uint32_t result = GCM_FUNC(SetTransferImage)(GCM_ARGS(mode, dstOffset, dstPitch, dstX, dstY, srcOffset, srcPitch, srcX, srcY, width, height, bytesPerPixel));

	return result;
}

GPU_DECL GPU_VOID GPU_FUNC(TransferData)(GPU_ARGS(uint32_t dstOffset, int32_t dstPitch, uint32_t srcOffset, int32_t srcPitch, int32_t bytesPerRow, int32_t rowCount))
{
	GCM_FUNC(TransferData)(GCM_ARGS(dstOffset, dstPitch, srcOffset, srcPitch, bytesPerRow, rowCount));

	GPU_RETURN_VOID;
}

GPU_DECL uint32_t GPU_FUNC(SetTransferData)(GPU_ARGS(const uint8_t mode, const uint32_t dstOffset, const uint32_t dstPitch, const uint32_t srcOffset, const uint32_t srcPitch, const uint32_t bytesPerRow, const uint32_t rowCount))
{
	const uint32_t result = GCM_FUNC(SetTransferData)(GCM_ARGS(mode, dstOffset, dstPitch, srcOffset, srcPitch, bytesPerRow, rowCount));

	return result;
}

GPU_DECL GPU_VOID GPU_FUNC(SetRenderEnable)(GPU_ARGS(const uint8_t mode, const uint32_t index))
{
	GCM_FUNC(SetRenderEnable)(GCM_ARGS(mode, index));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetZpassPixelCountEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetZpassPixelCountEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetClearReport)(GPU_ARGS(const uint32_t type))
{
	GCM_FUNC(SetClearReport)(GCM_ARGS(type));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetReport)(GPU_ARGS(const uint32_t type, const uint32_t index))
{
	GCM_FUNC(SetReport)(GCM_ARGS(type, index));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetZcullStatsEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetZcullStatsEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetZcullControl)(GPU_ARGS(const uint8_t zCullDir, const uint8_t zCullFormat))
{
	GCM_FUNC(SetZcullControl)(GCM_ARGS(zCullDir, zCullFormat));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetZcullLimit)(GPU_ARGS(const uint16_t moveForwardLimit, const uint16_t pushBackLimit))
{
	GCM_FUNC(SetZcullLimit)(GCM_ARGS(moveForwardLimit, pushBackLimit));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetScullControl)(GPU_ARGS(const uint8_t sFunc, const uint8_t sRef, const uint8_t sMask))
{
	GCM_FUNC(SetScullControl)(GCM_ARGS(sFunc, sRef, sMask));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetConvertSwizzleFormat)(GPU_ARGS(uint32_t dstOffset, uint32_t dstWidth, uint32_t dstHeight, uint32_t dstX, uint32_t dstY, uint32_t srcOffset, uint32_t srcPitch, uint32_t srcX, uint32_t srcY, uint32_t width, uint32_t height, uint32_t bytesPerPixel, uint8_t mode))
{
	GCM_FUNC(SetConvertSwizzleFormat)(GCM_ARGS(dstOffset, dstWidth, dstHeight, dstX, dstY, srcOffset, srcPitch, srcX, srcY, width, height, bytesPerPixel, mode));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexTextureAddress)(GPU_ARGS(const uint8_t index, const uint8_t wraps, const uint8_t wrapt))
{
	GCM_FUNC(SetVertexTextureAddress)(GCM_ARGS(index, wraps, wrapt));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexTextureFilter)(GPU_ARGS(const uint8_t index, const uint16_t bias))
{
	GCM_FUNC(SetVertexTextureFilter)(GCM_ARGS(index, bias));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexTextureBorderColor)(GPU_ARGS(const uint8_t index, const uint32_t color))
{
	GCM_FUNC(SetVertexTextureBorderColor)(GCM_ARGS(index, color));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetPerfMonTrigger)(GPU_NO_ARGS())
{
	GCM_FUNC(SetPerfMonTrigger)(GCM_NO_ARGS());

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineArray)(GPU_ARGS(const uint8_t mode, const uint32_t count, const void *data))
{
	GCM_FUNC(SetDrawInlineArray)(GCM_ARGS(mode, count, data));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray32)(GPU_ARGS(const uint8_t mode, const uint32_t start, const uint32_t count, const uint32_t *data))
{
	GCM_FUNC(SetDrawInlineIndexArray32)(GCM_ARGS(mode, start, count, data));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray16)(GPU_ARGS(const uint8_t mode, const uint32_t start, const uint32_t count, const uint16_t *data))
{
	GCM_FUNC(SetDrawInlineIndexArray16)(GCM_ARGS(mode, start, count, data));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFogMode)(GPU_ARGS(const uint32_t mode))
{
	GCM_FUNC(SetFogMode)(GCM_ARGS(mode));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFogParams)(GPU_ARGS(const float p0, const float p1))
{
	GCM_FUNC(SetFogParams)(GCM_ARGS(p0, p1));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTransferLocation)(GPU_ARGS(const uint32_t location))
{
	GCM_FUNC(SetTransferLocation)(GCM_ARGS(location));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDepthFormat)(GPU_ARGS(const uint32_t format))
{
	GCM_FUNC(SetDepthFormat)(GCM_ARGS(format));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetBlendOptimization)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetBlendOptimization)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetPolygonOffsetLineEnable)(GPU_ARGS(const uint32_t enable))
{
	GCM_FUNC(SetPolygonOffsetLineEnable)(GCM_ARGS(enable));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexAttribOutputMask)(GPU_ARGS(const uint32_t mask))
{
	GCM_FUNC(SetVertexAttribOutputMask)(GCM_ARGS(mask));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramStartSlot)(GPU_ARGS(const uint32_t startSlot))
{
	GCM_FUNC(SetVertexProgramStartSlot)(GCM_ARGS(startSlot));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramRegisterCount)(GPU_ARGS(const uint32_t registerCount))
{
	GCM_FUNC(SetVertexProgramRegisterCount)(GCM_ARGS(registerCount));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramLoad)(GPU_ARGS(const CellCgbFragmentProgramConfiguration *conf))
{
	GCM_FUNC(SetFragmentProgramLoad)(GCM_ARGS(conf));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetInlineTransferPointer)(GPU_ARGS(const uint32_t offset, const uint32_t count, void **pointer))
{
	GCM_FUNC(SetInlineTransferPointer)(GCM_ARGS(offset, count, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTransferDataMode)(GPU_ARGS(const uint8_t mode))
{
	GCM_FUNC(SetTransferDataMode)(GCM_ARGS(mode));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTransferDataFormat)(GPU_ARGS(const int32_t inPitch, const int32_t outPitch, const uint32_t lineLength, const uint32_t lineCount, const uint8_t inFormat, const uint8_t outFormat))
{
	GCM_FUNC(SetTransferDataFormat)(GCM_ARGS(inPitch, outPitch, lineLength, lineCount, inFormat, outFormat));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTransferDataOffset)(GPU_ARGS(const uint32_t dstOffset, const uint32_t srcOffset))
{
	GCM_FUNC(SetTransferDataOffset)(GCM_ARGS(dstOffset, srcOffset));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTransferScaleMode)(GPU_ARGS(const uint8_t mode, const uint8_t surface))
{
	GCM_FUNC(SetTransferScaleMode)(GCM_ARGS(mode, surface));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTransferScaleSurface)(GPU_ARGS(const CellGcmTransferScale *scale, CellGcmTransferSurface *surface))
{
	GCM_FUNC(SetTransferScaleSurface)(GCM_ARGS(scale, surface));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTransferScaleSwizzle)(GPU_ARGS(const CellGcmTransferScale *scale, CellGcmTransferSwizzle *swizzle))
{
	GCM_FUNC(SetTransferScaleSwizzle)(GCM_ARGS(scale, swizzle));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawBegin)(GPU_ARGS(const uint8_t mode))
{
	GCM_FUNC(SetDrawBegin)(GCM_ARGS(mode));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawEnd)(GPU_NO_ARGS())
{
	GCM_FUNC(SetDrawEnd)(GCM_NO_ARGS());

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineArrayPointer)(GPU_ARGS(const uint32_t count, void **pointer))
{
	GCM_FUNC(SetDrawInlineArrayPointer)(GCM_ARGS(count, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramConstantsPointer)(GPU_ARGS(uint32_t first, uint32_t count, void **pointer))
{
	GCM_FUNC(SetVertexProgramConstantsPointer)(GCM_ARGS(first, count, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray32Pointer)(GPU_ARGS(const uint32_t count, void **pointer))
{
	GCM_FUNC(SetDrawInlineIndexArray32Pointer)(GCM_ARGS(count, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray16Pointer)(GPU_ARGS(const uint32_t count, void **pointer))
{
	GCM_FUNC(SetDrawInlineIndexArray16Pointer)(GCM_ARGS(count, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetAnisoSpread)(GPU_ARGS(const uint8_t index, const uint8_t reduceSamplesEnable, const uint8_t hReduceSamplesEnable, const uint8_t vReduceSamplesEnable, const uint8_t spacingSelect, const uint8_t hSpacingSelect, const uint8_t vSpacingSelect))
{
	GCM_FUNC(SetAnisoSpread)(GCM_ARGS(index, reduceSamplesEnable, hReduceSamplesEnable, vReduceSamplesEnable, spacingSelect, hSpacingSelect, vSpacingSelect));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTextureAddressAnisoBias)(GPU_ARGS(const uint8_t index, const uint8_t wraps, const uint8_t wrapt, const uint8_t wrapr, const uint8_t unsignedRemap, const uint8_t zfunc, const uint8_t gamma, const uint8_t anisoBias))
{
	GCM_FUNC(SetTextureAddressAnisoBias)(GCM_ARGS(index, wraps, wrapt, wrapr, unsignedRemap, zfunc, gamma, anisoBias));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramParameterBlockPointer)(GPU_ARGS(uint32_t baseConst, uint32_t constCount, void **pointer))
{
	GCM_FUNC(SetVertexProgramParameterBlockPointer)(GCM_ARGS(baseConst, constCount, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexDataArrayFormat)(GPU_ARGS(const uint8_t index, const uint16_t frequency, const uint8_t stride, const uint8_t size, const uint8_t type))
{
	GCM_FUNC(SetVertexDataArrayFormat)(GCM_ARGS(index, frequency, stride, size, type));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexDataArrayOffset)(GPU_ARGS(const uint8_t index, const uint8_t location, const uint32_t offset))
{
	GCM_FUNC(SetVertexDataArrayOffset)(GCM_ARGS(index, location, offset));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetInlineTransferAlignedPointer)(GPU_ARGS(const uint32_t offset, const uint32_t count, void **pointer))
{
	GCM_FUNC(SetInlineTransferAlignedPointer)(GCM_ARGS(offset, count, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramConstantsAlignedPointer)(GPU_ARGS(uint32_t first, uint32_t count, void **pointer))
{
	GCM_FUNC(SetVertexProgramConstantsAlignedPointer)(GCM_ARGS(first, count, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramParameterBlockAlignedPointer)(GPU_ARGS(const uint32_t baseConst, const uint32_t constCount, void **pointer))
{
	GCM_FUNC(SetVertexProgramParameterBlockAlignedPointer)(GCM_ARGS(baseConst, constCount, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineArrayAlignedPointer)(GPU_ARGS(const uint32_t count, void **pointer))
{
	GCM_FUNC(SetDrawInlineArrayAlignedPointer)(GCM_ARGS(count, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray32AlignedPointer)(GPU_ARGS(const uint32_t count, void **pointer))
{
	GCM_FUNC(SetDrawInlineIndexArray32AlignedPointer)(GCM_ARGS(count, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetDrawInlineIndexArray16AlignedPointer)(GPU_ARGS(const uint32_t count, void **pointer))
{
	GCM_FUNC(SetDrawInlineIndexArray16AlignedPointer)(GCM_ARGS(count, pointer));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramLoadLocation)(GPU_ARGS(const CellCgbFragmentProgramConfiguration *conf, const uint32_t location))
{
	GCM_FUNC(SetFragmentProgramLoadLocation)(GCM_ARGS(conf, location));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetUpdateFragmentProgramParameterLocation)(GPU_ARGS(const uint32_t offset, const uint32_t location))
{
	GCM_FUNC(SetUpdateFragmentProgramParameterLocation)(GCM_ARGS(offset, location));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexDataBase)(GPU_ARGS(const uint32_t baseOffset, const uint32_t baseIndex))
{
	GCM_FUNC(SetVertexDataBase)(GCM_ARGS(baseOffset, baseIndex));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgramParameter)(GPU_ARGS(const CGparameter param, const float *value))
{
	GCM_FUNC(SetVertexProgramParameter)(GCM_ARGS(param, value));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetVertexProgram)(GPU_ARGS(const CGprogram prog, const void *ucode))
{
	GCM_FUNC(SetVertexProgram)(GCM_ARGS(prog, ucode));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramOffset)(GPU_ARGS(const CGprogram prog, const uint32_t offset, const uint32_t location))
{
	GCM_FUNC(SetFragmentProgramOffset)(GCM_ARGS(prog, offset, location));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramControl)(GPU_ARGS(const CGprogram prog, const uint8_t controlTxp, const uint8_t reserved0, const uint8_t reserved1))
{
	GCM_FUNC(SetFragmentProgramControl)(GCM_ARGS(prog, controlTxp, reserved0, reserved1));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgram)(GPU_ARGS(const CGprogram prog, uint32_t offset))
{
	GCM_FUNC(SetFragmentProgram)(GCM_ARGS(prog, offset));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramParameter)(GPU_ARGS(const CGprogram prog, const CGparameter param, const float *value, const uint32_t offset))
{
	GCM_FUNC(SetFragmentProgramParameter)(GCM_ARGS(prog, param, value, offset));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetFragmentProgramParameterPointer)(GPU_ARGS(const CGprogram prog, const CGparameter param, const uint32_t offset, const uint32_t srcOffset))
{
	GCM_FUNC(SetFragmentProgramParameterPointer)(GCM_ARGS(prog, param, offset, srcOffset));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetClearZcullSurface)(GPU_ARGS(const uint32_t depth, const uint32_t stencil))
{
	GCM_FUNC(SetClearZcullSurface)(GCM_ARGS(depth, stencil));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetZcullEnable)(GPU_ARGS(const uint32_t depth, const uint32_t stencil))
{
	GCM_FUNC(SetZcullEnable)(GCM_ARGS(depth, stencil));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetTransferReportData)(GPU_ARGS(const uint32_t offset, const uint32_t index, const uint32_t count))
{
	GCM_FUNC(SetTransferReportData)(GCM_ARGS(offset, index, count));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetWriteBackEndLabelForConditional)(GPU_ARGS(uint8_t index, uint32_t value))
{
	GCM_FUNC(SetWriteBackEndLabelForConditional)(GCM_ARGS(index, value));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetWriteTextureLabelForConditional)(GPU_ARGS(uint8_t index, uint32_t value))
{
	GCM_FUNC(SetWriteTextureLabelForConditional)(GCM_ARGS(index, value));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetUserCommand)(GPU_ARGS(const uint32_t cause))
{
	GCM_FUNC(SetUserCommand)(GCM_ARGS(cause));

	GPU_RETURN_VOID;
}

GPU_DECL GPU_VOID GPU_FUNC(SetWaitFlip)(GPU_NO_ARGS())
{
	GCM_FUNC(SetWaitFlip)(GCM_NO_ARGS());

	GPU_RETURN_VOID;
}

GPU_DECL int32_t GPU_FUNC(SetFlip)(GPU_ARGS(const uint8_t id))
{
	const int32_t result = GCM_FUNC(SetFlip)(GCM_ARGS(id));

	return result;
}

GPU_DECL GPU_VOID GPU_FUNC(ReserveMethodSize)(GPU_ARGS(uint32_t size))
{
	GCM_FUNC(ReserveMethodSize)(GCM_ARGS(size));

	GPU_RETURN_VOID;
}

#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------

#undef GCM_FUNC
#undef GCM_NO_ARGS
#undef GCM_ARGS
#undef GPU_FUNC
#undef GPU_DECL
