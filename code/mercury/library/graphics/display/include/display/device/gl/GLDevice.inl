INLINE CGprofile C_GLDevice::GetCGProfileVS()
{
	return m_cgLatestProfileVS;
}

INLINE CGprofile C_GLDevice::GetCGProfilePS()
{
	return m_cgLatestProfilePS;
}
	
INLINE CGcontext C_GLDevice::GetCGContext()
{
	return m_cgContext;
}

INLINE const C_BackBuffer& C_GLDevice::GetBackBuffer() const
{
	return m_BackBuffer;
}

INLINE const C_RenderTarget& C_GLDevice::GetRenderTarget() const
{
	return m_RenderTarget;
}

INLINE void C_GLDevice::DrawPrimitive(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount)
{
	SetVertexDeclaration();
	glDrawArrays(GLPrimitiveType(primitiveType), startVertex, vertexCount);
}

INLINE void C_GLDevice::DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 startIndex, const U32 indexCount)
{
	SetVertexDeclaration();
	BIZ_ASSERT(m_pIndexBuffer);
	glDrawElements(GLPrimitiveType(primitiveType), indexCount, m_pIndexBuffer->m_Format == E_Format_Index32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, BUFFER_OFFSET(startIndex));
}

INLINE void C_GLDevice::DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount)
{
	SetVertexDeclaration();
	BIZ_ASSERT(m_pIndexBuffer);
	glDrawElements(GLPrimitiveType(primitiveType), indexCount, m_pIndexBuffer->m_Format == E_Format_Index32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, BUFFER_OFFSET(startIndex));
}

INLINE CGparameter C_GLDevice::GetShaderParameter(CGprogram cgProgram, const char* name)
{
	return cgGetNamedParameter(cgProgram, name);
}

INLINE void C_GLDevice::SetAlphaEnable(const E_Bool alphaEnable)
{
	if (alphaEnable != m_AlphaEnable)
	{
		if (alphaEnable == E_Bool_True)
			glEnable(GL_ALPHA_TEST);
		else
			glDisable(GL_ALPHA_TEST);
	
		m_AlphaEnable = alphaEnable;
	}
}

INLINE void C_GLDevice::SetAlphaFunc(const E_CmpFunc alphaFunc)
{
	if (alphaFunc != m_AlphaFunc)
	{
		glAlphaFunc(GLCmpFunc(alphaFunc), (F32)m_AlphaRef / 255.f);
		
		m_AlphaFunc = alphaFunc;
	}
}

INLINE void C_GLDevice::SetAlphaRef(const U32 alphaRef)
{
	if (alphaRef != m_AlphaRef)
	{
		glAlphaFunc(GLCmpFunc(m_AlphaFunc), (F32)alphaRef / 255.f);
		
		m_AlphaRef = alphaRef;
	}
}

INLINE void C_GLDevice::SetBlendDst(const E_Blend blendDst)
{
	if (blendDst != m_BlendDst)
	{
		glBlendFunc(GLBlend(m_BlendSrc), GLBlend(blendDst));
		
		m_BlendDst = blendDst;
	}
}

INLINE void C_GLDevice::SetBlendEnable(const E_Bool blendEnable)
{
	if (blendEnable != m_BlendEnable)
	{
		if (blendEnable == E_Bool_True)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
		
		m_BlendEnable = blendEnable;
	}
}

INLINE void C_GLDevice::SetBlendMode(const E_BlendMode blendMode)
{
	if (m_BlendMode != blendMode)
    {
        switch (blendMode)
        {
		case E_BlendMode_Solid:
          	SetAlphaEnable(E_Bool_False);
			SetBlendEnable(E_Bool_False);
            break;

		case E_BlendMode_SolidAlphaTest:
			SetAlphaEnable(E_Bool_True);
			SetAlphaFunc(E_CmpFunc_Greater);
			SetAlphaRef(0x80);
			SetBlendEnable(E_Bool_False);
            break;

		case E_BlendMode_Alpha:
			SetAlphaEnable(E_Bool_True);
			SetAlphaFunc(E_CmpFunc_Greater);
			SetAlphaRef(0x00);
            SetBlendEnable(E_Bool_True);
			SetBlendSrc(E_Blend_SrcAlpha);
			SetBlendDst(E_Blend_InvSrcAlpha);
            SetBlendOp(E_BlendOp_Add);
            break;

		case E_BlendMode_AlphaPreMultiply:
			SetAlphaEnable(E_Bool_False);
			SetAlphaFunc(E_CmpFunc_Greater);
			SetAlphaRef(0x00);
            SetBlendEnable(E_Bool_True);
			SetBlendSrc(E_Blend_One);
			SetBlendDst(E_Blend_InvSrcAlpha);
            SetBlendOp(E_BlendOp_Add);
            break;

		case E_BlendMode_Additive:
			SetAlphaEnable(E_Bool_False);
            SetBlendEnable(E_Bool_True);
			SetBlendSrc(E_Blend_One);
			SetBlendDst(E_Blend_One);
            SetBlendOp(E_BlendOp_Add);
        break;

		case E_BlendMode_AlphaAdditive:
        	SetAlphaEnable(E_Bool_True);
			SetAlphaFunc(E_CmpFunc_Greater);
			SetAlphaRef(0x00);
			SetBlendEnable(E_Bool_True);
            SetBlendSrc(E_Blend_SrcAlpha);
			SetBlendDst(E_Blend_One);
            SetBlendOp(E_BlendOp_Add);
            break;

		case E_BlendMode_Subtractive:
            SetAlphaEnable(E_Bool_False);
			SetBlendEnable(E_Bool_True);
            SetBlendSrc(E_Blend_One);
			SetBlendDst(E_Blend_One);
            SetBlendOp(E_BlendOp_RevSubtract);
			break;
		
		case E_BlendMode_AdditiveSmooth:
			SetAlphaEnable(E_Bool_False);
            SetBlendEnable(E_Bool_True);
			SetBlendSrc(E_Blend_One);
			SetBlendDst(E_Blend_InvSrcColour);
            SetBlendOp(E_BlendOp_Add);
			break;

		default:
			SetAlphaEnable(E_Bool_False);
			SetBlendEnable(E_Bool_False);
            break;
        }

        m_BlendMode = blendMode;
    }
}

INLINE void C_GLDevice::SetBlendOp(const E_BlendOp blendOp)
{
	if (blendOp != m_BlendOp)
	{
		glBlendEquation(GLBlendOp(blendOp));
		
		m_BlendOp = blendOp;
	}
}

INLINE void C_GLDevice::SetBlendSrc(const E_Blend blendSrc)
{
	if (blendSrc != m_BlendSrc)
	{
		glBlendFunc(GLBlend(blendSrc), GLBlend(m_BlendDst));
		
		m_BlendSrc = blendSrc;
	}
}

INLINE void C_GLDevice::SetColourArray(const bool enable)
{
	if (enable != m_ColourArrayEnable)
    {
        if (enable)
            glEnableClientState(GL_COLOR_ARRAY);
        else
            glDisableClientState(GL_COLOR_ARRAY);
        
        m_ColourArrayEnable = enable;
    }
}

INLINE void C_GLDevice::SetColourWriteEnable(const U32 colourWriteEnable)
{
}

INLINE void C_GLDevice::SetColourSurface(const C_Surface* pSurface)
{
	SetColourSurface(0, pSurface);
}

INLINE void C_GLDevice::SetCullMode(E_CullMode cullMode)
{
	if (cullMode == E_CullMode_Default)
		cullMode = m_CullModeDefault;

	if (cullMode != m_CullMode)
	{
		switch (cullMode)
		{
		case E_CullMode_None:
			{
				glDisable(GL_CULL_FACE);
			}
			break;
		
		case E_CullMode_Cw:
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				//glCullFace(GL_BACK);
			}
			break;

		case E_CullMode_Ccw:
			{
				glEnable(GL_CULL_FACE);
				//glCullFace(GL_FRONT);
				glCullFace(GL_BACK);
			}
			break;
		
		default:
			BIZ_ASSERT_FAIL("Unsupported cull mode");
			break;
		}

		m_CullMode = cullMode;
	}
}

INLINE void C_GLDevice::SetDepthBias(const F32 depthBias)
{
}

INLINE void C_GLDevice::SetDepthEnable(E_Bool depthEnable)
{
	if (depthEnable == E_Bool_Default)
		depthEnable = m_DepthEnableDefault;
	if (depthEnable != m_DepthEnable)
	{
		if (depthEnable == E_Bool_True)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		m_DepthEnable = depthEnable;
	}
}

INLINE void C_GLDevice::SetDepthFlip(const bool depthCmpFlip, const bool depthViewportFlip)
{
	m_DepthCmpFlip = depthCmpFlip;
	m_DepthViewportFlip = depthCmpFlip && depthViewportFlip;

	m_DepthClear = m_DepthCmpFlip ? 0.f : 1.f;
}

INLINE void C_GLDevice::SetDepthFunc(const E_CmpFunc depthFunc)
{
	glDepthFunc( GLCmpFunc( depthFunc ) );
}
	
INLINE void C_GLDevice::SetDepthWriteEnable(const E_Bool depthWriteEnable)
{
	if (depthWriteEnable != m_DepthWriteEnable)
	{
		glDepthMask(depthWriteEnable == E_Bool_True ? true : false);

		m_DepthWriteEnable = depthWriteEnable;
	}
}

INLINE void C_GLDevice::SetFillMode(E_FillMode fillMode)
{
	if (fillMode == E_FillMode_Default)
		fillMode = m_FillModeDefault;

	if (fillMode != m_FillMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GLFillMode(fillMode));

		m_FillMode = fillMode;
	}
}

INLINE void C_GLDevice::SetIndexBuffer(const C_IndexBuffer* pIndexBuffer)
{
	if (pIndexBuffer != m_pIndexBuffer)
	{
		if (!m_SetVertexDeclaration)
			m_SetVertexDeclaration = true;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer ? pIndexBuffer->m_Name : 0);
		
		m_pIndexBuffer = pIndexBuffer;
	}
}

INLINE void C_GLDevice::SetNormalArray(const bool enable)
{
	if (enable != m_NormalArrayEnable)
    {
        if (enable)
            glEnableClientState(GL_NORMAL_ARRAY);
        else
            glDisableClientState(GL_NORMAL_ARRAY);
        
        m_NormalArrayEnable = enable;
    }
}

INLINE void C_GLDevice::SetPixelShader(const C_PixelShader* pPixelShader)
{
	if (pPixelShader != m_pPixelShader)
	{
		if (pPixelShader)
		{
			if (m_CurrentProfilePS != m_LatestProfilePS)
			{
				cgGLEnableProfile(m_cgLatestProfilePS);

				m_CurrentProfilePS = m_LatestProfilePS;
			}
			
			cgGLBindProgram(pPixelShader->m_cgProgram);
		}
		else
		{
			/*if (m_CurrentProfilePS != E_PixelShaderProfile_0_0)
			{
				cgGLDisableProfile(m_cgLatestProfilePS);

				m_CurrentProfilePS = E_PixelShaderProfile_0_0;
			}*/
		}

		m_pPixelShader = pPixelShader;
	}
}

INLINE void C_GLDevice::SetPixelShaderParameter(const C_PixelShader* pPixelShader, CGparameter cgParameter, F32* pValue)
{
	BIZ_ASSERT(cgParameter);
	BIZ_ASSERT(pValue);
	cgSetParameter4fv(cgParameter, pValue);
}

INLINE void C_GLDevice::SetPresentInterval(E_PresentInterval presentInterval)
{
	if (presentInterval == E_PresentInterval_Default)
		presentInterval = m_PresentIntervalDefault;

	if (presentInterval != m_PresentInterval)
	{
	#ifdef PLATFORM_PS3
		if (E_PresentInterval_Immediate)
			glDisable(GL_VSYNC_SCE);
		else
			glEnable(GL_VSYNC_SCE);
	#endif
		
		m_PresentInterval = presentInterval;
	}
}

INLINE void C_GLDevice::SetSlopeScaledBias(const F32 slopeScaledBias)
{
}

INLINE void C_GLDevice::SetTexcoordArray(const U32 sampler, const bool enable)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	glClientActiveTexture(GL_TEXTURE0 + sampler);

    if (enable != m_TexcoordArrayEnable[sampler])
    {
		if (enable)
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        else
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        m_TexcoordArrayEnable[sampler] = enable;
    }
}

INLINE void C_GLDevice::SetTexture(const U32 sampler, const C_Texture* pTexture)
{
	BIZ_ASSERT(sampler < E_Sampler_Count);

	if (pTexture != m_Sampler[sampler].m_pTexture)
	{
		glActiveTexture(GL_TEXTURE0 + sampler);

		if (pTexture)
		{
			glBindTexture(GL_TEXTURE_2D, pTexture->m_glID);

			if (!m_Sampler[sampler].m_Enabled)
			{
				glEnable(GL_TEXTURE_2D);

				m_Sampler[sampler].m_Enabled = true;
			}
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);

			if (m_Sampler[sampler].m_Enabled)
			{
				glDisable(GL_TEXTURE_2D);

				m_Sampler[sampler].m_Enabled = false;
			}
		}

		m_Sampler[sampler].m_pTexture = pTexture;
	}
}

INLINE void C_GLDevice::SetTextureAddressU(const U32 sampler, const E_TextureAddress addressU)
{
}

INLINE void C_GLDevice::SetTextureAddressV(const U32 sampler, const E_TextureAddress addressV)
{
}

INLINE void C_GLDevice::SetTextureAddressW(const U32 sampler, const E_TextureAddress addressW)
{
}

INLINE void C_GLDevice::SetTextureAddressUVW(const U32 sampler, const E_TextureAddress addressU, const E_TextureAddress addressV, const E_TextureAddress addressW)
{
}

INLINE void C_GLDevice::SetTextureBorderColour(const U32 sampler, const U32 borderColour)
{
}

INLINE void C_GLDevice::SetTextureGamma(const U32 sampler, const E_Bool gamma)
{
}

INLINE void C_GLDevice::SetTextureFilterMag(const U32 sampler, const E_TextureFilter filterMag)
{
}

INLINE void C_GLDevice::SetTextureFilterMin(const U32 sampler, const E_TextureFilter filterMin)
{
}

INLINE void C_GLDevice::SetTextureFilterMip(const U32 sampler, const E_TextureFilter filterMip)
{
}

INLINE void C_GLDevice::SetTextureFilterMagMinMip(const U32 sampler, const E_TextureFilter filterMag, E_TextureFilter filterMin, E_TextureFilter filterMip)
{
}

INLINE void C_GLDevice::SetVertexArray(const bool enable)
{
	if (enable != m_VertexArrayEnable)
    {
        if (enable)
            glEnableClientState(GL_VERTEX_ARRAY);
        else
            glDisableClientState(GL_VERTEX_ARRAY);
        
        m_VertexArrayEnable = enable;
    }
}

INLINE void C_GLDevice::SetVertexBuffer(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride)
{
	BIZ_ASSERT(stream < E_Stream_Count);

	if (pVertexBuffer != m_Stream[stream].m_pVertexBuffer || stride != m_Stream[stream].m_VertexStride  || offset != m_Stream[stream].m_VertexOffset)
	{
		if (!m_SetVertexDeclaration)
			m_SetVertexDeclaration = true;

		glBindBuffer(GL_ARRAY_BUFFER, pVertexBuffer ? pVertexBuffer->m_Name : 0);
		
		m_Stream[stream].m_pVertexBuffer = pVertexBuffer;
		m_Stream[stream].m_VertexStride = stride;
		m_Stream[stream].m_VertexOffset = offset;
	}
}

INLINE void C_GLDevice::SetVertexDeclaration(const C_VertexDeclaration* pVertexDeclaration)
{
	//if (pVertexDeclaration != m_pVertexDeclaration)
	{
		if (!m_SetVertexDeclaration)
			m_SetVertexDeclaration = true;

		m_pVertexDeclaration = pVertexDeclaration;
	}
}

INLINE void C_GLDevice::SetVertexShader(const C_VertexShader* pVertexShader)
{
	if (pVertexShader != m_pVertexShader)
	{
		if (pVertexShader)
		{
			if (m_CurrentProfileVS != m_LatestProfileVS)
			{
				cgGLEnableProfile(m_cgLatestProfileVS);

				m_CurrentProfileVS = m_LatestProfileVS;
			}

			cgGLBindProgram(pVertexShader->m_cgProgram);
		}
		else
		{
			/*if (m_CurrentProfileVS != E_VertexShaderProfile_0_0)
			{
				cgGLDisableProfile(m_cgLatestProfileVS);

				m_CurrentProfileVS = E_VertexShaderProfile_0_0;
			}*/
		}

		m_pVertexShader = pVertexShader;
	}
}

INLINE void C_GLDevice::SetVertexShaderParameter(CGparameter cgParameter, F32* pValue)
{
	BIZ_ASSERT(cgParameter);
	BIZ_ASSERT(pValue);
	cgSetParameter4fv(cgParameter, pValue);
}

INLINE void C_GLDevice::SetViewport(const C_Viewport& viewport)
{
	m_Viewport = viewport;
	glViewport(viewport.GetX(), viewport.GetY(), viewport.GetW(), viewport.GetH());
}

INLINE U32 C_GLDevice::GetFreeResourceMemory()
{
	return 0;
}
