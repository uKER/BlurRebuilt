INLINE const F32 C_State::GetClearDepth() const
{
	return m_ClearDepth;
}

INLINE const U32 C_State::GetClearStencil() const
{
	return m_ClearStencil;
}

INLINE const bool C_State::GetFlipZ() const
{
	return m_FlipZ;
}

INLINE void C_State::SetAlphaEnable(const bool alphaEnable)
{
	if (alphaEnable != m_AlphaEnable)
	{
		cellGcmSetAlphaTestEnable(alphaEnable);

		m_AlphaEnable = alphaEnable;
	}
}

INLINE void C_State::SetAlphaFunc(const E_CmpFunc alphaFunc)
{
	// TODO: [LloydW] Alpha ref from 0 to 65535 when using the F_W16Z16Y16X16 format.

	if (alphaFunc != m_AlphaFunc)
	{
		cellGcmSetAlphaFunc(GCMCmpFunc(alphaFunc), m_AlphaRef);
		
		m_AlphaFunc = alphaFunc;
	}
}

INLINE void C_State::SetAlphaRef(const U32 alphaRef)
{
	// TODO: [LloydW] Alpha ref from 0 to 65535 when using the F_W16Z16Y16X16 format.

	if (alphaRef != m_AlphaRef)
	{
		cellGcmSetAlphaFunc(GCMCmpFunc(m_AlphaFunc), alphaRef);
		
		m_AlphaRef = alphaRef;
	}
}

INLINE void C_State::SetBlendDst(const E_Blend blendDst)
{
	if (blendDst != m_BlendDst)
	{
		const U32 gcmBlendSrc = GCMBlend(m_BlendSrc);
		const U32 gcmBlendDst = GCMBlend(blendDst);
		cellGcmSetBlendFunc(gcmBlendSrc, gcmBlendDst, gcmBlendSrc, gcmBlendDst);
		
		m_BlendDst = blendDst;
	}
}

INLINE void C_State::SetBlendEnable(const bool blendEnable)
{
	if (blendEnable != m_BlendEnable)
	{
		cellGcmSetBlendEnable(blendEnable);
		
		m_BlendEnable = blendEnable;
	}
}

INLINE void C_State::SetBlendMode(const E_BlendMode blendMode)
{
	if (m_BlendMode != blendMode)
    {
        switch (blendMode)
        {
		case E_BlendMode_Solid:
    		SetAlphaEnable(false);
			SetBlendEnable(false);
	        break;

		case E_BlendMode_SolidAlphaTest:
        	SetAlphaEnable(true);
			SetAlphaFunc(E_CmpFunc_Greater);
			SetAlphaRef(0x80);
			SetBlendEnable(false);
            break;

		case E_BlendMode_Alpha:
			SetAlphaEnable(false);
			SetAlphaFunc(E_CmpFunc_Greater);
			SetAlphaRef(0x00);
            SetBlendEnable(true);
			SetBlendSrc(E_Blend_SrcAlpha);
			SetBlendDst(E_Blend_InvSrcAlpha);
            SetBlendOp(E_BlendOp_Add);
            break;

		case E_BlendMode_Additive:
        	SetAlphaEnable(false);
		    SetBlendEnable(true);
			SetBlendSrc(E_Blend_One);
			SetBlendDst(E_Blend_One);
            SetBlendOp(E_BlendOp_Add);
            break;

		case E_BlendMode_AlphaAdditive:
            SetAlphaEnable(true);
			SetAlphaFunc(E_CmpFunc_Greater);
			SetAlphaRef(0x00);				
			SetBlendEnable(true);
            SetBlendSrc(E_Blend_SrcAlpha);
			SetBlendDst(E_Blend_One);
            SetBlendOp(E_BlendOp_Add);
			break;

		case E_BlendMode_Subtractive:
            SetAlphaEnable(false);
			SetBlendEnable(true);
            SetBlendSrc(E_Blend_One);
			SetBlendDst(E_Blend_One);
            SetBlendOp(E_BlendOp_RevSubtract);
			break;

		case E_BlendMode_AdditiveSmooth:
        	SetAlphaEnable(false);
		    SetBlendEnable(true);
			SetBlendSrc(E_Blend_One);
			SetBlendDst(E_Blend_InvSrcColour);
            SetBlendOp(E_BlendOp_Add);
            break;

		default:
			SetAlphaEnable(false);
			SetBlendEnable(false);
            break;
        }

        m_BlendMode = blendMode;
    }
}

INLINE void C_State::SetBlendOp(const E_BlendOp blendOp)
{
	if (blendOp != m_BlendOp)
	{
		const U32 gcmBlendOp = GCMBlendOp(blendOp);
		cellGcmSetBlendEquation(gcmBlendOp, gcmBlendOp);
		
		m_BlendOp = blendOp;
	}
}

INLINE void C_State::SetBlendSrc(const E_Blend blendSrc)
{
	if (blendSrc != m_BlendSrc)
	{
		const U32 gcmBlendSrc = GCMBlend(blendSrc);
		const U32 gcmBlendDst = GCMBlend(m_BlendDst);
		cellGcmSetBlendFunc(gcmBlendSrc, gcmBlendDst, gcmBlendSrc, gcmBlendDst);
		
		m_BlendSrc = blendSrc;
	}
}

INLINE void C_State::SetCullEnable(const bool cullEnable)
{
	if (m_CullEnable != cullEnable)
	{
		cellGcmSetCullFaceEnable(cullEnable);

		m_CullEnable = cullEnable;
	}
}

INLINE void C_State::SetCullMode(const E_CullMode cullMode)
{
	if (cullMode != m_CullMode)
	{
		switch (cullMode)
		{
			case E_CullMode_None:
				SetCullEnable(false);
				break;
		
			case E_CullMode_Cw:
				SetCullEnable(true);
				cellGcmSetFrontFace(CELL_GCM_CW);
				break;

			case E_CullMode_Ccw:
				SetCullEnable(true);
				cellGcmSetFrontFace(CELL_GCM_CCW);
				break;

			default:
				BIZ_ASSERT_FAIL("Unsupported cull mode");
				break;
		}

		m_CullMode = cullMode;
	}
}

INLINE void C_State::SetFillMode(const E_FillMode fillMode)
{
	if (fillMode != m_FillMode)
	{
		cellGcmSetFrontPolygonMode(GCMFillMode(fillMode));

		m_FillMode = fillMode;
	}
}

INLINE void C_State::SetFlipZ(const bool flipZ)
{
	if (flipZ != m_FlipZ)
	{
		m_ClearDepth = flipZ ? 0.f : 1.f;

		m_FlipZ = flipZ;
	}
}

INLINE void C_State::SetZEnable(const bool zEnable)
{
	if (zEnable != m_ZEnable)
	{
		cellGcmSetDepthTestEnable(zEnable);

		m_ZEnable = zEnable;
	}
}

INLINE void C_State::SetZFunc(const E_CmpFunc zFunc)
{
	if (zFunc != m_ZFunc)
	{
		cellGcmSetDepthFunc(GCMCmpFunc(zFunc));

		m_ZFunc = zFunc;
	}
}
	
INLINE void C_State::SetZWriteEnable(const bool zWriteEnable)
{
	if (zWriteEnable != m_ZWriteEnable)
	{
		cellGcmSetDepthMask(zWriteEnable);

		m_ZWriteEnable = zWriteEnable;
	}
}
