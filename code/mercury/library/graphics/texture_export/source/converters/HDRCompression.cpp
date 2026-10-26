#include "stdafx.h"

#include "texture_export/Texture.h"

using namespace ActTools;

namespace Texutils
{

/// create the luminance texture
ActTools::Texture* HDRMakeLuminance(ActTools::Texture* aTexture)
{
	ActTools::L16 l16;

	ActTools::Texture* pTextureHDR = new ActTools::Texture();
	pTextureHDR->CreateTexture(&l16, aTexture->GetWidth() / 4, aTexture->GetHeight() / 4, aTexture->GetNumMipMaps());

	ActTools::Surface *pSurfaceHDR = pTextureHDR->GetMipmap(0, 0);
	pSurfaceHDR->Create(aTexture->GetWidth() / 4, aTexture->GetHeight() / 4);

	// normalise the colour
	MathsStorage::Float4*	pInData		= (MathsStorage::Float4*)aTexture->GetMipmap(0, 0)->GetTexels();
	U16*					pHDRData	= (U16*)pSurfaceHDR->GetTexels();
	F32*					pTempData	= new F32[aTexture->GetWidth() * aTexture->GetHeight() / 16];
	for (U32 y=0; y<aTexture->GetHeight(); y+=4)
	{
		for (U32 x=0; x<aTexture->GetWidth(); x+=4)
		{
			const MathsStorage::Float4	vInput0			= pInData[y * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput1			= pInData[y * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput2			= pInData[y * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput3			= pInData[y * aTexture->GetWidth() + x+3];
			const MathsStorage::Float4	vInput4			= pInData[(y+1) * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput5			= pInData[(y+1) * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput6			= pInData[(y+1) * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput7			= pInData[(y+1) * aTexture->GetWidth() + x+3];
			const MathsStorage::Float4	vInput8			= pInData[(y+2) * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput9			= pInData[(y+2) * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput10		= pInData[(y+2) * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput11		= pInData[(y+2) * aTexture->GetWidth() + x+3];
			const MathsStorage::Float4	vInput12		= pInData[(y+3) * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput13		= pInData[(y+3) * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput14		= pInData[(y+3) * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput15		= pInData[(y+3) * aTexture->GetWidth() + x+3];

			Maths::HVec3	vRGB0	= Maths::HVec3(vInput0.y, vInput0.z, vInput0.w);
			Maths::HVec3	vRGB1	= Maths::HVec3(vInput1.y, vInput1.z, vInput1.w);
			Maths::HVec3	vRGB2	= Maths::HVec3(vInput2.y, vInput2.z, vInput2.w);				
			Maths::HVec3	vRGB3	= Maths::HVec3(vInput3.y, vInput3.z, vInput3.w);
			Maths::HVec3	vRGB4	= Maths::HVec3(vInput4.y, vInput4.z, vInput4.w);
			Maths::HVec3	vRGB5	= Maths::HVec3(vInput5.y, vInput5.z, vInput5.w);
			Maths::HVec3	vRGB6	= Maths::HVec3(vInput6.y, vInput6.z, vInput6.w);				
			Maths::HVec3	vRGB7	= Maths::HVec3(vInput7.y, vInput7.z, vInput7.w);
			Maths::HVec3	vRGB8	= Maths::HVec3(vInput8.y, vInput8.z, vInput8.w);
			Maths::HVec3	vRGB9	= Maths::HVec3(vInput9.y, vInput9.z, vInput9.w);
			Maths::HVec3	vRGB10	= Maths::HVec3(vInput10.y, vInput10.z, vInput10.w);				
			Maths::HVec3	vRGB11	= Maths::HVec3(vInput11.y, vInput11.z, vInput11.w);
			Maths::HVec3	vRGB12	= Maths::HVec3(vInput12.y, vInput12.z, vInput12.w);
			Maths::HVec3	vRGB13	= Maths::HVec3(vInput13.y, vInput13.z, vInput13.w);
			Maths::HVec3	vRGB14	= Maths::HVec3(vInput14.y, vInput14.z, vInput14.w);				
			Maths::HVec3	vRGB15	= Maths::HVec3(vInput15.y, vInput15.z, vInput15.w);

			const Maths::HF32	vLength0	= vRGB0.Length();
			const Maths::HF32	vLength1	= vRGB1.Length();
			const Maths::HF32	vLength2	= vRGB2.Length();
			const Maths::HF32	vLength3	= vRGB3.Length();
			const Maths::HF32	vLength4	= vRGB4.Length();
			const Maths::HF32	vLength5	= vRGB5.Length();
			const Maths::HF32	vLength6	= vRGB6.Length();
			const Maths::HF32	vLength7	= vRGB7.Length();
			const Maths::HF32	vLength8	= vRGB8.Length();
			const Maths::HF32	vLength9	= vRGB9.Length();
			const Maths::HF32	vLength10	= vRGB10.Length();
			const Maths::HF32	vLength11	= vRGB11.Length();
			const Maths::HF32	vLength12	= vRGB12.Length();
			const Maths::HF32	vLength13	= vRGB13.Length();
			const Maths::HF32	vLength14	= vRGB14.Length();
			const Maths::HF32	vLength15	= vRGB15.Length();
			
			const Maths::HF32	vCombine0	= (vLength0 + vLength1 + vLength2 + vLength3) / 4.0f;
			const Maths::HF32	vCombine1	= (vLength4 + vLength5 + vLength6 + vLength7) / 4.0f;
			const Maths::HF32	vCombine2	= (vLength8 + vLength9 + vLength10 + vLength11) / 4.0f;
			const Maths::HF32	vCombine3	= (vLength12 + vLength13 + vLength14 + vLength15) / 4.0f;
			const Maths::HF32	vCombine	= (vCombine0 + vCombine1 + vCombine2 + vCombine3) / 4.0f;

			pTempData[(y/4) * (aTexture->GetWidth()/4) + (x/4)] = vCombine;
		}
	}

	// gaussian blur it
	S32 width = aTexture->GetWidth()/4 - 1;
	S32 height = aTexture->GetHeight()/4 - 1;
	for (S32 y=0; y<=height; ++y)
	{
		for (S32 x=0; x<=width; ++x)
		{
			S32 xLeft = (x == 0) ? 0 : x - 1;
			S32 xRight = (x == width) ? width : x + 1;
			S32 yUp = (y == 0) ? 0 : y - 1;
			S32 yDown = (y == height) ? height : y + 1;

			F32 f0 = pTempData[y * aTexture->GetWidth() / 4 + x];
			F32 f1 = pTempData[y * aTexture->GetWidth() / 4 + xLeft];
			F32 f2 = pTempData[y * aTexture->GetWidth() / 4 + xRight];
			F32 f3 = pTempData[yUp * aTexture->GetWidth() / 4 + x];
			F32 f4 = pTempData[yDown * aTexture->GetWidth() / 4 + x];
			F32 f = (f0 + f0 + f1 + f2 + f3 + f4) / 6.0f;

			pHDRData[y * (aTexture->GetWidth()/4) + x] = (U16)(Maths::Clamp01(f / 8.0f) * 65535.0f);
		}
	}

	delete[] pTempData;

	return pTextureHDR;
}

/// compress an HDR texture into 5 bits per pixel and blurred
ACTRESULT HDRCompress5BPPBlur( ActTools::Texture* aTexture, ActTools::Texture*& pTextureLDR, ActTools::Texture*& pTextureHDR )
{
	ActTools::A8R8G8B8 a8r8g8b8;
	ActTools::L16 l16;

	pTextureLDR = new ActTools::Texture();
	pTextureLDR->CreateTexture(&a8r8g8b8, aTexture->GetWidth(), aTexture->GetHeight(), aTexture->GetNumMipMaps());

	ActTools::Surface *pSurfaceLDR = pTextureLDR->GetMipmap(0, 0);
	pSurfaceLDR->Create(aTexture->GetWidth(), aTexture->GetHeight());

	pTextureHDR = HDRMakeLuminance(aTexture);

	// normalise the colour
	MathsStorage::Float4*	pInData		= (MathsStorage::Float4*)aTexture->GetMipmap(0, 0)->GetTexels();
	U32*					pLDRData	= (U32*)pSurfaceLDR->GetTexels();
	U16*					pHDRData	= (U16*)pTextureHDR->GetMipmap(0, 0)->GetTexels();
	for (U32 y=0; y<aTexture->GetHeight(); y+=4)
	{
		for (U32 x=0; x<aTexture->GetWidth(); x+=4)
		{
			const MathsStorage::Float4	vInput0			= pInData[y * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput1			= pInData[y * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput2			= pInData[y * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput3			= pInData[y * aTexture->GetWidth() + x+3];
			const MathsStorage::Float4	vInput4			= pInData[(y+1) * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput5			= pInData[(y+1) * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput6			= pInData[(y+1) * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput7			= pInData[(y+1) * aTexture->GetWidth() + x+3];
			const MathsStorage::Float4	vInput8			= pInData[(y+2) * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput9			= pInData[(y+2) * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput10		= pInData[(y+2) * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput11		= pInData[(y+2) * aTexture->GetWidth() + x+3];
			const MathsStorage::Float4	vInput12		= pInData[(y+3) * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput13		= pInData[(y+3) * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput14		= pInData[(y+3) * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput15		= pInData[(y+3) * aTexture->GetWidth() + x+3];

			Maths::HVec3	vRGB0	= Maths::HVec3(vInput0.y, vInput0.z, vInput0.w);
			Maths::HVec3	vRGB1	= Maths::HVec3(vInput1.y, vInput1.z, vInput1.w);
			Maths::HVec3	vRGB2	= Maths::HVec3(vInput2.y, vInput2.z, vInput2.w);				
			Maths::HVec3	vRGB3	= Maths::HVec3(vInput3.y, vInput3.z, vInput3.w);
			Maths::HVec3	vRGB4	= Maths::HVec3(vInput4.y, vInput4.z, vInput4.w);
			Maths::HVec3	vRGB5	= Maths::HVec3(vInput5.y, vInput5.z, vInput5.w);
			Maths::HVec3	vRGB6	= Maths::HVec3(vInput6.y, vInput6.z, vInput6.w);				
			Maths::HVec3	vRGB7	= Maths::HVec3(vInput7.y, vInput7.z, vInput7.w);
			Maths::HVec3	vRGB8	= Maths::HVec3(vInput8.y, vInput8.z, vInput8.w);
			Maths::HVec3	vRGB9	= Maths::HVec3(vInput9.y, vInput9.z, vInput9.w);
			Maths::HVec3	vRGB10	= Maths::HVec3(vInput10.y, vInput10.z, vInput10.w);				
			Maths::HVec3	vRGB11	= Maths::HVec3(vInput11.y, vInput11.z, vInput11.w);
			Maths::HVec3	vRGB12	= Maths::HVec3(vInput12.y, vInput12.z, vInput12.w);
			Maths::HVec3	vRGB13	= Maths::HVec3(vInput13.y, vInput13.z, vInput13.w);
			Maths::HVec3	vRGB14	= Maths::HVec3(vInput14.y, vInput14.z, vInput14.w);				
			Maths::HVec3	vRGB15	= Maths::HVec3(vInput15.y, vInput15.z, vInput15.w);

			const Maths::HF32	vLength0	= vRGB0.Length();
			const Maths::HF32	vLength1	= vRGB1.Length();
			const Maths::HF32	vLength2	= vRGB2.Length();
			const Maths::HF32	vLength3	= vRGB3.Length();
			const Maths::HF32	vLength4	= vRGB4.Length();
			const Maths::HF32	vLength5	= vRGB5.Length();
			const Maths::HF32	vLength6	= vRGB6.Length();
			const Maths::HF32	vLength7	= vRGB7.Length();
			const Maths::HF32	vLength8	= vRGB8.Length();
			const Maths::HF32	vLength9	= vRGB9.Length();
			const Maths::HF32	vLength10	= vRGB10.Length();
			const Maths::HF32	vLength11	= vRGB11.Length();
			const Maths::HF32	vLength12	= vRGB12.Length();
			const Maths::HF32	vLength13	= vRGB13.Length();
			const Maths::HF32	vLength14	= vRGB14.Length();
			const Maths::HF32	vLength15	= vRGB15.Length();
			
			const Maths::HF32	vCombine0	= (vLength0 + vLength1 + vLength2 + vLength3) / 4.0f;
			const Maths::HF32	vCombine1	= (vLength4 + vLength5 + vLength6 + vLength7) / 4.0f;
			const Maths::HF32	vCombine2	= (vLength8 + vLength9 + vLength10 + vLength11) / 4.0f;
			const Maths::HF32	vCombine3	= (vLength12 + vLength13 + vLength14 + vLength15) / 4.0f;

			const Maths::HF32	vCombine	= pHDRData[(y/4) * (aTexture->GetWidth()/4) + (x/4)] * (8.0f / 65535.0f);

			vRGB0 = vRGB0 * vCombine.Rcp();
			vRGB1 = vRGB1 * vCombine.Rcp();
			vRGB2 = vRGB2 * vCombine.Rcp();
			vRGB3 = vRGB3 * vCombine.Rcp();
			vRGB4 = vRGB4 * vCombine.Rcp();
			vRGB5 = vRGB5 * vCombine.Rcp();
			vRGB6 = vRGB6 * vCombine.Rcp();
			vRGB7 = vRGB7 * vCombine.Rcp();
			vRGB8 = vRGB8 * vCombine.Rcp();
			vRGB9 = vRGB9 * vCombine.Rcp();
			vRGB10 = vRGB10 * vCombine.Rcp();
			vRGB11 = vRGB11 * vCombine.Rcp();
			vRGB12 = vRGB12 * vCombine.Rcp();
			vRGB13 = vRGB13 * vCombine.Rcp();
			vRGB14 = vRGB14 * vCombine.Rcp();
			vRGB15 = vRGB15 * vCombine.Rcp();

			pLDRData[y * aTexture->GetWidth() + x]		= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB0)).Saturate());
			pLDRData[y * aTexture->GetWidth() + x+1]		= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB1)).Saturate());
			pLDRData[y * aTexture->GetWidth() + x+2]		= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB2)).Saturate());
			pLDRData[y * aTexture->GetWidth() + x+3]		= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB3)).Saturate());
			pLDRData[(y+1) * aTexture->GetWidth() + x]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB4)).Saturate());
			pLDRData[(y+1) * aTexture->GetWidth() + x+1]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB5)).Saturate());
			pLDRData[(y+1) * aTexture->GetWidth() + x+2]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB6)).Saturate());
			pLDRData[(y+1) * aTexture->GetWidth() + x+3]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB7)).Saturate());
			pLDRData[(y+2) * aTexture->GetWidth() + x]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB8)).Saturate());
			pLDRData[(y+2) * aTexture->GetWidth() + x+1]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB9)).Saturate());
			pLDRData[(y+2) * aTexture->GetWidth() + x+2]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB10)).Saturate());
			pLDRData[(y+2) * aTexture->GetWidth() + x+3]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB11)).Saturate());
			pLDRData[(y+3) * aTexture->GetWidth() + x]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB12)).Saturate());
			pLDRData[(y+3) * aTexture->GetWidth() + x+1]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB13)).Saturate());
			pLDRData[(y+3) * aTexture->GetWidth() + x+2]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB14)).Saturate());
			pLDRData[(y+3) * aTexture->GetWidth() + x+3]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB15)).Saturate());
		}
	}

	return ACT_OK;
}

/// compress an HDR texture into 5 bits per pixel
ACTRESULT HDRCompress5BPP( ActTools::Texture* aTexture, ActTools::Texture*& pTextureLDR, ActTools::Texture*& pTextureHDR )
{
	ActTools::A8R8G8B8 a8r8g8b8;
	ActTools::L16 l16;

	pTextureLDR = new ActTools::Texture();
	pTextureHDR = new ActTools::Texture();

	pTextureLDR->CreateTexture(&a8r8g8b8, aTexture->GetWidth(), aTexture->GetHeight(), aTexture->GetNumMipMaps());
	pTextureHDR->CreateTexture(&l16, aTexture->GetWidth() / 4, aTexture->GetHeight() / 4, aTexture->GetNumMipMaps());

	ActTools::Surface *pSurfaceLDR = pTextureLDR->GetMipmap(0, 0);
	ActTools::Surface *pSurfaceHDR = pTextureHDR->GetMipmap(0, 0);
	pSurfaceLDR->Create(aTexture->GetWidth(), aTexture->GetHeight());
	pSurfaceHDR->Create(aTexture->GetWidth() / 4, aTexture->GetHeight() / 4);

	// normalise the colour
	MathsStorage::Float4*	pInData		= (MathsStorage::Float4*)aTexture->GetMipmap(0, 0)->GetTexels();
	U32*					pLDRData	= (U32*)pSurfaceLDR->GetTexels();
	U16*					pHDRData	= (U16*)pSurfaceHDR->GetTexels();
	for (U32 y=0; y<aTexture->GetHeight(); y+=4)
	{
		for (U32 x=0; x<aTexture->GetWidth(); x+=4)
		{
			const MathsStorage::Float4	vInput0			= pInData[y * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput1			= pInData[y * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput2			= pInData[y * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput3			= pInData[y * aTexture->GetWidth() + x+3];
			const MathsStorage::Float4	vInput4			= pInData[(y+1) * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput5			= pInData[(y+1) * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput6			= pInData[(y+1) * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput7			= pInData[(y+1) * aTexture->GetWidth() + x+3];
			const MathsStorage::Float4	vInput8			= pInData[(y+2) * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput9			= pInData[(y+2) * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput10		= pInData[(y+2) * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput11		= pInData[(y+2) * aTexture->GetWidth() + x+3];
			const MathsStorage::Float4	vInput12		= pInData[(y+3) * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput13		= pInData[(y+3) * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput14		= pInData[(y+3) * aTexture->GetWidth() + x+2];
			const MathsStorage::Float4	vInput15		= pInData[(y+3) * aTexture->GetWidth() + x+3];

			Maths::HVec3	vRGB0	= Maths::HVec3(vInput0.y, vInput0.z, vInput0.w);
			Maths::HVec3	vRGB1	= Maths::HVec3(vInput1.y, vInput1.z, vInput1.w);
			Maths::HVec3	vRGB2	= Maths::HVec3(vInput2.y, vInput2.z, vInput2.w);				
			Maths::HVec3	vRGB3	= Maths::HVec3(vInput3.y, vInput3.z, vInput3.w);
			Maths::HVec3	vRGB4	= Maths::HVec3(vInput4.y, vInput4.z, vInput4.w);
			Maths::HVec3	vRGB5	= Maths::HVec3(vInput5.y, vInput5.z, vInput5.w);
			Maths::HVec3	vRGB6	= Maths::HVec3(vInput6.y, vInput6.z, vInput6.w);				
			Maths::HVec3	vRGB7	= Maths::HVec3(vInput7.y, vInput7.z, vInput7.w);
			Maths::HVec3	vRGB8	= Maths::HVec3(vInput8.y, vInput8.z, vInput8.w);
			Maths::HVec3	vRGB9	= Maths::HVec3(vInput9.y, vInput9.z, vInput9.w);
			Maths::HVec3	vRGB10	= Maths::HVec3(vInput10.y, vInput10.z, vInput10.w);				
			Maths::HVec3	vRGB11	= Maths::HVec3(vInput11.y, vInput11.z, vInput11.w);
			Maths::HVec3	vRGB12	= Maths::HVec3(vInput12.y, vInput12.z, vInput12.w);
			Maths::HVec3	vRGB13	= Maths::HVec3(vInput13.y, vInput13.z, vInput13.w);
			Maths::HVec3	vRGB14	= Maths::HVec3(vInput14.y, vInput14.z, vInput14.w);				
			Maths::HVec3	vRGB15	= Maths::HVec3(vInput15.y, vInput15.z, vInput15.w);

			const Maths::HF32	vLength0	= vRGB0.Length();
			const Maths::HF32	vLength1	= vRGB1.Length();
			const Maths::HF32	vLength2	= vRGB2.Length();
			const Maths::HF32	vLength3	= vRGB3.Length();
			const Maths::HF32	vLength4	= vRGB4.Length();
			const Maths::HF32	vLength5	= vRGB5.Length();
			const Maths::HF32	vLength6	= vRGB6.Length();
			const Maths::HF32	vLength7	= vRGB7.Length();
			const Maths::HF32	vLength8	= vRGB8.Length();
			const Maths::HF32	vLength9	= vRGB9.Length();
			const Maths::HF32	vLength10	= vRGB10.Length();
			const Maths::HF32	vLength11	= vRGB11.Length();
			const Maths::HF32	vLength12	= vRGB12.Length();
			const Maths::HF32	vLength13	= vRGB13.Length();
			const Maths::HF32	vLength14	= vRGB14.Length();
			const Maths::HF32	vLength15	= vRGB15.Length();
			
			const Maths::HF32	vCombine0	= (vLength0 + vLength1 + vLength2 + vLength3) / 4.0f;
			const Maths::HF32	vCombine1	= (vLength4 + vLength5 + vLength6 + vLength7) / 4.0f;
			const Maths::HF32	vCombine2	= (vLength8 + vLength9 + vLength10 + vLength11) / 4.0f;
			const Maths::HF32	vCombine3	= (vLength12 + vLength13 + vLength14 + vLength15) / 4.0f;
			const Maths::HF32	vCombine	= (vCombine0 + vCombine1 + vCombine2 + vCombine3) / 4.0f;

			vRGB0 = vRGB0 * vCombine.Rcp();
			vRGB1 = vRGB1 * vCombine.Rcp();
			vRGB2 = vRGB2 * vCombine.Rcp();
			vRGB3 = vRGB3 * vCombine.Rcp();
			vRGB4 = vRGB4 * vCombine.Rcp();
			vRGB5 = vRGB5 * vCombine.Rcp();
			vRGB6 = vRGB6 * vCombine.Rcp();
			vRGB7 = vRGB7 * vCombine.Rcp();
			vRGB8 = vRGB8 * vCombine.Rcp();
			vRGB9 = vRGB9 * vCombine.Rcp();
			vRGB10 = vRGB10 * vCombine.Rcp();
			vRGB11 = vRGB11 * vCombine.Rcp();
			vRGB12 = vRGB12 * vCombine.Rcp();
			vRGB13 = vRGB13 * vCombine.Rcp();
			vRGB14 = vRGB14 * vCombine.Rcp();
			vRGB15 = vRGB15 * vCombine.Rcp();

			pHDRData[(y/4) * (aTexture->GetWidth()/4) + (x/4)] = (U16)(Maths::Clamp01(vCombine / 8.0f) * 65535.0f);

			pLDRData[y * aTexture->GetWidth() + x]		= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB0)).Saturate());
			pLDRData[y * aTexture->GetWidth() + x+1]		= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB1)).Saturate());
			pLDRData[y * aTexture->GetWidth() + x+2]		= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB2)).Saturate());
			pLDRData[y * aTexture->GetWidth() + x+3]		= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB3)).Saturate());
			pLDRData[(y+1) * aTexture->GetWidth() + x]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB4)).Saturate());
			pLDRData[(y+1) * aTexture->GetWidth() + x+1]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB5)).Saturate());
			pLDRData[(y+1) * aTexture->GetWidth() + x+2]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB6)).Saturate());
			pLDRData[(y+1) * aTexture->GetWidth() + x+3]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB7)).Saturate());
			pLDRData[(y+2) * aTexture->GetWidth() + x]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB8)).Saturate());
			pLDRData[(y+2) * aTexture->GetWidth() + x+1]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB9)).Saturate());
			pLDRData[(y+2) * aTexture->GetWidth() + x+2]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB10)).Saturate());
			pLDRData[(y+2) * aTexture->GetWidth() + x+3]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB11)).Saturate());
			pLDRData[(y+3) * aTexture->GetWidth() + x]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB12)).Saturate());
			pLDRData[(y+3) * aTexture->GetWidth() + x+1]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB13)).Saturate());
			pLDRData[(y+3) * aTexture->GetWidth() + x+2]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB14)).Saturate());
			pLDRData[(y+3) * aTexture->GetWidth() + x+3]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB15)).Saturate());
		}
	}

	return ACT_OK;
}

/// compress an HDR texture into 8 bits per pixel
ACTRESULT HDRCompress8BPP( ActTools::Texture* aTexture, ActTools::Texture*& pTextureLDR, ActTools::Texture*& pTextureHDR )
{
	ActTools::A8R8G8B8 a8r8g8b8;
	ActTools::L16 l16;

	pTextureLDR = new ActTools::Texture();
	pTextureHDR = new ActTools::Texture();

	pTextureLDR->CreateTexture(&a8r8g8b8, aTexture->GetWidth(), aTexture->GetHeight(), aTexture->GetNumMipMaps());
	pTextureHDR->CreateTexture(&l16, aTexture->GetWidth() / 2, aTexture->GetHeight() / 2, aTexture->GetNumMipMaps());

	ActTools::Surface *pSurfaceLDR = pTextureLDR->GetMipmap(0, 0);
	ActTools::Surface *pSurfaceHDR = pTextureHDR->GetMipmap(0, 0);
	pSurfaceLDR->Create(aTexture->GetWidth(), aTexture->GetHeight());
	pSurfaceHDR->Create(aTexture->GetWidth() / 2, aTexture->GetHeight() / 2);

	// normalise the colour
	MathsStorage::Float4*	pInData		= (MathsStorage::Float4*)aTexture->GetMipmap(0, 0)->GetTexels();
	U32*					pLDRData	= (U32*)pSurfaceLDR->GetTexels();
	U16*					pHDRData	= (U16*)pSurfaceHDR->GetTexels();
	for (U32 y=0; y<aTexture->GetHeight(); y+=2)
	{
		for (U32 x=0; x<aTexture->GetWidth(); x+=2)
		{
			const MathsStorage::Float4	vInput0			= pInData[y * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput1			= pInData[y * aTexture->GetWidth() + x+1];
			const MathsStorage::Float4	vInput2			= pInData[(y+1) * aTexture->GetWidth() + x];
			const MathsStorage::Float4	vInput3			= pInData[(y+1) * aTexture->GetWidth() + x+1];

			Maths::HVec3	vRGB0	= Maths::HVec3(vInput0.y, vInput0.z, vInput0.w);
			Maths::HVec3	vRGB1	= Maths::HVec3(vInput1.y, vInput1.z, vInput1.w);
			Maths::HVec3	vRGB2	= Maths::HVec3(vInput2.y, vInput2.z, vInput2.w);				
			Maths::HVec3	vRGB3	= Maths::HVec3(vInput3.y, vInput3.z, vInput3.w);

			const Maths::HF32	vLength0	= vRGB0.Length();
			const Maths::HF32	vLength1	= vRGB1.Length();
			const Maths::HF32	vLength2	= vRGB2.Length();
			const Maths::HF32	vLength3	= vRGB3.Length();
			
			const Maths::HF32	vCombine	= (vLength0 + vLength1 + vLength2 + vLength3) / 4.0f;

			vRGB0 = vRGB0 * vCombine.Rcp();
			vRGB1 = vRGB1 * vCombine.Rcp();
			vRGB2 = vRGB2 * vCombine.Rcp();
			vRGB3 = vRGB3 * vCombine.Rcp();

			pHDRData[(y/2) * (aTexture->GetWidth()/2) + (x/2)] = (U16)(Maths::Clamp01(vCombine / 32.0f) * 65535.0f);

			pLDRData[y * aTexture->GetWidth() + x]		= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB0)).Saturate());
			pLDRData[y * aTexture->GetWidth() + x+1]		= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB1)).Saturate());
			pLDRData[(y+1) * aTexture->GetWidth() + x]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB2)).Saturate());
			pLDRData[(y+1) * aTexture->GetWidth() + x+1]	= Maths::C_RGBA_U8(Maths::C_RGBA_F32(Maths::HVec4(vRGB3)).Saturate());
		}
	}

	return ACT_OK;
}

}
