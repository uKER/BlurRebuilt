#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "SystemHeaders.h"
#endif

//using namespace Audio;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
 
////////////////////////////////////////////////////////////////////////////////
//	new
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
void* Audio::Definition::operator new(size_t sz)
{
	return System::Alloc(sz);
}

////////////////////////////////////////////////////////////////////////////////
//	delete
////////////////////////////////////////////////////////////////////////////////
void Audio::Definition::operator delete(void* ptr)
{
	System::Free(ptr);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Definition
////////////////////////////////////////////////////////////////////////////////
Audio::Definition::Definition()
{
    m_pData = NULL;
    m_pchStringTable = NULL;
    m_pchObjectName = NULL;
    m_uNumComponents = 0;
	m_u32DataSize = 0;
#ifdef NO_STRING_TABLE_ON_SPU
	m_u32DataSizeSpu = 0;
#endif
	m_pObject = NULL;
	m_uRefCount = 0;
}

////////////////////////////////////////////////////////////////////////////////
//	~Definition
////////////////////////////////////////////////////////////////////////////////
Audio::Definition::~Definition()
{
#ifndef PLATFORM_SPU
	if (m_pData)
		System::Free(m_pData);
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	Filename
////////////////////////////////////////////////////////////////////////////////
const Util::C_String& Audio::Definition::FileName() const 
{ 
	return m_strFilename; 
}

////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::Definition::Init(const char* pchName, void* pData, U32 u32Size)
{
    m_pData = pData;
	m_u32DataSize = u32Size;

	#ifndef	CFG_GOLD
#ifndef PLATFORM_SPU
	m_strFilename = pchName;
#endif
	#endif

    U32* puHeader = (U32*)pData;

	const char* pchDatTag = "AOBJ";
    if ((*puHeader) != 0x414f424a)		
	{
		// is header reverse of "AOBJ"? ie "JBOA" - if so its incorrect endian
		if (*(puHeader) == 0x4a424f41)
		{
			AUDIO_SYS_ERROR("Could not load '%s' : Dat file is wrong endian", pchName);
			return EndianMismatch;
		}
		else
		{
			AUDIO_SYS_ERROR("Could not load '%s' : Dat file has unknown header '%c%c%c%c'. Correct header should be '%s' ", pchName, *((char*)puHeader), *(((char*)puHeader)+1), *(((char*)puHeader)+2), *(((char*)puHeader)+3), pchDatTag);
	        return UnknownDefinitionType;
		}
	}

    //  version number
    U32 uVersion = puHeader[1];

	if ((uVersion >> 16) != DAT_FILE_VERSION_MAJOR || (uVersion & 0x0000ffff) != DAT_FILE_VERSION_MINOR)
	{
		AUDIO_SYS_ERROR("Dat file version mismatch when attempting to load %s! Version in file is %d.%d, version in engine is %d.%d", pchName, uVersion >> 16, uVersion & 0x0000ffff, DAT_FILE_VERSION_MAJOR, DAT_FILE_VERSION_MINOR);
		return VersionMismatch;
	}

    //  get start of string table
    U32 uTableOffset = puHeader[2];
    m_pchStringTable = (char*)puHeader + uTableOffset;

#ifdef NO_STRING_TABLE_ON_SPU
	m_u32DataSizeSpu = uTableOffset + strlen(m_pchStringTable);
#endif

    //  object name
    m_pchObjectName = &m_pchStringTable[puHeader[3]];

    //  num components
    m_uNumComponents = puHeader[4];

    m_uFrameSkip = puHeader[5];

    m_pComponentData = &puHeader[6];

    return Success;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

