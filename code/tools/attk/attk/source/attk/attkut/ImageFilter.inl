template<class PixelType>
Attk::C_Status C_ImageFilter::Filter2D(
	U8* pDstPixelData,
	Attk::C_Image& img,
	U32 newWidth,
	U32 newHeight,
	E_PixelFilter filter,
	U32 dstFilterWidth,
	U32 dstFilterHeight)
{
	return Filter<PixelType>(pDstPixelData, img, newWidth, newHeight, 1, filter, dstFilterWidth, dstFilterHeight, 1);
}

template<class PixelType>
Attk::C_Status C_ImageFilter::Filter(
	U8* pDstPixelData, 
	Attk::C_Image& img, 
	U32 newWidth, 
	U32 newHeight,
	U32 newDepth,
	C_ImageFilter::E_PixelFilter filter,
	U32 dstFilterWidth,
	U32 dstFilterHeight,
	U32 dstFilterDepth)
{
	switch(filter)
	{
		case C_ImageFilter::E_PixelFilter_Box:
			return Filter<PixelType, AttkUt::C_BoxFilter>(
				pDstPixelData,
				img,
				newWidth,
				newHeight,
				newDepth,
				dstFilterWidth,
				dstFilterHeight,
				dstFilterDepth);

		case C_ImageFilter::E_PixelFilter_Emboss:
			return Filter<PixelType, AttkUt::C_EmbossFilter>(
				pDstPixelData,
				img,
				newWidth,
				newHeight,
				newDepth,
				dstFilterWidth,
				dstFilterHeight,
				dstFilterDepth);

		case C_ImageFilter::E_PixelFilter_Sharpness:
			return Filter<PixelType, AttkUt::C_SharpnessFilter>(
				pDstPixelData,
				img,
				newWidth,
				newHeight,
				newDepth,
				dstFilterWidth,
				dstFilterHeight,
				dstFilterDepth);

		case C_ImageFilter::E_PixelFilter_Bartlett:
			return Filter<PixelType, AttkUt::C_BartlettFilter>(
				pDstPixelData,
				img,
				newWidth,
				newHeight,
				newDepth,
				dstFilterWidth,
				dstFilterHeight,
				dstFilterDepth);

		case E_PixelFilter_Unknown:
		default:
			return Attk::E_Status_Failure;
	};

	return Attk::E_Status_Success;
}


template<class PixelType, class FilterType>
static Attk::C_Status C_ImageFilter::Filter(
	U8* pDstPixelData, 
	Attk::C_Image& img, 
	U32 newWidth, 
	U32 newHeight,
	U32 newDepth,
	U32 dstFilterWidth,
	U32 dstFilterHeight,
	U32 dstFilterDepth)
{
	U8* pSrcPixelData = img.GetPixelDataPtr();

	//Set filter size as being equal to the size of a single pixel relative to our dest image.
	U32 srcDimensions[] = { img.GetWidth(), img.GetHeight(), img.GetDepth() };
	U32 dstDimensions[] = { newWidth, newHeight, newDepth };
	U32 dstFilterDimensions[] = { dstFilterWidth, dstFilterHeight, dstFilterDepth };

	//Create filter object
	AttkUt::C_PixelFilter<PixelType, FilterType> filterClass(
		pSrcPixelData,		// pointer to source image pixel data
		srcDimensions,
		dstDimensions,
		dstFilterDimensions);

	for (U32 dstX = 0; dstX < newWidth; dstX++) 
	{
		filterClass.SetSrcFilterPosition(0, dstX);

		for (U32 dstY = 0; dstY < newHeight; dstY++)
		{
//			std::cout << "DST: " << dstX << " " << dstY << std::endl;

			filterClass.SetSrcFilterPosition(1, dstY);

			for (U32 dstZ = 0; dstZ < newDepth; dstZ++)
			{
				filterClass.SetSrcFilterPosition(2, dstZ);

				U32 dstPixelIndex = Attk::GetPixelIndex(
					sizeof(PixelType),
					newWidth,
					newHeight,
					newDepth,
					dstX, 
					dstY,
					dstZ);
				#ifdef BUILD_CONFIG_DEBUG
				{
					U32 imageSize = newWidth * newHeight * newDepth * sizeof(PixelType);
					assert(dstPixelIndex < imageSize);
				}
				#endif

				PixelType& dstPixel = (PixelType&)pDstPixelData[dstPixelIndex];
				memset(&dstPixel, 0, sizeof(PixelType));

				filterClass.Filter(dstPixel);

				//Clamp
				for (U32 i = 0; i < 4; i++)
				{
					Attk::C_PixelR32G32B32A32F dstPixelF(dstPixel);
					if (dstPixelF.data[i] > 1.f)
						dstPixelF.data[i] = 1.f;
					else if (dstPixelF.data[i] < 0.f)
						dstPixelF.data[i] = 0.f;

					dstPixel.Set(dstPixelF);
				}
			}
		}
	}

	return Attk::E_Status_Success;
}
