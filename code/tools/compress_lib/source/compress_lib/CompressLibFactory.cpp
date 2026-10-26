#include "stdafx.h"

using namespace CompressLib;

const E_CompressionLibrary C_CompressLibFactory::m_compressLibEnumTable[] =
{
	E_CompressionLibrary_Squish,
	E_CompressionLibrary_X360,
	E_CompressionLibrary_ATI,
	E_CompressionLibrary_NVidia,

	E_CompressionLibrary_Unknown
};

const char* C_CompressLibFactory::m_compressLibStringTable[] = 
{ 
	"Squish",
	"X360",
	"ATI",
	"NVidia",

	"Unknown",

	"\0"
};	


const I_CompressLib* C_CompressLibFactory::GetCompressLibPtr(E_CompressionLibrary compressionLibrary)
{
	switch (compressionLibrary)
	{
	case E_CompressionLibrary_Squish:
		return &C_CompressLibSquish::Instance();

#ifdef COMPRESS_LIB_ENABLE_X360
	case E_CompressionLibrary_X360:
		return &C_CompressLib360::Instance();
#endif

	case E_CompressionLibrary_NVidia:
		return &C_CompressLibNVidia::Instance();
	
#ifdef COMPRESS_LIB_ENABLE_ATI
	case E_CompressionLibrary_ATI:
		return &C_CompressLibAti::Instance();
#endif

	case E_CompressionLibrary_Unknown:
	default:
		break;
	}

	return 0;
}

const char* C_CompressLibFactory::GetCompressLibString(const E_CompressionLibrary compressLib)
{
	return m_compressLibStringTable[compressLib];
}

E_CompressionLibrary C_CompressLibFactory::GetCompressLibType(const char* compressLib)
{
	for (U32 i = 0; true; i++)
	{
		if (m_compressLibStringTable[i][0] == '\0')
			break;
		
		if (!_stricmp(m_compressLibStringTable[i], compressLib))
			return m_compressLibEnumTable[i];
	}

	return E_CompressionLibrary_Unknown;
}

