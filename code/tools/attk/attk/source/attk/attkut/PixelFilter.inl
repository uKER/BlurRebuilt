static F32 RemoveDecimal(F32 number)
{
    if (number >= 0)
		return number - F32(S32(number));
    return F32(S32(number)) - number;
}



template<class PixelType, class Impl>
C_PixelFilter<PixelType, Impl>::C_PixelFilter(
	const U8*	pSrcPixelData,
	U32*		srcDimensions,
	U32*		dstDimensions,
	U32*		dstFilterDimensions)
{
	m_pSrcPixelData = pSrcPixelData;
	memcpy(m_srcDimensions, srcDimensions, sizeof(S32)*3);
	memcpy(m_dstDimensions, dstDimensions, sizeof(S32)*3);

	//Calculate default filter sizes if needed.
	for (U32 i = 0; i < 3; i++)
	{
		//The scaling in our current axis.
		F32 scale = (F32)dstDimensions[i] / (F32)srcDimensions[i];

		//Filter size has been explicitly specified.
		m_dstPixelCoverage[i] = (F32)dstFilterDimensions[i];

		//Calculate filter size in the space of the source image.
		m_srcPixelCoverage[i] = m_dstPixelCoverage[i] * 1.f / scale;
	}
}



template<class PixelType, class Impl>
void C_PixelFilter<PixelType, Impl>::SetSrcFilterPosition(U32 axis, S32 dstLinePos)
{
	const F32 filterWidth = m_dstPixelCoverage[axis]; // number of filter elements in our line.

	//Offset our dstpos such that it's the pixel center.
	F32 dstLinePosf = (F32)dstLinePos + 0.5f;

	//Parameter describing our dst / src position within the 0..1 range.
	F32 u = dstLinePosf / (F32)m_dstDimensions[axis];

	//[OUTPUT] - Our filter origin as a floating-point position in pixels within
	//our source image.
	m_srcOrigin[axis] = (F32)m_srcDimensions[axis] * u;
}



template<class PixelType, class Impl>
void C_PixelFilter<PixelType, Impl>::Filter(PixelType& dstPixel)
{
	U32 nPixelsToFilter = m_impl.GetMatrixSize() * m_impl.GetMatrixSize() * m_impl.GetMatrixSize();
	U32 nPixelsFiltered = 0;
	//Note: m_dstPixelCoverage stores the number of destination pixels covered per
	//pixel in our source image.
	//
	//So if we're reducing an image 50% then the default for m_dstPixelCoverage[] = { 2,2 }
	//This is are the dimensions of our filter in our _SOURCE_ image. So, in other words 4 pixels
	//in our source image get filtered into a single pixel in our destination image.
	U32 matrixSize = m_impl.GetMatrixSize();
	F32 matrixElementSizeInSrcPixels[] = 
	{ 
		m_srcPixelCoverage[0] / (F32)matrixSize, 
		m_srcPixelCoverage[1] / (F32)matrixSize,
		m_srcPixelCoverage[2] / (F32)matrixSize
	};
	F32 matrixElementSizeInDstPixels[] = 
	{
		m_dstPixelCoverage[0] / (F32)matrixSize, 
		m_dstPixelCoverage[1] / (F32)matrixSize,
		m_dstPixelCoverage[2] / (F32)matrixSize
	};

	//Loop thru z-axis of our filter within the source image
	for (U32 kFilterZ = 0; kFilterZ < matrixSize; kFilterZ++)
	{
		F32 pxStepZ = 1.f / ((F32)matrixSize + 1.f);

		//Position in our filter matrix in the 0..1 range.
		F32 uZ = (F32)kFilterZ / (F32)matrixSize;

		F32 srcFilterElementCentreZ = m_srcOrigin[2] + (uZ+pxStepZ) * m_dstPixelCoverage[2] - (m_dstPixelCoverage[2]*0.5f);
		uZ += 1.f / (F32)matrixSize;

		//Loop thru y-axis of our filter within the source image.
		for (U32 jFilterY = 0; jFilterY < matrixSize; jFilterY++)
		{
			F32 pxStepY = 1.f / ((F32)matrixSize + 1.f);

			//Position in our filter matrix in the 0..1 range.
			F32 uY = (F32)jFilterY / (F32)matrixSize;

			F32 srcFilterElementCentreY = m_srcOrigin[1] + (uY+pxStepY) * m_dstPixelCoverage[1] - (m_dstPixelCoverage[1]*0.5f);
			uY += 1.f / (F32)matrixSize;

			//Loop thru x-axis of our filter within the source image.
			for (U32 iFilterX = 0; iFilterX < matrixSize; iFilterX++)
			{
				F32 pxStepX = 1.f / ((F32)matrixSize + 1.f);

				//Position in our filter matrix in the 0..1 range.
				F32 uX = (F32)iFilterX / (F32)matrixSize;

				F32 srcFilterElementCentreX = m_srcOrigin[0] + (uX+pxStepX) * m_dstPixelCoverage[0] - (m_dstPixelCoverage[0]*0.5f);
				uX += 1.f / (F32)matrixSize;

				Attk::C_PixelR32G32B32A32F averagedSrcPixel(0,0,0,0);
				//Within each filter matrix element we must average all of the pixels.
				//We then use the result of this averaging with our filter matrix.
				{
					F32 nSamples = 0;

					F32 srcLowerElementBoundZ = srcFilterElementCentreZ - matrixElementSizeInSrcPixels[2]*0.5f;
					F32 srcUpperElementBoundZ = srcFilterElementCentreZ + matrixElementSizeInSrcPixels[2]*0.5f;
					F32 matrixElementStepZ = matrixElementSizeInSrcPixels[2];
					if (matrixElementStepZ > 1.f)
						matrixElementStepZ = 1.f;
					for (F32 srcFilterZ = srcLowerElementBoundZ; srcFilterZ < srcUpperElementBoundZ; srcFilterZ += matrixElementStepZ)
					{
						F32 pixelWeight[] = {1.f,1.f, 1.f};
						if (matrixElementSizeInDstPixels[2] - srcFilterZ < 1.f)
							pixelWeight[2] = 1.f - RemoveDecimal(matrixElementSizeInDstPixels[2] - srcFilterZ);

						F32 srcPixelsToSampleForMatrixElementY = matrixElementSizeInSrcPixels[1];

						F32 srcLowerElementBoundY = srcFilterElementCentreY - matrixElementSizeInSrcPixels[1]*0.5f;
						F32 srcUpperElementBoundY = srcFilterElementCentreY + matrixElementSizeInSrcPixels[1]*0.5f;
						F32 matrixElementStepY = matrixElementSizeInSrcPixels[1];
						if (matrixElementStepY > 1.f)
							matrixElementStepY = 1.f;
						for (F32 srcFilterY = srcLowerElementBoundY; srcFilterY < srcUpperElementBoundY; srcFilterY += matrixElementStepY)
						{
							if (matrixElementSizeInDstPixels[1] - srcFilterY < 1.f)
								pixelWeight[1] = 1.f - RemoveDecimal(matrixElementSizeInDstPixels[1] - srcFilterY);

							F32 srcPixelsToSampleForMatrixElementX = matrixElementSizeInSrcPixels[0];

							F32 srcLowerElementBoundX = srcFilterElementCentreX - matrixElementSizeInSrcPixels[0]*0.5f;
							F32 srcUpperElementBoundX = srcFilterElementCentreX + matrixElementSizeInSrcPixels[0]*0.5f;
							F32 matrixElementStepX = matrixElementSizeInSrcPixels[0];
							if (matrixElementStepX > 1.f)
								matrixElementStepX = 1.f;
							for (F32 srcFilterX = srcLowerElementBoundX; srcFilterX < srcUpperElementBoundX; srcFilterX += matrixElementStepX)
							{
								if (matrixElementSizeInDstPixels[0] - srcFilterX < 1.f)
									pixelWeight[0] = 1.f - RemoveDecimal(matrixElementSizeInDstPixels[0] - srcFilterX);

								PixelType srcPixel;
								U32 srcPixelIndex = GetSrcPixel(
									srcPixel,
									m_pSrcPixelData,
									m_srcDimensions,
									srcFilterX,
									srcFilterY,
									srcFilterZ);
						
								F32 pixelAreaWeight = pixelWeight[0] * pixelWeight[1] * pixelWeight[2];
								nSamples+=pixelAreaWeight;
								averagedSrcPixel = averagedSrcPixel + Attk::C_PixelR32G32B32A32F(srcPixel) * pixelAreaWeight;
							}
						}
					}
					//Divide our accumulated pixel value by the number of pixels sampled in order
					//to get our averaged result.
					averagedSrcPixel = averagedSrcPixel * (1.f / nSamples);
				}
				dstPixel.Set(Attk::C_PixelR32G32B32A32F(dstPixel) + averagedSrcPixel * Evaluate(uX, uY, uZ));
				nPixelsFiltered++;
			}
		}
	}

	//FIX: Assumes 4-component pixel with 0..1 range.
	Attk::C_PixelR32G32B32A32F dstPixelF(dstPixel);
	for (U32 i = 0; i < 4; i++)
	{
		dstPixelF.data[i] += 1.f * m_impl.GetOutputOffset();
	}
	dstPixel.Set(dstPixelF);

	assert(nPixelsFiltered == nPixelsToFilter);
}



template<class PixelType, class Impl>
F32 C_PixelFilter<PixelType, Impl>::Evaluate(F32 u, F32 v, F32 w) const
{
	U32 numFilterElements = m_impl.GetMatrixSize();
	//Quantise 0..1 u,v values to the ith and jth row in our filter matrix.
	U32 i = (U32)(u * (numFilterElements-1));
	U32 j = (U32)(v * (numFilterElements-1));
	U32 k = (U32)(w * (numFilterElements-1));

	return m_impl.Evaluate(i, j, k);
}



template<class PixelType, class Impl>
U32 C_PixelFilter<PixelType, Impl>::GetSrcPixel(
	PixelType& srcPixel,

	const U8* pSrcPixelData,
	const S32* srcImageDimensions,
	const F32 srcFilterX, 
	const F32 srcFilterY,
	const F32 srcFilterZ)
{
	//Quantise
	S32 srcX = (S32)(srcFilterX);
	S32 srcY = (S32)(srcFilterY);
	S32 srcZ = (S32)(srcFilterZ);

//	std::cout << "   " << srcX << " " << srcY << std::endl;
	if (srcX < 0)
		srcX = 0;
	else if (srcX >= (S32)srcImageDimensions[0])
		srcX = (S32)srcImageDimensions[0]-1;

	if (srcY < 0)
		srcY = 0;
	else if (srcY >= (S32)srcImageDimensions[1])
		srcY = (S32)srcImageDimensions[1]-1;

	if (srcZ < 0)
		srcZ = 0;
	else if (srcZ >= (S32)srcImageDimensions[2])
		srcZ = (S32)srcImageDimensions[2]-1;


	U32 srcPixelIndex = Attk::GetPixelIndex(
		sizeof(PixelType),
		srcImageDimensions[0], srcImageDimensions[1], srcImageDimensions[2],
		srcX, srcY, srcZ);

	#ifdef BUILD_CONFIG_DEBUG
	{
		U32 imageSize = srcImageDimensions[0] * srcImageDimensions[1] * srcImageDimensions[2] * sizeof(PixelType);
		assert(srcPixelIndex < imageSize);
	}
	#endif

	srcPixel = (PixelType&)pSrcPixelData[srcPixelIndex];
	return srcPixelIndex;
}

