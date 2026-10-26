#pragma once

namespace CompressLib
{
	class C_CompressLibBase : public I_CompressLib
	{
	public:
		S32 GetDefaultCompressionTechnique(
			E_CompressionFormat compressionFormat) const;

		S32 GetCompressionTechnique(
			const char* techniqueStr) const;

		const char* GetCompressionTechniqueString(
			S32 compressionTechnique) const;

		E_CompressionFormat GetCompressionFormat(
			S32 compressionFormat) const;

		S32 GetCompressionFormat(
			E_CompressionFormat compressionFormat) const;

	protected:
		const S32*		m_libFormatEnumTableRef;
		const E_CompressionFormat*	
						m_genericFormatEnumTableRef;

		const S32*		m_techniqueEnumTableRef;
		const char**	m_techniqueStringTableRef;
		const S32*		m_defaultTechniqueMappingRef;
	};
}
