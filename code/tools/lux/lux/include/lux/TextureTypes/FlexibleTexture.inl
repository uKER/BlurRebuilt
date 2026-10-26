#ifndef __FLEXIBLETEXTURE_INL__
#define __FLEXIBLETEXTURE_INL__

#pragma intrinsic(fabs)

// constructor
template<typename T>
FlexibleTexture<T>::FlexibleTexture(int x, int y, LUX::SurfaceIntegrator *pI) :
	ITexture(x,y,pI?pI->m_pGaussianFilter:NULL)
{
	Create(x,y);

	m_pRAWBuffer = NULL;
	Clear();
	m_context.SetIntegrator(pI);
}

// retrieve context data for this chart
template<typename T>
void FlexibleTexture<T>::GetCustomData(Ut::Vector<U8> &data)
{
	int n = m_context.GetSize();
	if (n == 0)
		return;
	data.Resize(n);
	U8 *p = &data[0];
	m_context.GetData(p);
}

// export RGBA values
template<typename T>
U32 *FlexibleTexture<T>::GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool *pbFoundQNAN)
{
	int n = m_cX * m_cY;

	float fInvGamma = 1.f/fGamma;

	if (n == 0)
	{
		if (pbFoundQNAN)
			*pbFoundQNAN = false;
		return NULL;
	}

	// creat buffer containing RAW data (if it doesn't exist already)
	if (!m_pRAWBuffer)
	{
		m_iStride = m_pTotalBuf[0].Scalars();
		m_pRAWBuffer = new float[m_iStride * n];
		float *pf = m_pRAWBuffer;

		for (int i=0; i<n; i++)
		{
			// apply brightness and gamma
			m_pTotalBuf[i] = m_pTotalBuf[i] * fBrightness;
			m_pTotalBuf[i].ApplyGammaCorrection(fInvGamma, m_context);

			// get data
			m_pTotalBuf[i].GetData(pf, m_context);
		}
	}

	// return a buffer containing this layer's data
	// caller receives ownership of the buffer
	U32 *pRGBA32Data = new U32[m_cX*m_cY];

	bool bFoundQNAN = false;

	// when the first layer gets grabbed, swizzle the SH nodes
	float *pfRAW = m_pRAWBuffer + iLayer*4;
	U32 *pOut = pRGBA32Data;
	int iUsed = m_iStride - iLayer*4;	// calculate how many components of this layer are used (force the rest to white)
	if (iUsed>4)
		iUsed = 4;
	for (int i=0; i<n; i++, pOut++, pfRAW+=m_iStride)
	{
		int c,iColour = LMAP_UNUSED_SPACE_COLOUR; // untouched pixels are coloured bright green
		U8 *pColour = (U8*)&iColour;

		if (CheckPixelUsed(i))
		{
			// calculate the RGBA32 value from the float3
			for (int j=0; j<iUsed; j++)
			{
				float f = pfRAW[j];

				bFoundQNAN |= CheckBuggeredFloat(f);					

				c = (int)(f*255.0f + 0.5f); // clamp
				if (c < 0)   c = 0;
				if (c > 255)
					c = 255;
				pColour[j] = c;
			}
			for (int j=iUsed; j<4; j++)
				pColour[j] = 255;

			if (iColour == TEXEL_UNUSED_COLOUR_VALUE)
				iColour = TEXEL_UNUSED_COLOUR_VALUE_PLUS_ONE;

			*pOut = iColour;
		}
		else
			*pOut = TEXEL_UNUSED_COLOUR_VALUE;
	}

	if (pbFoundQNAN)
		*pbFoundQNAN = bFoundQNAN;

	return pRGBA32Data;
}

// create a new texture
template<typename T>
void FlexibleTexture<T>::Create(int x, int y)
{
	m_pAccBuf = new SLMapPixel[x*y];
	m_pTotalBuf = new T[x*y];
}

// clear the contents of the texture
template<typename T>
void FlexibleTexture<T>::Clear()
{
	int c = m_cX * m_cY;
	memset(m_pAccBuf, 0, sizeof(SLMapPixel) * c);
	memset(m_pTotalBuf, 0, sizeof(T) * c);
	if (m_pRAWBuffer)
	{
		delete m_pRAWBuffer;
		m_pRAWBuffer = NULL;
	}

	ITexture::Clear();
}

// free this texture
template<typename T>
void FlexibleTexture<T>::Free()
{
	delete [] m_pAccBuf;
	delete [] m_pTotalBuf;
	if (m_pRAWBuffer)
		delete [] m_pRAWBuffer;

	m_bInitialized = false;

	ITexture::Free();
}

// add a T into the texture
template<typename T>
void FlexibleTexture<T>::AddSample(float2 t, const T &sample, const HMat33 &mtxTangent)
{
	int x0, x1, y0, y1;
	float *pfTable = m_pFilter->GetTable();
	//float f;
	int ifx[16];
	int ify[16];

	PrecalcSampleOffsets(t, m_pFilter, x0, y0, x1, y1, ifx, ify);

	for (int y = y0; y <= y1; ++y)
	{
		for (int x = x0; x <= x1; ++x)
		{
			// Evaluate filter value at (x,y) pixel
			int offset = ify[y-y0]*FILTER_TABLE_SIZE + ifx[x-x0];
			float filterWt = pfTable[offset];
			// Update pixel values with filtered sample contribution
			SLMapPixel *pPixel = &GetPixel(x, y);

			*(T*)pPixel = *(T*)pPixel + (sample * filterWt);

			pPixel->fWeight += filterWt;
		}
	}
}

// add a sample to the normal buffer
template<typename T>
void FlexibleTexture<T>::AddSample(float2 t, const LightSample &sample, const HMat33 &mtxTangent)
{
	T tmp = 0;
	tmp.SetSample(sample, mtxTangent, m_context);

	AddSample(t, tmp, mtxTangent);
}

// add a sample to the normal buffer
template<typename T>
void FlexibleTexture<T>::AddSample(float2 t, const Ut::Vector<LightSample> &samples, const HMat33 &mtxTangent)
{
	T acc = 0;
	for (int i=0; i<samples.Size(); i++)
	{
		T tmp = 0;
		tmp.SetSample(samples[i], mtxTangent, m_context);
		acc = acc + tmp;
	}

	AddSample(t, acc, mtxTangent);
}

// add a T to the discontinuity buffer
template<typename T>
void FlexibleTexture<T>::AddDiscontinuitySample(int iX, int iY, const T &sample, const HMat33 &mtxTangent)
{
	int x0 = iX-1;
	int x1 = iX+1;
	int y0 = iY-1;
	int y1 = iY+1;

	x0 = max(x0, m_window.x);
	x1 = min(x1, m_window.x + m_window.cx -1);
	y0 = max(y0, m_window.y);
	y1 = min(y1, m_window.y + m_window.cy -1);

	for (int y = y0; y <= y1; ++y)
	{
		for (int x = x0; x <= x1; ++x)
		{
			SLMapPixel *pPixel = &GetPixel(x, y);

			*(T*)pPixel = *(T*)pPixel + sample;

			pPixel->fWeight += 1.0f;
		}
	}
}

// add a sample to the discontinuity buffer
template<typename T>
void FlexibleTexture<T>::AddDiscontinuitySample(int iX, int iY, const LightSample &sample, const HMat33 &mtxTangent)
{
	T tmp = 0;
	tmp.SetSample(sample, mtxTangent, m_context);

	AddDiscontinuitySample(iX, iY, tmp, mtxTangent);
}

// add multiple samples to the discontinuity buffer
template<typename T>
void FlexibleTexture<T>::AddDiscontinuitySample(int iX, int iY, const Ut::Vector<LightSample> &samples, const HMat33 &mtxTangent)
{
	T acc = 0;
	for (int i=0; i<samples.Size(); i++)
	{
		T tmp = 0;
		tmp.SetSample(samples[i], mtxTangent, m_context);
		acc = acc + tmp;
	}

	AddDiscontinuitySample(iX, iY, acc, mtxTangent);
}

// convolve all added samples into the intermediate buffer
template<typename T>
void FlexibleTexture<T>::Resolve()
{
	for (int i=0; i<m_cX*m_cY; i++)
	{
		if (!CheckPixelUsed(i) || m_pAccBuf[i].fWeight<0.000000001) continue;

		// divide the accumulated result by the weight and add it to the total buffer
		float f = 1.f/m_pAccBuf[i].fWeight;

		m_pTotalBuf[i] = m_pTotalBuf[i] + (T)m_pAccBuf[i] * f;

		(T&)m_pAccBuf[i] = 0;
		m_pAccBuf[i].fWeight = 0;
	}	
}

// get number of texture layers needed
template<typename T>
int	FlexibleTexture<T>::GetNumLayers()
{
	return (m_pTotalBuf[0].Scalars()+3)/4;
}

// returns true if this texture needs tangent space to render (depends on T)
template<typename T>
bool FlexibleTexture<T>::NeedTangentSpace()
{
	return LUX::DataTypes::NeedTangentSpace<T>();
}

// merge the values of two texels. this function is used by the anti chart seam code
template<typename T>
void FlexibleTexture<T>::MergeTexels(float2 t1, ITexture *_pOtherTexture, float2 t2,
									int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	FlexibleTexture<T> *pOtherTexture = (FlexibleTexture<T>*)_pOtherTexture;

#ifdef ENABLE_MERGETEXELS_DEBUG_CODE
	if (int(t1.x)>=0 && int(t1.x)<m_cX &&
		int(t1.y)>=0 && int(t1.y)<m_cY)
	{
		T  *p1 = m_pTotalBuf + (m_cX * (int)t1.y + (int)t1.x);
		*p1 = 1;
	}

	if (int(t2.x)>=0 && int(t2.x)<pOtherTexture->m_cX  &&
		int(t2.y)>=0 && int(t2.y)<pOtherTexture->m_cY)
	{
		T  *p2 = pOtherTexture->m_pTotalBuf + (pOtherTexture->m_cX * (int)t2.y + (int)t2.x);
		*p2 = 1;
	}

	return;
#endif

	// seems to fire erroneously
//	assert(x1 >= 0 && y1 >= 0 && x2 >= 0 && y2 >= 0 &&
//		x1+w1 <= m_cX && x2+h2 <= m_cY && x2+w2 <= pOtherTexture->m_cX && y2+h2 <= pOtherTexture->m_cY);

	// check that both the positions aren in bounds
	if (int(t1.x)<x1 || int(t1.x)>=x1+w1 ||
		int(t1.y)<y1 || int(t1.y)>=y1+h1 ||
		int(t2.x)<x2 || int(t2.x)>=x2+w2 ||
		int(t2.y)<y2 || int(t2.y)>=y2+h2 ||
		int(t1.x)<0 || int(t1.x)>=m_cX ||
		int(t1.y)<0 || int(t1.y)>=m_cY ||
		int(t2.x)<0 || int(t2.x)>=pOtherTexture->m_cX ||
		int(t2.y)<0 || int(t2.y)>=pOtherTexture->m_cY)
	{
		// shouldn't happen in theory, but sometimes it does
		return;
	}

	// fetch texels
	T  *p1 = m_pTotalBuf + (m_cX * (int)t1.y + (int)t1.x);
	T  *p2 = pOtherTexture->m_pTotalBuf + (pOtherTexture->m_cX * (int)t2.y + (int)t2.x);

	// average
	T vAvg;
	bool bUsed1 = CheckPixelUsed((int)t1.x,(int)t1.y);
	bool bUsed2 = pOtherTexture->CheckPixelUsed((int)t2.x,(int)t2.y);
	if (!bUsed1 && !bUsed2)
		return;
	else
	if (bUsed1 && !bUsed2)
		vAvg = *p1;
	else if (!bUsed1 && bUsed2)
		vAvg = *p2;
	else if (bUsed1 && bUsed2)
		vAvg = ((*p1) + (*p2)) * 0.5f;
	*p1 = vAvg;
	*p2 = vAvg;

}

#endif

