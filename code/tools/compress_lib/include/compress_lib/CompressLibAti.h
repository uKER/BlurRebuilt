#pragma once

namespace CompressLib
{
	class C_CompressLibAti : public C_CompressLibBase
	{
	public:
		bool CompressSingleMipLevel(
			C_OutputTexture& output,
			E_CompressionFormat compressionFormat,
			bool useAlpha,
			S32 compressionTechnique,
			E_Endianness outputEndianness,
			U32 width,
			U32 height,
			Attk::E_PixelFormat format,
			const U8* pInput) const;

		bool CompressSingleMipLevel(
			C_OutputTexture& output, 
			E_CompressionFormat compressionFormat,
			bool useAlpha,
			S32 compressionTechnique,
			E_Endianness outputEndianness,
			const Attk::C_Image& input) const;

		static const C_CompressLibAti& Instance();

	private:
		C_CompressLibAti();

		bool ValidateFormatAndTechnique(
			E_CompressionFormat compressionFormat,
			S32 compressionTechnique) const;

		static const S32	m_libFormatEnumTable[];
		static const E_CompressionFormat
							m_genericFormatEnumTable[];

		static const S32	m_techniqueEnumTable[];
		static const char*	m_techniqueStringTable[];
		static const S32	m_defaultTechniqueMapping[];

		static C_CompressLibAti	m_instance;
	};
}
	