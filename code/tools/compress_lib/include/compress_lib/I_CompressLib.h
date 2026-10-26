#pragma once

namespace CompressLib
{
	class I_CompressLib
	{
	public:
		virtual bool CompressSingleMipLevel(
			C_OutputTexture& output,
			E_CompressionFormat compressionFormat,
			bool useAlpha,
			S32 compressionTechnique,
			E_Endianness outputEndianness,
			U32 width,
			U32 height,
			Attk::E_PixelFormat format,
			const U8* pInput) const = 0;

		virtual bool CompressSingleMipLevel(
			C_OutputTexture& output, 
			E_CompressionFormat compressionFormat,
			bool useAlpha,
			S32 compressionTechnique,
			E_Endianness outputEndianness,
			const Attk::C_Image& input) const = 0;

		virtual S32 GetDefaultCompressionTechnique(
			E_CompressionFormat compressionFormat) const = 0;

		virtual S32 GetCompressionTechnique(
			const char* techniqueStr) const = 0;

		virtual const char* GetCompressionTechniqueString(
			S32 compressionTechnique) const = 0;

		virtual E_CompressionFormat GetCompressionFormat(
			S32 compressionFormat) const = 0;

		virtual S32 GetCompressionFormat(
			E_CompressionFormat compressionFormat) const = 0;

	};
}
