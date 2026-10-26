#ifndef ATTK_FLOOD_FILL_H
#define ATTK_FLOOD_FILL_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AttkUt
{
	class C_FloodFill
	{
	public:
		// Performs a flood filling operation on an image.
		// Smears image over pixels with 0 alpha
		static Attk::C_Status FloodFill(Attk::C_Image& dest, const Attk::C_Image& src);

	private:

		template <typename T>
		static F32 NormalizePixelData(T pixelData)
		{
			if (pixelData > 1.0f)
				return (F32) pixelData/255.f;
			else
				return pixelData;
		}

		template <typename PixFormat>
		static Attk::C_Status PerformFloodFill(PixFormat* destPixels, const PixFormat* srcPixels, U32 width, U32 height, Attk::E_PixelFormat ePixFormat)
		{
			U32 filter[3][3] = 
			{
				1, 2, 1,
				2, 0, 2,
				1, 2, 1
			};

			// Copy source pixels into a temp image so we can flip between it and the dest image when applying the filter
			Attk::C_Image tempImage;
			Attk::C_Status status = tempImage.NewPixelData(width, height, 1, ePixFormat, (const U8*)srcPixels);
			if (!status)
				return status;

			PixFormat* imageBuffers[2];
			imageBuffers[0] = (PixFormat*) tempImage.GetPixelDataPtr();
			imageBuffers[1] = destPixels;

			U32 source = 0;
			U32 dest = 1;

			//Preprocess image: blank out pixels with low alpha values
			for (U32 y=0; y<height; ++y)
			{
				for (U32 x=0; x<width; ++x)
				{
					PixFormat& srcPixel = imageBuffers[source][y*width + x];
					if (NormalizePixelData(srcPixel.alpha) < 0.51f)
						memset(&srcPixel, 0, sizeof(PixFormat));
				}
			}

			bool modified;
			bool isWritingAlpha;
			do
			{
				modified = false;
				isWritingAlpha = false;	// Need to ensure we are actually writing alpha values > 0 or we end up in an infinite loop!

				for (U32 y=0; y<height; ++y)
				{
					for (U32 x=0; x<width; ++x)
					{
						PixFormat& srcPixel = imageBuffers[source][y*width + x];
						PixFormat& destPixel = imageBuffers[dest][y*width + x];
						destPixel = srcPixel;

						if (NormalizePixelData(srcPixel.alpha) > FLT_EPSILON)
							continue;

						modified = true;

						C_PixelR32G32B32A32F accumulate;
						memset(&accumulate, 0, sizeof(C_PixelR32G32B32A32F));

						U32 numAccum = 0;

						for (U32 yFilter=0; yFilter<3; ++yFilter)
						{
							for (U32 xFilter=0; xFilter<3; ++xFilter)
							{
								U32 xKernel = x - 1 + xFilter;
								U32 yKernel = y - 1 + yFilter;

								if ( yKernel < 0 || yKernel >= height || xKernel < 0 || xKernel >= width )
									continue;

								PixFormat& kernelPixel = imageBuffers[source][yKernel*width + xKernel];
								if (NormalizePixelData(kernelPixel.alpha) < FLT_EPSILON)
									continue;

								U32 weight = filter[yFilter][xFilter];

								accumulate.red += kernelPixel.red * weight;
								accumulate.green += kernelPixel.green * weight;
								accumulate.blue += kernelPixel.blue * weight;
								accumulate.alpha += kernelPixel.alpha * weight;

								numAccum += weight;
							}
						}

						if (numAccum)
						{
							destPixel.red += (PixFormat::ComponentType) (accumulate.red / numAccum);
							destPixel.green += (PixFormat::ComponentType) (accumulate.green / numAccum);
							destPixel.blue += (PixFormat::ComponentType) (accumulate.blue  / numAccum);
							destPixel.alpha += (PixFormat::ComponentType) (accumulate.alpha  / numAccum);
							if (NormalizePixelData(accumulate.alpha) > numAccum*FLT_EPSILON)
								isWritingAlpha = true;
						}
					}
				}
				source = 1-source;
				dest = 1-dest;
			}
			while (modified && isWritingAlpha);
		
			// reapply alpha
			for (U32 y=0; y<height; ++y)
				for (U32 x=0; x<width; ++x)
					destPixels[y*width + x].alpha = srcPixels[y*width + x].alpha;

			return Attk::E_Status_Success;
		}
	};
}

#endif
