#include "stdafx.h"

#include "texture_export/Texture.h"

using namespace ActTools;

namespace Texutils
{

U8 PackFloatInByte(float in)
{
   return (U8) ((in+1.0f) / 2.0f * 255.0f);
}

// Working pixel format.
struct Pixel_A8R8G8B8
{
	U8 blue;
	U8 green;
	U8 red;
	U8 alpha;

	Pixel_A8R8G8B8() {}
	Pixel_A8R8G8B8(U8 a, U8 r, U8 g, U8 b) {alpha = a; red = r; green = g; blue = b;}
};

class Image
{
public:
	Image(int iWidth, int iHeight, U32* pData)
	{
		m_iWidth = iWidth;
		m_iHeight = iHeight;
		m_pData = pData;
	}

	void ReadPixel(Pixel_A8R8G8B8 *pPixel, int x, int y)
	{
		*(U32*)pPixel = m_pData[x + y * m_iWidth];
	}

	void WritePixel(Pixel_A8R8G8B8 *pPixel, int x, int y)
	{
		m_pData[x + y * m_iWidth] = *(U32*)pPixel;
	}

	U32 *m_pData;
	int m_iWidth, m_iHeight;
};

void GenerateNormalMap(U32* data, int width, int height)
{
	Image source(width, height, data);
	U32 *pOutput = new U32[width*height];
	Image destination(width, height, pOutput);

	Pixel_A8R8G8B8 pix;
	float dX, dY;
	float nX, nY, nZ;

	for (int y = 0; y < height; y++)
    {
		for (int x = 0; x < width; x++)
		{
			// Do Y Sobel filter
			source.ReadPixel(&pix, (x-1+width) % width, (y+1) % height);
			dY  = ((float) pix.red) / 255.0f * -1.0f;
		       
			source.ReadPixel(&pix,   x   % width, (y+1) % height);
			dY += ((float) pix.red) / 255.0f * -2.0f;
		       
			source.ReadPixel(&pix, (x+1) % width, (y+1) % height);
			dY += ((float) pix.red) / 255.0f * -1.0f;
		       
			source.ReadPixel(&pix, (x-1+width) % width, (y-1+height) % height);
			dY += ((float) pix.red) / 255.0f *  1.0f;
		       
			source.ReadPixel(&pix,   x   % width, (y-1+height) % height);
			dY += ((float) pix.red) / 255.0f *  2.0f;
		       
			source.ReadPixel(&pix, (x+1) % width, (y-1+height) % height);
			dY += ((float) pix.red) / 255.0f *  1.0f;
		       
			// Do X Sobel filter
			source.ReadPixel(&pix, (x-1+width) % width, (y-1+height) % height);
			dX  = ((float) pix.red) / 255.0f * -1.0f;
		       
			source.ReadPixel(&pix, (x-1+width) % width,   y   % height);
			dX += ((float) pix.red) / 255.0f * -2.0f;
		       
			source.ReadPixel(&pix, (x-1+width) % width, (y+1) % height);
			dX += ((float) pix.red) / 255.0f * -1.0f;
		       
			source.ReadPixel(&pix, (x+1) % width, (y-1+height) % height);
			dX += ((float) pix.red) / 255.0f *  1.0f;
		       
			source.ReadPixel(&pix, (x+1) % width,   y   % height);
			dX += ((float) pix.red) / 255.0f *  2.0f;
		       
			source.ReadPixel(&pix, (x+1) % width, (y+1) % height);
			dX += ((float) pix.red) / 255.0f *  1.0f;
		       
			// Cross Product of components of gradient reduces to
			nX = -dX;
			nY = -dY;
			nZ = 1;
		       
			// Normalize
			float oolen = 1.0f/((float) sqrt(nX*nX + nY*nY + nZ*nZ));
			nX *= oolen;
			nY *= oolen;
			nZ *= oolen;
		       
			pix.red   = (U8) PackFloatInByte(nX);
			pix.green = (U8) PackFloatInByte(nY);
			pix.blue  = (U8) PackFloatInByte(nZ);
		       
			destination.WritePixel(&pix, x, y);
		}
    }

	memcpy(data, pOutput, width*height*sizeof(U32));

	delete [] pOutput;
}

/// convert the given texture from a heightmap to a normal map
ACTRESULT HeightMapToNormalMap( ActTools::Texture* aTexture )
{
	// convert to A8R8G8B8
	ActTools::A8R8G8B8 a8r8g8b8;
	aTexture->ConvertTo(&a8r8g8b8);

	// convert to normalmap
	for (U32 i=0; i<aTexture->GetDepth() * aTexture->GetNumMipMaps(); i++)
	{
		GenerateNormalMap((U32*)aTexture->GetSurface(i).GetTexels(), aTexture->GetWidth(), aTexture->GetHeight());
	}

	return ACT_OK;
}

/// convert the given texture from a heightmap to a normal map
ACTRESULT InvertNormalMapX( ActTools::Texture* aTexture )
{
	// convert to A8R8G8B8
	ActTools::A8R8G8B8 a8r8g8b8;
	aTexture->ConvertTo(&a8r8g8b8);

	// invert red channel
	for (U32 i=0; i<aTexture->GetDepth() * aTexture->GetNumMipMaps(); i++)
	{
		U32 *pTexels = (U32*)aTexture->GetSurface(i).GetTexels();
		int iWidth = aTexture->GetWidth();
		int iHeight = aTexture->GetHeight();
		Image source(iWidth, iHeight, pTexels);

		for (int x=0; x<iWidth * iHeight; x++)
		{
			Pixel_A8R8G8B8 pix;

			source.ReadPixel(&pix, x, 0);
			pix.red = 255 - pix.red;
			source.WritePixel(&pix, x, 0);
		}
	}

	return ACT_OK;
}
/// premultiply alpha on the given texture
ACTRESULT PreMultiplyAlpha( ActTools::Texture* aTexture )
{
	// convert to A8R8G8B8
	ActTools::A8R8G8B8 a8r8g8b8;
	aTexture->ConvertTo(&a8r8g8b8);

	// premultiply alpha
	for (U32 i=0; i<aTexture->GetDepth() * aTexture->GetNumMipMaps(); i++)
	{
		U32 *pTexels = (U32*)aTexture->GetSurface(i).GetTexels();
		int iWidth = aTexture->GetSurface(i).GetWidth();
		int iHeight = aTexture->GetSurface(i).GetHeight();
		Image source(iWidth, iHeight, pTexels);

		for (int x=0; x<iWidth * iHeight; x++)
		{
			Pixel_A8R8G8B8 pix;

			source.ReadPixel(&pix, x, 0);
			float r=(float)pix.red/255.f;
			float g=(float)pix.green/255.f;
			float b=(float)pix.blue/255.f;
			float a=(float)pix.alpha/255.f;
			r*=a; g*=a; b*=a;
			pix.red   = (U8)(r*255.f);
			pix.green = (U8)(g*255.f);
			pix.blue  = (U8)(b*255.f);
			source.WritePixel(&pix, x, 0);
		}
	}

	return ACT_OK;
}


}
