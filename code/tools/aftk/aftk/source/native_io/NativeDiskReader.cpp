#include "stdafx.h"
#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"

#include <fcntl.h> // for open()
#include <cerrno> // for errno 
#include <cstdio> // for perror()

using namespace Aftk;

//#pragma message("TODO: Rewrite Aftk I/O system using core tech. Currently its very ugly.")

C_NativeDiskReader::C_NativeDiskReader()
{
	m_pFile = 0;
	m_pDeserialiser = 0;
	m_depth = 0;
}

C_NativeDiskReader::~C_NativeDiskReader()
{
	Close();
}

bool C_NativeDiskReader::IsLegacyA3DFile()
{
	//Legacy files will be little endian only.
	if (!C_NativeUtility::IsLittleEndian())
	{
		m_endianSwitchRequired = true;
	}
	else
	{
		m_endianSwitchRequired = false;
	}

	C_Status status;

	Open();

	bool isLegacy;
	U32 fileId;
	ReadBytes(&fileId, sizeof(U32), false);

	if (
		fileId == C_NativeUtility::GetBlockIdInt("A3D ") ||
		fileId == C_NativeUtility::GetBlockIdInt("A3D2")
	)
		isLegacy = true;
	else
		isLegacy = false;

	Close();

	return isLegacy;
}

void C_NativeDiskReader::OpenLegacyA3DFile()
{
	Open();
	AFTK_ASSERT(m_pFile);

	m_depth = 0;
	U32	id0 = EnterBlock();
	if (id0 == C_NativeUtility::GetBlockIdInt("A3D "))
	{
		F32 version;
		*this >> version;
		m_version = (U32)(version*100.0f);

		if (m_version == 100)
		{
			//this version is no longer supported
			throw C_ReadException(C_Status(C_Status::E_Status_Failure, "A3D V1.0 is no longer supported.", 1));
		}
		if (m_version == 110)
		{
			//this version is no longer supported
			throw C_ReadException(C_Status(C_Status::E_Status_Failure, "A3D V1.1 is no longer supported.", 1));
		}
		if (m_version == 143)
		{
			//this version is no longer supported
			throw C_ReadException(C_Status(C_Status::E_Status_Failure, "A3D V1.43 is no longer supported.", 1));
		}
		if (m_version == 175)
		{
			//this version is no longer supported
			throw C_ReadException(C_Status(C_Status::E_Status_Failure, "A3D V1.75 is no longer supported.", 1));
		}
	}
	else
	{
		*this >> m_version;
		if (m_version > 150)
		{
			U32 metaID;
			ReadBytes(&metaID, sizeof(U32), false);
			if (metaID != C_NativeUtility::GetBlockIdInt("META"))
			{
				throw C_ReadException(C_Status(C_Status::E_Status_Failure, "Failed to find meta data."));
			}
			*this >> m_metaData;
		}
		if (m_version > NATIVE_FILE_VERSION)
		{
			//We're trying to read in a file of a later version than we can handle!
			//Abort.
			throw C_ReadException(C_Status(C_Status::E_Status_Failure,
				"Attempting to read a version of the file greater than we can handle.", 1));
		}
		else if (m_version < NATIVE_LOWEST_FILE_VERSION)
		{
			//We're trying to read in a file of a later version than we can handle!
			//Abort.
			throw C_ReadException(C_Status(C_Status::E_Status_Failure, 
				"Attempting to read an obsolete file.", 1));
		}
	}

	EnterBlock();
	{
		U32 compressionType;

		U32 position3ElementType;
		U32 normal3ElementType;
		U32 textureCoordinate4ElementType;
		U32 matrix44ElementType;

		U8 numTextureCoordinateElements;
		U8 colour4BitsPerPixel;

		*this >> compressionType;

		//Bits-per-datatype info
		*this >> position3ElementType;
		*this >> normal3ElementType;
		*this >> textureCoordinate4ElementType;
		*this >> matrix44ElementType;

		*this >> numTextureCoordinateElements;
		*this >> colour4BitsPerPixel;	
	}
	ExitBlock();
}

void C_NativeDiskReader::OpenA3DFile()
{
	Open();

	ReadHeader();

	m_depth=0;
	U32	id0= EnterBlock();
	if (id0 != C_NativeUtility::GetBlockIdInt("A3D2"))
		throw C_ReadException(C_Status(C_Status::E_Status_Failure, "Unexpected file type."));
}

void C_NativeDiskReader::Open(const std::string& fileName)
{
	m_fileName = fileName;
	if (IsLegacyA3DFile())
	{
		OpenLegacyA3DFile();
		return;
	}

	OpenA3DFile();

	if (m_version > NATIVE_FILE_VERSION)
	{
		//We're trying to read in a file of a later version than we can handle!
		//Abort.
		std::stringstream ss;
		ss << "Attempting to read a version of the file greater than we can handle.";
		ss << "A3D version is " << m_version << " while we can only read up to version " << NATIVE_FILE_VERSION << ".";
		throw C_ReadException(C_Status(C_Status::E_Status_Failure, ss.str(), 1));
	}
	else if (m_version < NATIVE_LOWEST_FILE_VERSION)
	{
		std::stringstream ss;
		ss << "Attempting to read an obsolete file.";
		ss << "A3D version is " << m_version << " - minimum supported version is " << NATIVE_LOWEST_FILE_VERSION << ".";
		throw C_ReadException(C_Status(C_Status::E_Status_Failure, ss.str(), 1));
	}
}

void C_NativeDiskReader::ReadHeader()
{
	//Header description:
	//Endian tag		: 4bytes.
	//Version tag		: 4bytes.
	//Compression mode	: 4bytes.
	//'A3D2' Block...
	U32 endianTag;
	ReadBytes(&endianTag, sizeof(U32), false);
	if (endianTag == 1)
		m_endianSwitchRequired = false;
	else
	{
		m_endianSwitchRequired = true;
	}
	ReadBytes(&m_version, sizeof(U32), m_endianSwitchRequired);
	ReadBytes(&m_compressionMode, sizeof(U32), m_endianSwitchRequired);

	U32 metaID;
	ReadBytes(&metaID, sizeof(U32), false);
	if (metaID != C_NativeUtility::GetBlockIdInt("META"))
		throw C_ReadException(C_Status(C_Status::E_Status_Failure, "Failed to find meta data."));
	*this >> m_metaData;
}

U32 C_NativeDiskReader::GetVersion() const
{
	return m_version;
}

const std::string& C_NativeDiskReader::GetMetaData() const
{
	return m_metaData;
}

void C_NativeDiskReader::Open()
{
	m_pFile = Sys::C_FileSystem::Instance().Open(
		m_fileName.c_str(), 
		Sys::E_FileAccess_Read);

	//Create deserialiser.
	if (m_pFile)
		m_pDeserialiser = new Sys::BinaryDeserialiserAutoEndian(*m_pFile);

	if (!m_pFile || !m_pDeserialiser)
	{
		throw C_ReadException(C_Status(C_Status::E_Status_Failure, "Failed to open file."));
	}
}

void C_NativeDiskReader::ReadBytes(void *pBuf, U32 size, bool endianSwitch)
{
	m_pFile->SetEndianSwapFlag(endianSwitch);
//    U32 read = (U32)fread(pBuf, size, 1, m_pFile);
	if (!m_pDeserialiser->Read(pBuf, size))
	{
		static char buffer[1024];
#if defined(WIN32) || defined(AMD64)
		FormatMessage( 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			buffer,
			1024,
			NULL );
#else
		sprintf(buffer, "Something bad happened in ReadBytes");
#endif
		OutputDebugString("FAILED TO READ\n");

		throw C_ReadException(C_Status(C_Status::E_Status_Failure, buffer));
	}
}

void C_NativeDiskReader::ReadBytes(void *pBuf, U32 size)
{
	ReadBytes(pBuf, size, m_endianSwitchRequired);
}

U32 C_NativeDiskReader::GetNextBlockInfo(U32* size) 
{
	// is there a next block
	//S32	pos = ftell(m_pFp);
	U32 pos = (U32)m_pFile->GetPosition();
//#pragma message("TODO: 64-bit file pos issue needs fixing at some point.")
	// POS WITHIN BLOCK?
	if (m_depth>0) 
	{
		U32 blockEnd = m_blockPos[m_depth-1] + 12 + m_blockSize[m_depth-1];
		if ( (pos+12) >= (S32)blockEnd ) 
		{
			if (size)
				*size=0;
			return 0;
		};
	};

	U32 id;
	ReadBytes(&id, sizeof(U32), false);
	U32 offset;
	*this >> offset;
	if (size) 
		*this >> *size;

	//fseek(m_pFp, pos, SEEK_SET);
	m_pFile->SetPosition(pos);
	return id;
}

U32 C_NativeDiskReader::EnterBlock() 
{
	U32 id;
	U32 offset;
	m_blockPos[m_depth] = (U32)m_pFile->GetPosition();
//	m_blockPos[m_depth] = ftell(m_pFp);
	ReadBytes(&id, sizeof(U32), false);
	*this >> offset;
	U32 offsetCopy = offset;
	offsetCopy = C_NativeUtility::EndianSwitch(offsetCopy);
	*this >> m_blockSize[m_depth];
	m_blockID[m_depth] = id;
	m_depth++;
	return id;
}

void C_NativeDiskReader::SkipBlock() 
{
	EnterBlock();
	ExitBlock();
}

void C_NativeDiskReader::ExitBlock() 
{
	m_depth--;
//	fseek(m_pFp, m_blockPos[m_depth]+12 + m_blockSize[m_depth], SEEK_SET);
	S32 seekPos = m_blockPos[m_depth]+12 + m_blockSize[m_depth];
	m_pFile->SetPosition(seekPos);
}

void C_NativeDiskReader::Close() 
{
	delete m_pFile;
	m_pFile = 0;

	delete m_pDeserialiser;
	m_pDeserialiser = 0;
}

bool C_NativeDiskReader::FindBlock(U32 blockId)
{
	U32 tag;
	while (tag = GetNextBlockInfo())
	{
		if (tag == blockId)
		{
			// We've found what we were looking for.
			break;
		}
		else
		{
			// We've encountered an unknown block, skip it.
			SkipBlock();
		}
	}
	if (tag != blockId)
	{
		// We couldn't find what we were looking for. Abort.
		return false;
	}
	return true;
}

bool C_NativeDiskReader::FindID(U32 blockId)
{
	// is there a next block
	//S32	pos = ftell(m_pFp);
	U32 pos = (U32)m_pFile->GetPosition();
	U32 originalPos = pos;
	// POS WITHIN BLOCK?
	U32 blockEnd = 0xffffffff;
	if (m_depth>0) 
	{
		blockEnd = m_blockPos[m_depth-1] + 12 + m_blockSize[m_depth-1];
		if ( (pos+12) >= (S32)blockEnd ) 
		{
			return false;
		};
	};

	while(pos < blockEnd && pos < m_pFile->Length())
	{
		U32 id;
		ReadBytes(&id, sizeof(U32), false);
		if (id == blockId)
		{
			//Rewind to the start of our id.
			m_pFile->SetPosition(pos);
			return true;
		}
		//We're going to have to look at things a byte at a time as our data isn't going to be aligned.
		m_pFile->SetPosition(m_pFile->GetPosition()-3);
		pos = (U32)m_pFile->GetPosition();
	}	

	//We didn't find anything, restore original position.
	m_pFile->SetPosition(originalPos);
	return true;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(C_BlobData &blob)
{
	U32 size;
	*this >> size;
	blob.Resize(size);

	void *dataPtr = blob.GetDataPtr();
	ReadBytes(dataPtr, size, m_endianSwitchRequired);

	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(std::string &string)
{
	U32	size;
	*this >> size;
	string.resize(size);

	ReadBytes((void*)string.c_str(), size, m_endianSwitchRequired);
	
//	if (m_version < 205)
	//Legacy extra-null compensation.
	{
		U32 l = (U32)strlen(string.c_str());
		if (l != size)
			string = string.substr(0, l);
	}
	return	*this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(Matrix44& value)
{
	ReadBytes((void*)&value, sizeof(Matrix44), m_endianSwitchRequired);
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(S64& value)
{
	ReadBytes(&value, sizeof(S64));
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(U64& value)
{
	ReadBytes(&value, sizeof(U64));
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(S32& value)
{
	ReadBytes(&value, sizeof(S32));
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(U32& value)
{
	ReadBytes(&value, sizeof(U32));
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(S16& value)
{
	ReadBytes(&value, sizeof(S16));
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(U16& value)
{
	ReadBytes(&value, sizeof(U16));
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(S8& value)
{
	ReadBytes(&value, sizeof(S8));
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(U8& value)
{
	ReadBytes(&value, sizeof(U8));
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(F64& value)
{
	ReadBytes(&value, sizeof(F64));
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(F32& value)
{
	ReadBytes(&value, sizeof(F32));
	return *this;
}

C_NativeDiskReader& C_NativeDiskReader::operator >>(bool& value)
{
	ReadBytes(&value, sizeof(bool));
	return *this;
}
