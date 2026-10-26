#include "stdafx.h"
#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskWriter.h"
#include <system/misc/StringUtil.h>

#include <fcntl.h> // for open()
#include <cerrno> // for errno 
#include <cstdio> // for perror()
#ifndef YDL
#include <io.h>
#endif
#ifndef GCC
#include <share.h>
#endif

using namespace Aftk;

C_NativeDiskWriter::C_NativeDiskWriter() 
{
	m_pFile = 0;
	m_pSerialiser = 0;
	m_depth = 0;
	m_blockFirstNestedBlock[0] = 0xFFFFFFFF;
}

C_NativeDiskWriter::~C_NativeDiskWriter() 
{
	Close();
}

void C_NativeDiskWriter::BeginBlock(U32 name) 
{
	m_blockName[m_depth] = name;
	m_blockPos[m_depth] = (U32)(m_pFile->GetPosition());

	//write amount of data written 
	//to previous depth

	if (m_depth > 0) {
		if (m_blockFirstNestedBlock[m_depth - 1] == 0xFFFFFFFF)
			m_blockFirstNestedBlock[m_depth - 1] = m_blockPos[m_depth] - m_blockPos[m_depth - 1] - 12;
	}

	m_blockFirstNestedBlock[m_depth] = 0xFFFFFFFF;

	m_depth++;
	if (m_depth >= NATIVE_MAX_BLOCK_DEPTH) {
		//DebugUtils::DEBUGMANAGER << "ExportFile::max depth exceeded.\n";
	}

	*this << name;
	*this << U32(0); //Will give num bytes to next nested block
	*this << U32(0); //Will give num bytes in block
};

void C_NativeDiskWriter::EndBlock() 
{
	S32	curr_pos = (U32)m_pFile->GetPosition();
	m_depth--;

	S32	block_start = m_blockPos[m_depth];
	m_pFile->SetPosition(block_start+8);
	S32	block_size = curr_pos-block_start -12;

	WriteBytes(&block_size, sizeof(S32));

	if (m_blockFirstNestedBlock[m_depth] == 0xFFFFFFFF)
		m_blockFirstNestedBlock[m_depth] = block_size;

	m_pFile->SetPosition(block_start + 4);
	WriteBytes(&m_blockFirstNestedBlock[m_depth], sizeof(S32));
	//fwrite(&blockFirstNestedBlock[depth], 1,4,m_pFp);

	m_pFile->SetPosition(curr_pos);
};

void C_NativeDiskWriter::Open()
{
	//Create output folder.
	if (!Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(m_fileName.c_str())))
		throw WriteException(C_Status(C_Status::E_Status_Failure, "Failed to create directory. Check folder permissions."));
	
	m_pFile = Sys::C_FileSystem::Instance().Open(
		m_fileName.c_str(), 
		Sys::E_FileAccess_Write);

	//Create deserialiser.
	if (m_pFile)
		m_pSerialiser = new Sys::BinarySerialiser<Ut::NoEndianSwap>(*m_pFile);

	if (!m_pFile || !m_pSerialiser)
	{
		throw WriteException(C_Status(C_Status::E_Status_Failure, "Failed to open file for writing. Check file is not read-only."));
	}	
}

void C_NativeDiskWriter::WriteHeader(const std::string& metaData)
{
	//Header description:
	//Endian tag		: 4bytes.
	//Version tag		: 4bytes.
	//Compression mode	: 4bytes.
	//'A3D2' Block...

	U32 endianTag(1);
	U32 versionTag(NATIVE_FILE_VERSION);
	U32 compressionMode = 0;
	WriteBytes(&endianTag, sizeof(U32));
	WriteBytes(&versionTag, sizeof(U32));
	WriteBytes(&compressionMode, sizeof(U32));

	//Precede meta data by 'META'.
	*this << (U32)C_NativeUtility::GetBlockIdInt("META");
	*this << metaData;
}

void C_NativeDiskWriter::Open(
	const std::string& fileName, 
	const std::string& metaData)
{
	m_fileName = fileName;
	Open();
	WriteHeader(metaData);
	BeginBlock(NATIVE_FILE_TYPE_ID_A3D);
}

void C_NativeDiskWriter::Close() 
{
	if (m_pFile) 
	{
		EndBlock();
	}
	delete m_pFile;
	m_pFile = 0;

	delete m_pSerialiser;
	m_pSerialiser = 0;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(const C_BlobData& blob)
{
	U32	size = blob.GetSize();
	*this << size;
	void *pData = blob.GetDataPtr();
	WriteBytes(pData, size);
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(const char*	value) 
{
	U32	len = (U32)strlen(value);
	*this << (len+1);
	WriteBytes((void*)value, len);
	//Write our null termination character manually.
	*this << (U8) 0;
	return	*this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(const std::string &value) 
{
	U32 length = (U32)strlen(value.c_str());
	*this << length;
	WriteBytes((void*)value.c_str(), length);
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(const Matrix44& value) 
{
	WriteBytes((void*)&value, sizeof(Matrix44));
	return*	this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(S64 value)
{
	WriteBytes(&value, sizeof(S64));
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(U64 value)
{
	WriteBytes(&value, sizeof(U64));
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(S32 value)
{
	WriteBytes(&value, sizeof(S32));
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(U32 value)
{
	WriteBytes(&value, sizeof(U32));
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(S16 value)
{
	WriteBytes(&value, sizeof(S16));
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(U16 value)
{
	WriteBytes(&value, sizeof(U16));
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(S8 value)
{
	WriteBytes(&value, sizeof(S8));
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(U8 value)
{
	WriteBytes(&value, sizeof(U8));
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(F64 value)
{
	WriteBytes(&value, sizeof(F64));
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(F32 value)
{
	WriteBytes(&value, sizeof(F32));
	return *this;
}

C_NativeDiskWriter& C_NativeDiskWriter::operator <<(bool value)
{
	WriteBytes(&value, sizeof(bool));
	return *this;
}

void C_NativeDiskWriter::WriteBytes(const void* pData, U32 size)
{
	if (m_pFile->GetPosition() + size >= 190)
		int breakMe = 1;	

	if (size == 0)
		return;

	if (!m_pSerialiser->Write(pData, size))
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

		throw WriteException(C_Status(C_Status::E_Status_Failure, buffer));
	}
}
