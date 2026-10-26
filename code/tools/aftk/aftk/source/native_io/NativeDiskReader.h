#ifndef NATIVE_DISK_READER_H
#define NATIVE_DISK_READER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	///\brief Note that this is not a generic disk read/writer class. 
	///This class is specifically designed for processing A3D files.
	///
	class C_NativeDiskReader
	{
	public:
		class C_ReadException
		{
		public:
			C_ReadException(S32 errorCode)
			{
				m_status = Aftk::C_Status(Aftk::C_Status::E_Status_Failure);
			}

			C_ReadException(const C_Status& status)
			{
				m_status = status;
			}

			C_Status GetStatus() const
			{
				return m_status;
			}

		private:
			C_Status m_status;
		};

		C_NativeDiskReader();

		~C_NativeDiskReader(); 
		
		void Open(const std::string& fileName);

		void Close();

		U32 GetNextBlockInfo(U32* size = 0);

		U32 EnterBlock();

		void SkipBlock();

		void ExitBlock();

		U32	GetVersion() const;

		const std::string& GetMetaData() const;

		bool FindBlock(U32 blockId);

		bool FindID(U32 blockId);

		C_NativeDiskReader& operator >>(std::string &value);

		C_NativeDiskReader& operator >>(S64& value);

		C_NativeDiskReader& operator >>(U64& value);

		C_NativeDiskReader& operator >>(S32& value);

		C_NativeDiskReader& operator >>(U32& value);

		C_NativeDiskReader& operator >>(S16& value);

		C_NativeDiskReader& operator >>(U16& value);

		C_NativeDiskReader& operator >>(S8& value);

		C_NativeDiskReader& operator >>(U8& value);

		C_NativeDiskReader& operator >>(F64& value);

		C_NativeDiskReader& operator >>(F32& value);

		C_NativeDiskReader& operator >>(bool& value);

		template<class T, int N>
		FORCE_INLINE C_NativeDiskReader&	operator >>(UtMaths::BaseVector<T, N>& value);

		C_NativeDiskReader& operator >>(Matrix44& value);

		C_NativeDiskReader& operator >>(C_BlobData& blob);

		///\brief fread wrapper
		///
		void ReadBytes(void* pData, U32 size, bool endianSwitch);

		///\brief fread wrapper that includes exception throwing and automatic endian swapping.
		///
		void ReadBytes(void* pData, U32 size);

	private:
		void Open();

		///\brief Returns true if this is an A3D file version prior to 1.75. Prior to this there was no
		///consideration given to endianness.
		bool IsLegacyA3DFile();

		void OpenLegacyA3DFile();

		void OpenA3DFile();

		void ReadHeader();

		Sys::I_File* m_pFile;
		Sys::BinaryDeserialiserAutoEndian* m_pDeserialiser;
		U32		m_blockPos[NATIVE_MAX_BLOCK_DEPTH];
		U32		m_blockSize[NATIVE_MAX_BLOCK_DEPTH];
		U32		m_blockID[NATIVE_MAX_BLOCK_DEPTH];
		U32		m_depth;
		U32		m_version;
		U32		m_compressionMode;

		std::string m_fileName;
		std::string m_metaData;
		bool m_endianSwitchRequired;
	};

	template<class T, int N>
	FORCE_INLINE C_NativeDiskReader& C_NativeDiskReader::operator >>(UtMaths::BaseVector<T, N>& value)
	{
		for (U32 i = 0; i < value.SIZE; i++)
			*this >> value[i];

		return*	this;
	}
}
#endif
