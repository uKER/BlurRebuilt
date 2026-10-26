#include "stdafx.h"

using namespace CompressLib;

S32 C_CompressLibBase::GetCompressionTechnique(
	const char* techniqueStr) const
{
	for (U32 i = 0; true; i++)
	{
		if (m_techniqueStringTableRef[i][0] == '\0')
			break;
		
		if (!_stricmp(m_techniqueStringTableRef[i], techniqueStr))
			return m_techniqueEnumTableRef[i];
	}

	return -1;
}

const char* C_CompressLibBase::GetCompressionTechniqueString(
	S32 compressionTechnique) const
{
	if (compressionTechnique == -1)
		return 0;

	return m_techniqueStringTableRef[compressionTechnique];
}

S32 C_CompressLibBase::GetDefaultCompressionTechnique(
	E_CompressionFormat compressionFormat) const
{
	if (compressionFormat == E_CompressionFormat_Unknown)
		return -1;
	return m_defaultTechniqueMappingRef[compressionFormat];
}

E_CompressionFormat C_CompressLibBase::GetCompressionFormat(
	S32 compressionFormat) const
{
	if (compressionFormat == -1)
		return E_CompressionFormat_Unknown;

	return m_genericFormatEnumTableRef[compressionFormat];
}

S32 C_CompressLibBase::GetCompressionFormat(
	E_CompressionFormat compressionFormat) const
{
	if (compressionFormat == E_CompressionFormat_Unknown)
		return -1;
	return m_libFormatEnumTableRef[compressionFormat];
}
