#pragma once

namespace CompressLib
{
	class C_CompressLib360 : public C_CompressLibBase
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

		static const C_CompressLib360& Instance();

	private:
		C_CompressLib360();

		static const S32	m_libFormatEnumTable[];
		static const E_CompressionFormat	
							m_genericFormatEnumTable[];

		static const S32	m_techniqueEnumTable[];
		static const char*	m_techniqueStringTable[];
		static const S32	m_defaultTechniqueMapping[];

		static C_CompressLib360	m_instance;
	};
}
	