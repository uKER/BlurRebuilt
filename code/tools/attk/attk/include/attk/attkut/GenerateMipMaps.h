#ifndef ATTK_GENERATE_MIP_MAPS_H
#define ATTK_GENERATE_MIP_MAPS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AttkUt
{
	class C_GenerateMipmaps
	{
	public:

#ifndef WRAP_32
		struct CubeMapFilterParameters
		{
			enum E_FilterType
			{
				E_FilterType_Disc = 0,
				E_FilterType_Cone,
				E_FilterType_Cosine,
				E_FilterType_AngularGaussian
			};

			enum E_EdgeFixupType
			{
				E_EdgeFixupType_None = 0,
				E_EdgeFixupType_PullLinear,
				E_EdgeFixupType_PullHermite,
				E_EdgeFixupType_AvgLinear,
				E_EdgeFixupType_AvgHermite
			};

			enum E_Compression
			{
				E_Compression_None = 0,
				E_Compression_DXT1 = 100,
				E_Compression_DXT3 = 101,
				E_Compression_DXT5 = 102
			};

			U32 numThreads;
			F32 baseFilterAngle;
			F32 initialMipAngle;
			F32 mipAnglePerLevelScale;
			E_FilterType filterType;
			E_EdgeFixupType fixupType;
			U32 fixupWidth;
			U8 useSolidAngle;
			E_Compression compression;

			CubeMapFilterParameters();
		};
#endif

		static Attk::C_Status Generate(
			Attk::C_MipmapImageChain& mipmapChain,
			const Attk::C_Image& image,
			const U32 levels);

		static Attk::C_Status Generate(
			Attk::C_MipmapPixelDataChain& mipmapChain,
			const Attk::C_Image& image,
			const U32 levels);

#ifndef WRAP_32
		static Attk::C_Status Generate(
			Attk::C_MipmapCubeMapImageChain mipmapChain,
			const Attk::C_CubeMap& cubemap,
			const U32 levels);

		static Attk::C_Status Generate(
			Attk::C_MipmapCubeMapPixelDataChain& mipmapChain,
			const Attk::C_CubeMap& cubemap,
			const U32 levels);

		static CubeMapFilterParameters	m_CubeMapFilterParams;
#endif
	};
}

#endif
