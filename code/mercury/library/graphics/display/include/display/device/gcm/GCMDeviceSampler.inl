INLINE void C_GCMSampler::SetTexture(const C_Texture* pTexture)
{
	if (pTexture != m_pTexture)
	{
		if (!m_SetTexture)
			m_SetTexture = true;
		m_pTexture = pTexture;
	}
}

INLINE void C_GCMSampler::SetAddressU(const E_TextureAddress addressU)
{
	if (addressU != m_AddressU)
	{
		if (!m_SetAddress)
			m_SetAddress = true;
		m_AddressU = addressU;
	}
}

INLINE void C_GCMSampler::SetAddressV(const E_TextureAddress addressV)
{
	if (addressV != m_AddressV)
	{
		if (!m_SetAddress)
			m_SetAddress = true;
		m_AddressV = addressV;
	}
}

INLINE void C_GCMSampler::SetAddressW(const E_TextureAddress addressW)
{
	if (addressW != m_AddressW)
	{
		if (!m_SetAddress)
			m_SetAddress = true;
		m_AddressW = addressW;
	}
}

INLINE void C_GCMSampler::SetMagFilter(const E_TextureFilter filterMag)
{
	if (filterMag != m_FilterMag)
	{
		if (!m_SetFilter)
			m_SetFilter = true;
		m_FilterMag = filterMag;
	}
}

INLINE void C_GCMSampler::SetMinFilter(const E_TextureFilter filterMin)
{
	if (filterMin != m_FilterMin)
	{
		if (!m_SetFilter)
			m_SetFilter = true;
		m_FilterMin = filterMin;
	}
}

INLINE void C_GCMSampler::SetMinLod(const U16 minLod)
{
	if (minLod != m_MinLod)
	{
		if (!m_SetControl)
			m_SetControl = true;
		m_MinLod = minLod;
	}
}

INLINE void C_GCMSampler::SetMaxLod(const U16 maxLod)
{
	if (maxLod != m_MaxLod)
	{
		if (!m_SetControl)
			m_SetControl = true;
		m_MaxLod = maxLod;
	}
}

INLINE void C_GCMSampler::SetBorderColour(const U32 borderColour)
{
	if (borderColour != m_BorderColour)
	{
		if (!m_SetBorderColour)
			m_SetBorderColour = true;
		m_BorderColour = borderColour;
	}
}

INLINE void C_GCMSampler::Set(const U32 sampler)
{
	assert(sampler < E_Sampler_Count);

	if (m_SetTexture)
	{
		//cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);
		//if (m_pTexture)
			//cellGcmSetTexture(sampler, m_pTexture);
		m_SetTexture = false;
	}

	if (m_SetControl)
	{
		//cellGcmSetTextureControl(sampler, CELL_GCM_TRUE, m_MinLod, m_MaxLod, CELL_GCM_TEXTURE_MAX_ANISO_1);
		m_SetControl = false;
	}
	
	if (m_SetAddress)
	{
		//cellGcmSetTextureAddress(sampler, GCMTextureAddress(m_AddressU), GCMTextureAddress(m_AddressV), GCMTextureAddress(m_AddressW), CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_LESS, 0);
		m_SetAddress = false;
	}

	if (m_SetFilter)
	{
		//cellGcmSetTextureFilter(sampler, 0, GCMTextureFilter(m_FilterMin), GCMTextureFilter(m_FilterMag), CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
		m_SetFilter = false;
	}
}

INLINE void C_GCMSampler::SetDefaults(const U32 sampler)
{
	assert(sampler < E_Sampler_Count);

	m_pTexture = NULL;

	//cellGcmSetTextureControl(sampler, CELL_GCM_FALSE, m_MinLod, m_MaxLod, CELL_GCM_TEXTURE_MAX_ANISO_1);
	//cellGcmSetTextureAddress(sampler, GCMTextureAddress(m_AddressU), GCMTextureAddress(m_AddressV), GCMTextureAddress(m_AddressW), CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_LESS, 0);
	//cellGcmSetTextureFilter(sampler, 0, GCMTextureFilter(m_FilterMin), GCMTextureFilter(m_FilterMag), CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
	//cellGcmSetTextureBorderColor(sampler, m_BorderColour);
}
