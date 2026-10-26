#pragma once

#ifndef TU_GENERIC_DISK_READER_H
#define TU_GENERIC_DISK_READER_H

#define TU_GENERIC_DISK_READER_META_FAIL			"Failed to find meta data."
#define TU_GENERIC_DISK_READER_UNKNOWN_FILE_FAIL	"Unknown file format."
#define TU_GENERIC_DISK_READER_EOF_FAIL				"Unexpected end of file."

namespace Tu
{
	class GenericDiskReader
	{
	public:
		class ReadException
		{
		};

		inline GenericDiskReader();

		inline ~GenericDiskReader(); 
		
		inline bool Open(const std::string& fileName, U32 formatStamp);

		inline void Close();

		inline U32	GetNextBlockInfo(U32* size = 0);

		inline void EnterBlock();

		inline void EnterBlock(U32& id);

		inline S32	GetRemainingBlockSize();

		inline void SkipRemainingBlocks();

		inline void SkipBlock();

		inline void ExitBlock();

		inline U32	GetVersion() const;

		inline const std::string& GetMetaData() const;

		inline bool FindBlock(U32 blockId);

		inline GenericDiskReader&	operator >>(bool& value);

		inline GenericDiskReader&	operator >>(F64& value);

		inline GenericDiskReader&	operator >>(F32& value);

		inline GenericDiskReader&	operator >>(U64& value);

		inline GenericDiskReader&	operator >>(U32& value);

		inline GenericDiskReader&	operator >>(U16& value);

		inline GenericDiskReader&	operator >>(U8&  value);

		inline GenericDiskReader&	operator >>(S64& value);

		inline GenericDiskReader&	operator >>(S32& value);

		inline GenericDiskReader&	operator >>(S16& value);

		inline GenericDiskReader&	operator >>(S8&  value);

		inline GenericDiskReader&	operator >>(std::string &value);

		inline void BlockRead(void* pData, U32 size);

	private:
		inline void SafeFread(
			void *pBuf, 
			size_t size, 
			size_t num, 
			FILE *f);

		FILE*	m_pFp;
		U32		m_blockPos[TU_GENERIC_DISK_ACCESS_MAX_BLOCK_DEPTH];
		U32		m_blockSize[TU_GENERIC_DISK_ACCESS_MAX_BLOCK_DEPTH];
		U32		m_blockID[TU_GENERIC_DISK_ACCESS_MAX_BLOCK_DEPTH];
		U32		m_depth;
		U32		m_version;
		std::string m_fileName;
		std::string m_metaData;
	};


	inline GenericDiskReader::GenericDiskReader()
	{
		m_pFp = 0;
		m_depth = 0;
	}

	inline GenericDiskReader::~GenericDiskReader()
	{
		if (m_pFp) 
		{
			fclose(m_pFp);
		}
	}

	inline bool GenericDiskReader::Open(const std::string& fileName, U32 formatStamp)
	{
		m_fileName = fileName;

		m_pFp=fopen(fileName.c_str(),"rb");
		if (m_pFp)
		{
			m_depth=0;
			U32	id0;
			EnterBlock(id0);
			if (id0 == formatStamp)
			{
				*this >> m_version;
				U32 metaID;
				*this >> metaID;
				if (metaID != TU_GENERIC_DISK_ACCESS_ID(TU_GENERIC_DISK_ACCESS_META_TAG))
				{
					throw ReadException();
				}
				*this >> m_metaData;
				return true;
			}
			else
			{
				throw ReadException();
				return false;
			}
		};

		return false;
	}

	inline U32 GenericDiskReader::GetVersion() const
	{
		return m_version;
	}

	inline const std::string& GenericDiskReader::GetMetaData() const
	{
		return m_metaData;
	}

	inline void GenericDiskReader::SafeFread(void *pBuf, size_t size, size_t num, FILE *f)
	{
		if (size == 0)
			return;
		size_t read = fread(pBuf, size, num, f);
		if (read != num)
		{
			if (feof(f))
			{
				ReadException();
			}

			throw ReadException();
		}
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(bool& value) 
	{
		SafeFread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(F64& value) 
	{
		fread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(F32& value) 
	{
		SafeFread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(U64& value) 
	{
		SafeFread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(U32& value) 
	{
		SafeFread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader& GenericDiskReader::operator >>(U16& value) 
	{
		SafeFread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(U8&	value) 
	{
		SafeFread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(S64& value) 
	{
		SafeFread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(S32& value) 
	{
		SafeFread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(S16& value) 
	{
		SafeFread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(S8&	value) 
	{
		SafeFread(&value,sizeof(value),1,m_pFp);
		return*	this;
	}

	inline GenericDiskReader&	GenericDiskReader::operator >>(std::string &string)
	{
		U32	size;
		*this >> size;
		string.resize(size);
		BlockRead((void*)&string[0], size);
		return *this;
	}

	inline void GenericDiskReader::BlockRead(void* pData, U32 size)
	{
		SafeFread(pData, size, 1, m_pFp);
	}

	inline U32 GenericDiskReader::GetNextBlockInfo(U32* size) 
	{
		U32	id;
		// is there a next block
		S32	pos = ftell(m_pFp);
		// POS WITHIN BLOCK?
		if (m_depth>0) 
		{
			S32 blockEnd = m_blockPos[m_depth-1] + 12 + m_blockSize[m_depth-1];
			if ( (pos+12) >= (S32)blockEnd ) 
			{
				if (size)
					*size=0;
				return	0;
			};
		};

		*this >> id;
		U32 offset;
		*this >> offset;
		if (size) {
			*this >> *size;
		};

		fseek(m_pFp, pos, SEEK_SET);
		return	id;
	}

	inline void GenericDiskReader::EnterBlock() 
	{ 
		U32 id; 
		EnterBlock(id); 
	}

	inline void GenericDiskReader::EnterBlock(U32& id) 
	{	U32 offset;
		m_blockPos[m_depth] = ftell(m_pFp);
		*this >> id;
		*this >> offset;
		*this >> m_blockSize[m_depth];
		m_blockID[m_depth] = id;
		m_depth++;
	}

	inline S32 GenericDiskReader::GetRemainingBlockSize() 
	{
		S32	pos = ftell(m_pFp);
		S32	blockEnd = m_blockPos[m_depth-1]+m_blockSize[m_depth-1]+12;
		return	blockEnd-pos;
	}

	inline void GenericDiskReader::SkipBlock() 
	{
		U32	id;
		EnterBlock(id);
		ExitBlock();
	}

	inline void GenericDiskReader::ExitBlock() 
	{
		m_depth--;
		fseek(m_pFp, m_blockPos[m_depth]+12 + m_blockSize[m_depth], SEEK_SET);
	}

	inline void GenericDiskReader::Close() 
	{
		if (m_pFp) 
		{
			fclose(m_pFp);
			m_pFp = NULL;
		}
	}

	inline bool GenericDiskReader::FindBlock(U32 blockId)
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

	inline void GenericDiskReader::SkipRemainingBlocks()
	{
		while (GetNextBlockInfo())
		{
			SkipBlock();
		}
	}
}

#endif
