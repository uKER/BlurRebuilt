//-----------------------------------------------------------------------------
// File: SHTexture.inl
//
// Desc: Store the lightmap data colour values, sizes, etc.
//       This is the spherical harmonic texture type which stores a SH
//       vector per texel.
//
//-----------------------------------------------------------------------------

// create a new texture
template <int iSHOrder>
void SHTexture<iSHOrder>::Create(int x, int y)
{
	m_pData = new SLMapPixel[x*y];
	m_pDData = new SLMapPixel[x*y];
	m_pIntermediateBuffer = new SLMapPixel[x*y];
}

// clear texture data
template <int iSHOrder>
void SHTexture<iSHOrder>::Clear()
{
	int c = m_cX * m_cY;
	memset(m_pData, 0, sizeof(SLMapPixel) * c);
	memset(m_pDData, 0, sizeof(SLMapPixel) * c);
	memset(m_pIntermediateBuffer, 0, sizeof(SLMapPixel) * c);

	ITexture::Clear();
}

// update weights - also optionally converts colours to mono channels
template <int iSHOrder>
void SHTexture<iSHOrder>::UpdateWeights(LUX::RenderChannel *pChannel)
{
	float tmp[ iSHOrder*iSHOrder ];

	for (int i=0; i<m_cX*m_cY; i++)
	{
		if (!CheckPixelUsed(i) || m_pData[i].fWeight<0.000000001) continue;

		// divide the accumulated result by the weight and add it to the total buffer
		for (int c=0; c<3; c++)
		{
			SHScale( tmp, iSHOrder, m_pData[i].fSH[c], 1.0f/m_pData[i].fWeight );
			SHAdd( m_pIntermediateBuffer[i].fSH[c], iSHOrder, m_pIntermediateBuffer[i].fSH[c], tmp );
		}
		memset(&m_pData[i], 0, sizeof(SLMapPixel));		// reset accumulation buffer
		m_pData[i].fWeight = 0;
	}
}

// free this texture
template <int iSHOrder>
void SHTexture<iSHOrder>::Free()
{
	delete [] m_pData;
	delete [] m_pDData;
	delete [] m_pIntermediateBuffer;

	m_bInitialized = false;

	ITexture::Free();
}

// add a sample to this texture
template <int iSHOrder>
void SHTexture<iSHOrder>::AddSample(float2 t, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent)
{
	static int ifx[16];
	static int ify[16];
	int x0, x1, y0, y1;
	float tmp[3][ iSHOrder*iSHOrder ];
	float *pfTable = m_pFilter->GetTable();

	PrecalcSampleOffsets(t, m_pFilter, x0, y0, x1, y1, ifx, ify);

	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x)
		{
			// Evaluate filter value at (x,y) pixel
			int offset = ify[y-y0]*FILTER_TABLE_SIZE + ifx[x-x0];
			float filterWt = pfTable[offset];
			// Update pixel values with filtered sample contribution
			SLMapPixel *pPixel = &GetPixel(x, y);

			// encode the sample into a SH vector and add it to the accumulation buffer
			SHEvalDirectionalLight( iSHOrder, &vNormal.x,
				vSample.x * filterWt, vSample.y * filterWt, vSample.z * filterWt, tmp[0], tmp[1], tmp[2]);
			// add the result to the accumulation buffer
			for (int c=0; c<3; c++)
				SHAdd( pPixel->fSH[c], iSHOrder, pPixel->fSH[c], tmp[c] );

			pPixel->fWeight += filterWt;
		}
}

// add a sample to this texture using a given filter
template <int iSHOrder>
void SHTexture<iSHOrder>::AddDiscontinuitySample(int iX, int iY, const Ut::Vector<float3> &samples, const float3 &vNormal, const HMat33 &mtxTangent)
{
	// TODO: implement this properly
	AddDiscontinuitySample(iX, iY, samples[0], vNormal, mtxTangent);
}

extern bool gbAlternativeGamma;

// export RGBA values 
template <int iSHOrder>
U32 *SHTexture<iSHOrder>::GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool bAlphaScaling, bool bSwizzleSH)
{
	// TODO: contrast, etc.
	float f;
	float scale = fBrightness;

	// gamma correction
	float invGamma = 1.0f / fGamma;

	bool bGamma = (fGamma != 1.0f);

	// malloc new data
	U32 *m_pRGBA32Data = new U32[m_cX*m_cY];

	U32 *pOut = m_pRGBA32Data;

	// when the first layer gets grabbed, swizzle the SH nodes
	int iAvgLayer = 0;
	if (bSwizzleSH)
	{
		iAvgLayer = 0;

		if (iLayer == 0)
		{
			for (int i=0; i<m_cX*m_cY; i++)
			{
				PreScaleSHVector( m_pIntermediateBuffer[i].fSH[0] );
				PreScaleSHVector( m_pIntermediateBuffer[i].fSH[1] );
				PreScaleSHVector( m_pIntermediateBuffer[i].fSH[2] );
			}
		}
	}

	for (int i=0; i<m_cX*m_cY; i++, pOut++)
	{
		int c,iColour = LMAP_UNUSED_SPACE_COLOUR; // untouched pixels are coloured bright green
		U8 *pColour = (U8*)&iColour;

		if (CheckPixelUsed(i))
		{
			float m = 1.0f;
			float m2 = 1.0f;

			// get main buffer colour
			float3 vColour;

			vColour.x = m_pIntermediateBuffer[i].fSH[0][iLayer];
			vColour.y = m_pIntermediateBuffer[i].fSH[1][iLayer];
			vColour.z = m_pIntermediateBuffer[i].fSH[2][iLayer];

			vColour *= scale;

			float3 vBaseCol(	m_pIntermediateBuffer[i].fSH[0][iAvgLayer],
								m_pIntermediateBuffer[i].fSH[1][iAvgLayer],
								m_pIntermediateBuffer[i].fSH[2][iAvgLayer]);

			vBaseCol *= scale;

			// calculate the RGBA32 value from the float3
			for (int j=0; j<3; j++)
			{
				f = vColour[j];

				if (f>0 && bGamma)
				{
					if (!gbAlternativeGamma)
					{
						// calculate the amount of gamma scaling on the base colour and apply that to this component as well
						float fx = powf(vBaseCol[j], invGamma);
						f *= fx / vBaseCol[j];
					}
					else
					{
						// calculate gamma, only on the main layer and using the hack for Paul Malin
						if (iLayer == iAvgLayer)
							f = powf(f, invGamma);
						else
						{
							float fSign = (f>=0)?1:-1;
							f *= fSign;
							f = powf(f,invGamma);
							f *= fSign;
						}
					}
				}

				if (iLayer != iAvgLayer)
					f = f * 0.5f + 0.5f;

				c = (int)(f*255.0f + 0.5f); // clamp
				if (c < 0)   c = 0;
				if (c > 255)
					c = 255;
				pColour[j] = c;
			}

			if (iColour == TEXEL_UNUSED_COLOUR_VALUE)
				iColour = TEXEL_UNUSED_COLOUR_VALUE_PLUS_ONE;

			*pOut = iColour;
		}
		else
			*pOut = TEXEL_UNUSED_COLOUR_VALUE;
	}

	return m_pRGBA32Data;
}

// blur the texture using a 3x3 filter kernel - result is added to intermediate buffer
template <int iSHOrder>
void SHTexture<iSHOrder>::Filter(LUX::RenderChannel *pChannel)
{
	for(int x = 0; x < m_cX; x++)
	{
		for(int y = 0; y < m_cY; y++)
		{
			int i = x + y * m_cX;
			if (!CheckPixelUsed(i)) continue;

			float total[3][ iSHOrder*iSHOrder ];
			memset(total, 0, sizeof(total));

			float fTotal = 0;
			for (int dx=-1; dx<2; dx++)
			{
				for (int dy=-1; dy<2; dy++)
				{
					int tx = x + dx;
					int ty = y + dy;

					if (tx<0 || ty<0 || tx>=m_cX || ty>=m_cY)
						continue;

					SLMapPixel *pSrc = &m_pDData[ tx + ty * m_cX ];

					// add the 9 adjacent texel values together
					for (int c=0; c<3; c++)
						SHAdd( total[c], iSHOrder, pSrc->fSH[c], total[c] );

					fTotal += pSrc->fWeight;
				}
			}

			// store the result
			for (int c=0; c<3; c++)
			{
				SHScale( total[c], iSHOrder, total[c], 1.0f/fTotal );
				SHAdd( m_pIntermediateBuffer[i].fSH[c], iSHOrder, total[c], m_pIntermediateBuffer[i].fSH[c] );
			}
		}
	}
}

// blur the texture using a 3x3 filter kernel - result is added to intermediate buffer
template <int iSHOrder>
void SHTexture<iSHOrder>::Filter(const Ut::Vector<LUX::RenderChannel*> &channels)
{
	// TODO: implement this properly
	Filter(channels[0]);
}

template <int iSHOrder>
void SHTexture<iSHOrder>::AddDiscontinuitySample(int iX, int iY, const float3 &vSample, const float3 &vNormal, const HMat33 &mtxTangent)
{
	SLMapPixel *pPixel = &m_pDData[iX + iY*m_cX];

	float tmp[3][ iSHOrder*iSHOrder ];

	// encode the sample into a SH vector and add it to the accumulation buffer
	SHEvalDirectionalLight( iSHOrder, &vNormal.x,
		vSample.x, vSample.y, vSample.z, tmp[0], tmp[1], tmp[2]);
	// add the result to the accumulation buffer
	for (int c=0; c<3; c++)
		SHAdd( pPixel->fSH[c], iSHOrder, pPixel->fSH[c], tmp[c] );

	pPixel->fWeight += 1.0f;
}

template <int iSHOrder>
int	SHTexture<iSHOrder>::GetNumLayers()
{
	return iSHOrder*iSHOrder;
}

// merge the values of two texels. this function is used by the anti chart seam code
template <int iSHOrder>
void SHTexture<iSHOrder>::MergeTexels(float2 t1, ITexture *_pOtherTexture, float2 t2)
{
	SHTexture<iSHOrder> *pOtherTexture = (SHTexture<iSHOrder>*)_pOtherTexture;

	// check that both the positions aren in bounds
	if (int(t1.x)<0 || int(t1.x)>=m_cX ||
		int(t1.y)<0 || int(t1.y)>=m_cY ||
		int(t2.x)<0 || int(t2.x)>=pOtherTexture->m_cX ||
		int(t2.y)<0 || int(t2.y)>=pOtherTexture->m_cY)
	{
		assert(0); // shouldn't happen in theory
		return;
	}

	// fetch texels
	SLMapPixel  *p1 = m_pIntermediateBuffer + (pOtherTexture->m_cX * (int)t1.y + (int)t1.x);
	SLMapPixel  *p2 = pOtherTexture->m_pIntermediateBuffer + (pOtherTexture->m_cX * (int)t2.y + (int)t2.x);

	// average
	SLMapPixel vAvg;
	for (int i=0; i<3; i++)
	{
		for (int j=0; j<iSHOrder *iSHOrder; j++)
		{
			vAvg.fSH[j][i] = (p1->fSH[j][i] + p2->fSH[j][i]) * 0.5f;
		}
	}

	*p1 = vAvg;
	*p2 = vAvg;
}
