#include "stdafx.h"

using namespace AttkUt;

Attk::C_Status C_GenerateNormalMap::GenerateNormalMap(
	Attk::C_Image2D& dest, 
	const Attk::C_Image2D& src)
{
	//Initialise destination to source.
	dest = src;

	Attk::C_PixelR32G32B32A32F pix;
	F32 dX, dY;
	F32 nX, nY, nZ;

	S32 height = src.GetHeight();
	S32 width = src.GetWidth();

	for (S32 y = 0; y < height; y++)
    {
		for (S32 x = 0; x < width; x++)
		{
			// Do Y Sobel filter
			pix = src.GetPixel((x-1+width) % width, (y+1) % height);
			dY  = pix.red * -1.0f;
		       
			pix = src.GetPixel(x % width, (y+1) % height);
			dY += pix.red * -2.0f;
		       
			pix = src.GetPixel((x+1) % width, (y+1) % height);
			dY += pix.red * -1.0f;
		       
			pix = src.GetPixel((x-1+width) % width, (y-1+height) % height);
			dY += pix.red * 1.0f;
		       
			pix = src.GetPixel(x % width, (y-1+height) % height);
			dY += pix.red *  2.0f;
		       
			pix = src.GetPixel((x+1) % width, (y-1+height) % height);
			dY += pix.red *  1.0f;
		       
			// Do X Sobel filter
			pix = src.GetPixel((x-1+width) % width, (y-1+height) % height);
			dX  = pix.red * -1.0f;
		       
			pix = src.GetPixel((x-1+width) % width,   y   % height);
			dX += pix.red * -2.0f;
		       
			pix = src.GetPixel((x-1+width) % width, (y+1) % height);
			dX += pix.red * -1.0f;
		       
			pix = src.GetPixel((x+1) % width, (y-1+height) % height);
			dX += pix.red * 1.0f;
		       
			pix = src.GetPixel((x+1) % width,   y   % height);
			dX += pix.red * 2.0f;
		       
			pix = src.GetPixel((x+1) % width, (y+1) % height);
			dX += pix.red * 1.0f;
		       
			// Cross Product of components of gradient reduces to
			nX = -dX;
			nY = -dY;
			nZ = 1;
		       
			// Normalize
			F32 oolen = 1.0f/((F32) sqrt(nX*nX + nY*nY + nZ*nZ));
			nX *= oolen;
			nY *= oolen;
			nZ *= oolen;
		       
			pix.red   = nX;
			pix.green = nY;
			pix.blue  = nZ;
			pix.alpha = 1.f;
		       
			dest.SetPixel(pix, x, y);
		}
    }

	return Attk::E_Status_Success;
}


