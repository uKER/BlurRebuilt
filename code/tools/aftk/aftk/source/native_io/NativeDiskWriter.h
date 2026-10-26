#ifdef _MSC_VER_
#pragma once
#endif

#ifndef NATIVE_DISK_WRITER_H
#define NATIVE_DISK_WRITER_H

namespace Aftk
{
	class C_NativeDiskWriter
	{
	public:
		class WriteException
		{
		public:
			WriteException(S32 errorCode)
			{
				m_status = Aftk::C_Status(Aftk::C_Status::E_Status_Failure);
			}

			WriteException(const Aftk::C_Status& status)
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


		C_NativeDiskWriter();

		~C_NativeDiskWriter();

		void Open(
			const std::string& fileName, 
			const std::string& metaData);

		void Close();

		void BeginBlock(U32 blockId);

		void EndBlock();

		C_NativeDiskWriter& operator <<(const char* pValue);	// null terminated string

		C_NativeDiskWriter& operator <<(const std::string& value);

		C_NativeDiskWriter& operator <<(S64 value);

		C_NativeDiskWriter& operator <<(U64 value);

		C_NativeDiskWriter& operator <<(S32 value);

		C_NativeDiskWriter& operator <<(U32 value);

		C_NativeDiskWriter& operator <<(S16 value);

		C_NativeDiskWriter& operator <<(U16 value);

		C_NativeDiskWriter& operator <<(S8 value);

		C_NativeDiskWriter& operator <<(U8 value);

		C_NativeDiskWriter& operator <<(F64 value);

		C_NativeDiskWriter& operator <<(F32 value);

		C_NativeDiskWriter& operator <<(bool value);

		template <class T, int N>
		FORCE_INLINE C_NativeDiskWriter& operator <<(const UtMaths::BaseVector<T, N>& value);

		C_NativeDiskWriter& operator <<(const Matrix44& value);

		C_NativeDiskWriter& operator <<(const C_BlobData& blob);

		void WriteBytes(const void* pData, U32 size);

	private:
		void Open();

		void WriteHeader(const std::string& metaData);

		Sys::I_File* m_pFile;
		Sys::BinarySerialiser<Ut::NoEndianSwap>* m_pSerialiser;

		U32			m_blockPos[NATIVE_MAX_BLOCK_DEPTH];
		U32			m_blockName[NATIVE_MAX_BLOCK_DEPTH];
		U32			m_blockFirstNestedBlock[NATIVE_MAX_BLOCK_DEPTH];
		U32			m_depth;
		std::string m_fileName;
	};

	template <class T, int N>
	FORCE_INLINE C_NativeDiskWriter&	C_NativeDiskWriter::operator <<(const UtMaths::BaseVector<T, N>& value)
	{
		for (U32 i = 0; i < value.SIZE; i++)
			*this << value[i];
		return*	this;
	}
}
#endif
