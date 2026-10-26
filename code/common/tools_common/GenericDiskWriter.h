#pragma once
#ifndef TU_GENERIC_DISK_WRITER_H
#define TU_GENERIC_DISK_WRITER_H

#define TU_GENERIC_DISK_WRITER_DISK_SPACE_FAIL	"Out of disk space."

namespace Tu
{
	class GenericDiskWriter
	{
	public:
		class WriteException
		{
		};

		inline GenericDiskWriter();

		inline ~GenericDiskWriter();

		inline bool Open(
			const std::string& fileName, 
			U32 formatStamp,
			U32 version,
			const std::string* pMetaData = 0);
		
		inline void Close();

		inline void BeginBlock(U32	blockType);

		inline void EndBlock();

		inline GenericDiskWriter&	operator<<(bool	value);

		inline GenericDiskWriter&	operator<<(F64 value);

		inline GenericDiskWriter&	operator<<(F32 value);

		inline GenericDiskWriter&	operator<<(U64 value);

		inline GenericDiskWriter&	operator<<(U32 value);

		inline GenericDiskWriter&	operator<<(U16 value);

		inline GenericDiskWriter&	operator<<(U8 value);

		inline GenericDiskWriter&	operator<<(S64 value);

		inline GenericDiskWriter&	operator<<(S32 value);

		inline GenericDiskWriter&	operator<<(S16 value);

		inline GenericDiskWriter&	operator<<(S8 value);

		//null terminated string
		inline GenericDiskWriter&	operator<<(const char* pValue);	

		inline GenericDiskWriter&	operator<<(const std::string& value);

		inline void	BlockWrite(void* pData, U32 size);

	private:
		inline void SafeFwrite(void *pBuf, size_t size, size_t num, FILE *f);

		FILE*		m_fp;
		U32			m_blockPos[TU_GENERIC_DISK_ACCESS_MAX_BLOCK_DEPTH];
		U32			m_blockName[TU_GENERIC_DISK_ACCESS_MAX_BLOCK_DEPTH];
		U32			m_blockFirstNestedBlock[TU_GENERIC_DISK_ACCESS_MAX_BLOCK_DEPTH];
		U32			m_depth;
		std::string m_fileName;
	};


	inline GenericDiskWriter::GenericDiskWriter() 
	{
		m_fp = 0;
		m_depth = 0;
		m_blockFirstNestedBlock[0] = 0xFFFFFFFF;
	}

	inline GenericDiskWriter::~GenericDiskWriter() 
	{
		if (m_fp) 
		{
			fclose(m_fp);
		}
	}

	inline void GenericDiskWriter::BeginBlock(U32 name) 
	{
		m_blockName[m_depth] = name;
		m_blockPos[m_depth] = ftell(m_fp);

		//write amount of data written 
		//to previous m_depth

		if (m_depth > 0) {
			if (m_blockFirstNestedBlock[m_depth - 1] == 0xFFFFFFFF)
				m_blockFirstNestedBlock[m_depth - 1] = m_blockPos[m_depth] - m_blockPos[m_depth - 1] - 12;
		}

		m_blockFirstNestedBlock[m_depth] = 0xFFFFFFFF;

		m_depth++;
		if (m_depth >= TU_GENERIC_DISK_ACCESS_MAX_BLOCK_DEPTH) {
			//DebugUtils::DEBUGMANAGER << "ExportFile::max m_depth exceeded.\n";
		}

		*this << name;
		*this << U32(0); //Will give num bytes to next nested block
		*this << U32(0); //Will give num bytes in block
	};

	inline void GenericDiskWriter::EndBlock() 
	{
		S32	curr_pos = ftell(m_fp);
		m_depth--;

		S32	block_start= m_blockPos[m_depth];
		fseek(m_fp, block_start+8, SEEK_SET);
		S32	block_size = curr_pos-block_start -12;
		
		if (fwrite(&block_size, 1,4,m_fp) < 1)
			throw WriteException();

		// [SteveC] Write offset
		if (m_blockFirstNestedBlock[m_depth] == 0xFFFFFFFF)
			m_blockFirstNestedBlock[m_depth] = block_size;

		fseek(m_fp, block_start + 4, SEEK_SET);
		if (fwrite(&m_blockFirstNestedBlock[m_depth], 1,4,m_fp) < 1)
			throw WriteException();

		fseek(m_fp, curr_pos, SEEK_SET);
	};

	inline GenericDiskWriter& GenericDiskWriter::operator <<(bool value) 
	{
		SafeFwrite(&value, sizeof(value), 1, m_fp);
		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(F64 value) 
	{
		SafeFwrite(&value, sizeof(value), 1, m_fp);
		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(F32 value) 
	{
		SafeFwrite(&value, sizeof(value), 1, m_fp);
		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(U64 value) 
	{
		SafeFwrite(&value, sizeof(value), 1, m_fp);
		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(U32 value) 
	{
		SafeFwrite(&value, sizeof(value), 1, m_fp);
		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(S64 value) 
	{
		SafeFwrite(&value, sizeof(value), 1, m_fp);
		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(S32 value) 
	{
		SafeFwrite(&value, sizeof(value), 1, m_fp);
		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(U16 value) 
	{
		SafeFwrite(&value, sizeof(value), 1, m_fp);
		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(S16 value) 
	{
		SafeFwrite(&value, sizeof(value), 1, m_fp);
		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(U8 value)
	{
		SafeFwrite(&value, sizeof(value), 1, m_fp);
		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(S8 value) 
	{
		if (fwrite(&value, sizeof(value), 1,m_fp) < 1)
			throw WriteException();

		return	*this;
	}

	inline GenericDiskWriter& GenericDiskWriter::operator <<(const std::string &value) 
	{
		U32	len = (U32)value.length();
		*this << len;
		BlockWrite((void*)&value[0], len);
		return *this;
	}

	void GenericDiskWriter::BlockWrite(void* pData, U32 size)
	{
		SafeFwrite(pData, size, 1, m_fp);
	}

	inline bool GenericDiskWriter::Open(
		const std::string& fileName, 
		U32 formatStamp,
		U32 version,
		const std::string* pMetaData)
	{
		m_fileName = fileName;
		m_depth=0;

		m_fp = fopen(m_fileName.c_str(), "wb");

		if (m_fp) 
		{
			BeginBlock(formatStamp);
			*this << version;
			//Precede meta data by 'META'.
			*this << (U32)TU_GENERIC_DISK_ACCESS_ID('META');
			if (!pMetaData)
			{
				*this << std::string();
			}
			else
			{
				*this << *pMetaData;
			}
			return true;
		}

		return false;
	};

	inline void GenericDiskWriter::Close() 
	{
		if (m_fp) 
		{
			EndBlock();
			fclose(m_fp);
			m_fp=0;
		}
	}

	inline void GenericDiskWriter::SafeFwrite(void *pBuf, size_t size, size_t num, FILE *f)
	{
		if (size == 0)
			return;
		size_t write = fwrite(pBuf, size, num, f);
		if (write != num)
		{
			throw WriteException();
		}
	}
}

#endif