#include "ImposterPch.h"

#include "Imposter/ImposterImageHandle.h"
#include "Imposter/ImposterData.h"
#include "Imposter/ImposterUtils.h"

namespace Imposter
{
template <E_ImposterType type>
struct ViewCount
{
	static const U32 count;
};

template <E_ImposterType type>
struct Views
{
	static const ViewData::ViewVector viewVectors[ViewCount<type>::count];
};


#define DECLARE_VIEWS(type, viewcount, ...)								\
	const U32 ViewCount<type>::count = viewcount;						\
	const ViewData::ViewVector Views<type>::viewVectors[viewcount] = {__VA_ARGS__};	

DECLARE_VIEWS(Imposter::E_ScreenAligned, 1, {0.f, 0.f, 1.f})
DECLARE_VIEWS(Imposter::E_Static1_Mirrored, 1, {0.f, 0.f, 1.f})
DECLARE_VIEWS(Imposter::E_Static2_Mirrored, 2, {0.f, 0.f, 1.f}, {1.f, 0.f, 0.f})
DECLARE_VIEWS(Imposter::E_Static1, 2, {0.f, 0.f, 1.f}, {0.f, 0.f, -1.f})
DECLARE_VIEWS(Imposter::E_Static2, 4, {0.f, 0.f, 1.f}, {0.f, 0.f, -1.f}, {1.f, 0.f, 0.f}, {-1.f, 0.f, 0.f})

#undef DECLARE_VIEWS

E_ImposterType GetImposterType(const char* param)
{
	for (S32 i=0; i<E_ImposterType_Count; ++i)
		if (strcmp(kImposterTypeParams[i], param) == 0)
			return (E_ImposterType) i;

	return E_ImposterType_Count;
}


ViewData GetViewData(const Imposter::E_ImposterType type)
{
	switch (type)
	{
	case Imposter::E_ScreenAligned:
	{
		ViewData v = {false, ViewCount<Imposter::E_ScreenAligned>::count, Views<Imposter::E_ScreenAligned>::viewVectors};
		return v;
	}
	case Imposter::E_Static1_Mirrored:
	{
		ViewData v = {true, ViewCount<Imposter::E_Static1_Mirrored>::count, Views<Imposter::E_Static1_Mirrored>::viewVectors};
		return v;
	}
	case Imposter::E_Static2_Mirrored:
	{
		ViewData v = {true, ViewCount<Imposter::E_Static2_Mirrored>::count, Views<Imposter::E_Static2_Mirrored>::viewVectors};
		return v;
	}
	case Imposter::E_Static1:
	{
		ViewData v = {false, ViewCount<Imposter::E_Static1>::count, Views<Imposter::E_Static1>::viewVectors};
		return v;
	}
	case Imposter::E_Static2:
	{
		ViewData v = {false, ViewCount<Imposter::E_Static2>::count, Views<Imposter::E_Static2>::viewVectors};
		return v;
	}
	};

	ViewData v = {false, 0, 0};
	return v;
}


template <typename T>
RECT DoGetBoundingRect(const T* pixelData, U32 width, U32 height, T& refPixel)
{
	RECT r = {-1, -1, -1, -1};
	
	//top
	for (U32 y=0; y<height; ++y)
	{
		bool emptyLine = true;
		const T* pPixel = &pixelData[y*width];
		for (U32 x=0; x<width; ++x)
		{
			if (memcmp(pPixel++, &refPixel, sizeof(T)) != 0)
			{
				emptyLine = false;
				break;
			}
		}

		if (!emptyLine)
		{
			r.top = y;
			break;
		}
	}

	if (r.top == -1)
		return r;

	//bottom
	for (U32 y=height-1; y>(U32)r.top; --y)
	{
		bool emptyLine = true;
		const T* pPixel = &pixelData[y*width];
		for (U32 x=0; x<width; ++x)
		{
			if (memcmp(pPixel++, &refPixel, sizeof(T)) != 0)
			{
				emptyLine = false;
				break;
			}
		}

		r.bottom = y;
		if (!emptyLine)
			break;
	}

	//left
	for (U32 x=0; x<width; ++x)
	{
		bool emptyLine = true;
		for (U32 y=(U32)r.top; y<=(U32)r.bottom; ++y)
		{
			if (memcmp(&pixelData[y*width+x], &refPixel, sizeof(T)) != 0)
			{
				emptyLine = false;
				break;
			}
		}

		r.left = x;
		if (!emptyLine)
			break;
	}

	//right
	for (U32 x=width-1; x>(U32)r.left; --x)
	{
		bool emptyLine = true;
		for (U32 y=(U32)r.top; y<=(U32)r.bottom; ++y)
		{
			if (memcmp(&pixelData[y*width+x], &refPixel, sizeof(T)) != 0)
			{
				emptyLine = false;
				break;
			}
		}

		r.right = x;
		if (!emptyLine)
			break;
	}

	return r;
}


RECT GetBoundingRect(const Attk::C_Image& image)
{
	RECT r;

	switch(image.GetPixelFormat())
	{
	case Attk::E_PixelFormat_R8G8B8A8U:
	{
		Attk::C_PixelR8G8B8A8U refPixel; refPixel.data = 0;
		r = DoGetBoundingRect((Attk::C_PixelR8G8B8A8U*)image.GetPixelDataPtr(0,0), image.GetWidth(), image.GetHeight(), refPixel);
		break;
	}
	case Attk::E_PixelFormat_A8R8G8B8U:
	{
		Attk::C_PixelA8R8G8B8U refPixel; refPixel.data = 0;
		r = DoGetBoundingRect((Attk::C_PixelA8R8G8B8U*)image.GetPixelDataPtr(0,0), image.GetWidth(), image.GetHeight(), refPixel);
		break;
	}
	case Attk::E_PixelFormat_B8G8R8A8U:
	{
		Attk::C_PixelB8G8R8A8U refPixel; refPixel.data = 0;
		r = DoGetBoundingRect((Attk::C_PixelB8G8R8A8U*)image.GetPixelDataPtr(0,0), image.GetWidth(), image.GetHeight(), refPixel);
		break;
	}
	//other pixel formats here if needed...
	}

	return r;
}


bool CropImage(Attk::C_Image& image, const RECT r)
{
	if (r.bottom > r.top && r.top >= 0 &&
		r.right > r.left && r.left >= 0 &&
		(U32)r.bottom < image.GetHeight() &&
		(U32)r.right < image.GetWidth())
	{
		U32 w=r.right-r.left+1, h=r.bottom-r.top+1;

		U8* pPixelData = new U8[image.GetBytesPerPixel()*w*h];
		if (pPixelData)
		{
			U8* pCurrent = pPixelData;
			for (U32 i=0; i<h; ++i)
			{
				memcpy(pCurrent, image.GetPixelDataPtr(r.left, r.top+i), image.GetBytesPerPixel()*w);
				pCurrent += image.GetBytesPerPixel()*w;
			}

			image.NewPixelData(w, h, 1, image.GetPixelFormat(), pPixelData);
			delete[] pPixelData;
			return true;
		}
	}

	return false;
}


Aftk::C_Bounds GetWorldBounds(const Aftk::C_SceneNode *pSceneNode)
{
	Aftk::C_Bounds bounds = pSceneNode->GetWorldBounds();

	//expand if the node contains screen aligned polygons
	if (pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
	{
		const Aftk::C_SceneMeshNode *pAftkMeshNode = static_cast<const Aftk::C_SceneMeshNode*>(pSceneNode);

		if (pAftkMeshNode->GetNumPolygonMaterialMappings())
		{
			Ut::BitVector isBillboard;
			bool hasBillboardPolys = false;

			isBillboard.Resize(pAftkMeshNode->GetNumPolygonMaterialMappings());

			for (S16 i=0; i<pAftkMeshNode->GetNumPolygonMaterialMappings(); ++i)
			{
				const Aftk::C_MaterialResource *pResource = pAftkMeshNode->GetPolygonMaterialMappingPtr(i);
				if (pResource)
				{
					const Aftk::C_MaterialNode *pMaterial = &pResource->GetContent();
					if (pMaterial->GetMaterialTypeId() == AftkRender::CGFX_SPEEDTREE_LEAF)
					{
						isBillboard[i] = true;
						hasBillboardPolys |= true;
					}
					else
					{
						bool speedtreeleaf = false;
						for (U32 j=0; j<pMaterial->GetNumShaderNodes(); ++j)
						{
							const Aftk::C_ShaderNode *pShader = pMaterial->GetShaderNodePtr(j);
							std::string shader;
							const Aftk::I_Plug *pPlug = pShader->GetPlugPtr("shader");
							if (pPlug && pPlug->GetValue(shader) && shader == "SpeedTreeLeaf.fx")
								speedtreeleaf = true;
						}
						isBillboard[i] = speedtreeleaf;
						hasBillboardPolys |= speedtreeleaf;
					}
				}
				else
					isBillboard[i] = false;
			}

			if (hasBillboardPolys)
			{
				const Aftk::PolygonArray& polygonArray = pAftkMeshNode->GetPolygonArray(0);
				for (S32 i=0; i<polygonArray.Size(); ++i)
				{
					const Aftk::C_Polygon& polygon = polygonArray[i];
					if (polygon.GetBasePolygonIndex() == -1 &&
						polygon.GetMaterialMappingIndex() != -1 &&
						isBillboard[polygon.GetMaterialMappingIndex()])
					{
						//assume the 2nd uv set is the pivot offset.
						S32 uvIdx = polygon.GetPolygonVertex(0).GetTextureCoordinateIndex(1);
						S32 pIdx = polygon.GetPolygonVertex(0).GetPositionIndex();

						if (uvIdx!=-1 && pIdx!=-1)
						{
							const Aftk::Position3& p = pAftkMeshNode->GetVertexPosition(pIdx);
							const Aftk::TextureCoordinate4& uv = pAftkMeshNode->GetTextureCoordinate(1, uvIdx);

							Aftk::Position3 c(p[0]-uv[0], p[1]-uv[1], p[2]);
							c = Aftk::ApplyMatrix(pSceneNode->GetWorldMatrix(), c);
							F32 r = Maths::Sqrt((p[0]-uv[0])*(p[0]-uv[0]) + (p[1]-uv[1])*(p[1]-uv[1]));
							
							bounds.AddVertex(Aftk::Position3(c[0]+r, c[1], c[2]));
							bounds.AddVertex(Aftk::Position3(c[0]-r, c[1], c[2]));
							bounds.AddVertex(Aftk::Position3(c[0], c[1]+r, c[2]));
							bounds.AddVertex(Aftk::Position3(c[0], c[1]-r, c[2]));
							bounds.AddVertex(Aftk::Position3(c[0], c[1], c[2]+r));
							bounds.AddVertex(Aftk::Position3(c[0], c[1], c[2]-r));
						}
					}
				}
			}
		}
	}

	return bounds;
}

void UnsharpMask(Attk::C_Image& image, F32 threshold, F32 amount)
{
	if (image.GetPixelFormat() != Attk::E_PixelFormat_B8G8R8A8U)
		return;

	Attk::C_Image tmp(image);
	U32 w = image.GetWidth(); U32 h = image.GetHeight();

	//we really need a gaussian blur...
	AttkUt::C_ImageResize::Resize(tmp, Maths::Max<U32>(w/2, 1), Maths::Max<U32>(h/2, 1), 1);
	AttkUt::C_ImageResize::Resize(tmp, w, h, 1);

	U32 nPixels = w*h;
	Attk::C_PixelB8G8R8A8U *pSrc = (Attk::C_PixelB8G8R8A8U*) image.GetPixelDataPtr();
	Attk::C_PixelB8G8R8A8U *pRef = (Attk::C_PixelB8G8R8A8U*) tmp.GetPixelDataPtr();

	for (U32 i=0; i<nPixels; ++i)
	{
		const S32 a0=pSrc->alpha, r0=pSrc->red, g0=pSrc->green, b0=pSrc->blue;
		const S32 a1=pRef->alpha, r1=pRef->red, g1=pRef->green, b1=pRef->blue;

		U32 a = (Maths::Abs<F32>(a0-a1) > threshold) ? (U32) Maths::Clamp<F32>(amount*(a0-a1)+a0, 0.f, 255.f) : a0;
		U32 r = (Maths::Abs<F32>(r0-r1) > threshold) ? (U32) Maths::Clamp<F32>(amount*(r0-r1)+r0, 0.f, 255.f) : r0;
		U32 g = (Maths::Abs<F32>(g0-g1) > threshold) ? (U32) Maths::Clamp<F32>(amount*(g0-g1)+g0, 0.f, 255.f) : g0;
		U32 b = (Maths::Abs<F32>(b0-b1) > threshold) ? (U32) Maths::Clamp<F32>(amount*(b0-b1)+b0, 0.f, 255.f) : b0;

		//pSrc->alpha	= (U8)a;
		pSrc->red	= (U8)r;
		pSrc->green	= (U8)g;
		pSrc->blue	= (U8)b;

		pSrc++;
		pRef++;
	}
}

void RampAlpha(Attk::C_Image& image)
{
	if (image.GetPixelFormat() != Attk::E_PixelFormat_B8G8R8A8U)
		return;

	U32 w = image.GetWidth(); U32 h = image.GetHeight();
	U32 nPixels = w*h;


	//contrast stretching histogram equalisation
	// - ignore 0 & 255 alpha contribution.
	// - ignore upper 50% of remaining pixels - this has the effect of equalising the remaining 50% (the upper 50% will be at 255)

	S32 histogram[256], cumulative[256];
	U8 min=0, max=255;
	U32 pixCount=0;
	{
		memset(histogram, 0, sizeof(U32)*lengthof(histogram));
		Attk::C_PixelB8G8R8A8U *pSrc = (Attk::C_PixelB8G8R8A8U*) image.GetPixelDataPtr();
		for (U32 i=0; i<nPixels; ++i)
		{
			histogram[pSrc->alpha]++;
			pSrc++;
		}

		U32 nContributingPixels = nPixels-histogram[0]-histogram[255];
		S32 low  = 0;
		S32 high = (U32)(((F32)nContributingPixels)*0.5f);

		S32 s = 0;
		for (U32 i=1; i<256; ++i)
		{
			s += histogram[i];
			if (s<low)
				min = i;
			if (s<high)
				max = i;
		}
		pixCount = high-low;

		s = 0;
		for (U32 i=0; i<256; ++i)
		{
			s += histogram[i];
			cumulative[i] = Maths::Clamp<S32>(s-histogram[0]-low, 0, pixCount);
		}

	}

	if (min == max)
		return;


	//now generate the lookup table
	U8 lut[256];
	{
		lut[0] = 0;
		for (U32 i=1; i<256; ++i)
		{
			F32 v = ((F32)cumulative[i]-(F32)cumulative[min])/(pixCount-cumulative[min])*255;
			//additional scaling factor
			lut[i] = (U8)Maths::Clamp(v*1.2f, 0.f, 255.f);
		}
	}

	Attk::C_PixelB8G8R8A8U *pSrc = (Attk::C_PixelB8G8R8A8U*) image.GetPixelDataPtr();
	for (U32 i=0; i<nPixels; ++i)
	{
		pSrc->alpha	= lut[pSrc->alpha];
		pSrc++;
	}
}

void PostProcessImage(Attk::C_Image& image)
{
	if (image.GetPixelFormat() != Attk::E_PixelFormat_B8G8R8A8U)
		return;

	UnsharpMask(image, 0.f, 0.25f);
	RampAlpha(image);
}

}