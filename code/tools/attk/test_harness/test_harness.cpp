#include "stdafx.h"
#include <Attk/Attk.h>
#include <sstream>

int main(int argc, char* argv[])
{
	std::cout << "Using Attk version " << Attk::C_Global::GetVersion() << std::endl;
#ifdef USER_KENM
	Attk::C_Status status;

	{	
		Attk::C_Image tmp;
//		status = tmp.Read("c:/images/16x16.bmp");
		//status = tmp.Read("c:/images/lenna.tga");
//		status = tmp.Read("PGR4/Art/Circuits/Tokyo/Textures/CityBlocks/block_01/B01_kddannexbase_DIFF.tga");
		status = tmp.Read("PGR4/Art/Circuits/Tokyo/Textures/CityBlocks/block_01/B01_shop06base_SIGMA_DIFF.tga");
//		status = tmp.Read("c:/images/input.bmp");
//		tmp.ChangePixelFormat(Attk::E_PixelFormat_A32R32G32B32F);
//		status = tmp.Write("c:/images/export/lenna_o.bmp");
//		return 0;

		Attk::C_MipmapImageChain mipmapChain;
//		Attk::C_MipmapPixelDataChain mipmapPixelDataChain;

//		tmp.Resize(16, 16, 1);
//		tmp.Resize(8, 8, 1);
//		tmp.Resize(16, 16, 1);
//		tmp.Resize(256, 256, 1);
//		tmp.Resize(1024, 1024, 1);

//		tmp.ChangePixelFormat(Attk::E_PixelFormat_Luminance8U);

//		AttkUt::C_ImageFilter::Filter2D(tmp, AttkUt::C_ImageFilter::E_PixelFilter_Box, 1, 1);
//		AttkUt::C_ImageFilter::Filter2D(tmp, AttkUt::C_ImageFilter::E_PixelFilter_Sharpness, 3, 3);
//		AttkUt::C_ImageFilter::Filter2D(tmp, AttkUt::C_ImageFilter::E_PixelFilter_Emboss, 3, 3);
//		AttkUt::C_ImageFilter::Filter2D(tmp, AttkUt::C_ImageFilter::E_PixelFilter_Bartlett,1,1);

//		AttkUt::C_ImageResize::Resize(tmp, 128, 128, 1, AttkUt::C_ImageFilter::E_PixelFilter_Box);
//		status = tmp.Write("box.bmp");
//		return 0;

		AttkUt::C_GenerateMipmaps::Generate(
			mipmapChain,
			tmp, 
			0);
/*
		AttkUt::C_GenerateMipmaps::Generate(
			mipmapPixelDataChain,
			tmp, 
			0);

		std::cout << "mip chain " << mipmapPixelDataChain.GetPixelDataSize() << "\n";
		*/
//		AttkUt::C_ImageResize::Resize(tmp, 128, 128, 1, AttkUt::C_ImageFilter::E_PixelFilter_Box);

		for (U32 i = 0; i < mipmapChain.GetNumLevels(); i++)
		{
			Attk::C_Image* pMipLevel = mipmapChain.GetLevelPtr(i);
			std::stringstream ss;
			ss << "miptest/test_" << i << ".bmp";
			status = pMipLevel->Write(ss.str().c_str());
		}
	}
#endif

#ifdef USER_OWENT
/*
	Attk::C_Image tmp[4];

	//load volume slices
	tmp[0].Read("test_textures\\red.tga");
	tmp[1].Read("test_textures\\green.tga");
	tmp[2].Read("test_textures\\blue.tga");
	tmp[3].Read("test_textures\\black.tga");
	//tmp[4].Read("test_textures\\yellow.tga");
	//tmp[5].Read("test_textures\\black.tga");


	//pack into single volume texture
	Attk::E_PixelFormat pixFormat = tmp[0].GetPixelFormat();
	U32 w=tmp[0].GetWidth(), h=tmp[0].GetHeight();
	for (U32 i=1; i<4; ++i)
	{
		tmp[i].ChangePixelFormat(pixFormat);
		AttkUt::C_ImageResize::Resize(tmp[i],w, h, 1);
	}
	Attk::C_Image image;
	{
		image.NewPixelData(w, h, 4, pixFormat, 0);
		U8* volumeData = image.GetPixelDataPtr();

		U32 offset = 0;
		for (U32 i=0; i<4; ++i)
		{
			memcpy(volumeData+offset, tmp[i].GetPixelDataPtr(), tmp[i].GetPixelDataSize());
			offset += tmp[i].GetPixelDataSize();
		}
	}

	//mip & write out each slice
	Attk::C_Status status = AttkUt::C_ImageResize::Resize(image, 128, 128, 2);
	char file[256];
	for (U32 i=0; i<image.GetDepth(); ++i)
	{
		U32 offset = image.GetWidth() * image.GetHeight()
			* Attk::C_BaseImage::GetBytesPerPixel(image.GetPixelFormat()) * i;
		U8* sliceData = image.GetPixelDataPtr();
		sliceData += offset;
		sprintf(file, "attktest\\volslice_%d.png", i);

		Attk::C_Image slice;
		slice.NewPixelData(image.GetWidth(), image.GetHeight(), 1, pixFormat, sliceData);
		slice.Write(file);
	}
*/

	Attk::C_Status status;
	Attk::C_CubeMap tmp;
	status = tmp.Read("C:\\Program Files\\ATI Research Inc\\CubeMapGen v1.1\\Textures\\CubeMaps\\SkyBeamHDR512.dds");

	//tmp[Attk::E_CubeMap_Face_X_Pos].Read("test_textures\\red.tga");
	//tmp[Attk::E_CubeMap_Face_X_Neg].Read("test_textures\\green.tga");
	//tmp[Attk::E_CubeMap_Face_Y_Pos].Read("test_textures\\blue.tga");
	//tmp[Attk::E_CubeMap_Face_Y_Neg].Read("test_textures\\yellow.tga");
	//tmp[Attk::E_CubeMap_Face_Z_Pos].Read("test_textures\\black.tga");
	//tmp[Attk::E_CubeMap_Face_Z_Neg].Read("test_textures\\white.tga");

	Attk::C_MipmapCubeMapImageChain mipmapChain;
	AttkUt::C_GenerateMipmaps::Generate(mipmapChain, tmp, 16);

	char filename[256];
	for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
	{
		for (U32 j=0; j<mipmapChain[i].GetNumLevels(); ++j)
		{
			sprintf(filename, "attktest\\attktest_%d_%d.png", i, j);
			mipmapChain[i].GetLevelPtr(j)->ChangePixelFormat(Attk::E_PixelFormat_R8G8B8A8U);
			mipmapChain[i].GetLevelPtr(j)->Write(filename);
		}
	}
#endif
	return 0;
}
