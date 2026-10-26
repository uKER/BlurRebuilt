#ifndef NET_BYTE_STREAM_H
#define NET_BYTE_STREAM_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NetPlay
{
	template <U32 bufferSize> 
	class C_ByteStream
	{
	public:
		C_ByteStream();
		virtual ~C_ByteStream();

		/// Flush all the internal data
		void Flush(void);

		/// Get direct acess to the internal buffer
		U8* InternalBuffer(void);

		/// gets the maximum number of bytes we can store
		U32 Max(void) const; 

		/// gets the current number of bytes stored.
		U32 Current(void) const; 

		/// gets the remaining number of bytes not unpacked yet.
		U32 Remaining(void) const;

		/// Adds an U8 val to the stream.
		void PackU8(U8 val);

		/// Adds an S8 val to the stream.
		void PackS8(S8 val);

		/// Adds an U16 val to the stream.
		void PackU16(U16 val);

		/// Adds an S16 val to the stream.
		void PackS16(S16 val);

		/// Adds an U32 val to the stream.
		void PackU32(U32 val);

		/// Adds an S32 val to the stream.
		void PackS32(S32 val);

		/// Adds an U64 val to the stream.
		void PackU64(U64 val);

		/// Adds an S64 val to the stream.
		void PackS64(S64 val);

		/// Adds an F32 val to the stream.
		void PackF32(F32 val);

		/// Adds an F64 val to the stream.
		void PackF64(F64 val);

		/// Adds a buffer to the stream
		void PackBuffer(void* pData, const S32 dataSize);

		/// Modify the first 2 bytes in the buffer to contain number of encrypted data bytes. required for Xbox VDP support
		void ModifyEncryptedSize(const U16 size);

		/// Its possible to remove a set number of bytes from the head of the list
		void RemovePacked(const U16 sizeToRemove);

		/// Change to unpack mode and set pointers to start of packed data
		void UnPackBegin(void);

		// unpack an U8 value from the stream
		U8 UnPackU8(void);

		// unpack an S8 value from the stream
		S8 UnPackS8(void);

		// unpack an U16 value from the stream
		U16 UnPackU16(void);

		// unpack an S16 value from the stream
		S16 UnPackS16(void);

		// unpack an U32 value from the stream
		U32 UnPackU32(void);

		// unpack an S32 value from the stream
		S32 UnPackS32(void);

		// unpack an U64 value from the stream
		U64 UnPackU64(void);

		// unpack an S64 value from the stream
		S64 UnPackS64(void);

		// unpack an F32 value from the stream
		F32 UnPackF32(void);
		
		// unpack an F64 value from the stream
		F64 UnPackF64(void);

		// unpack a set number of bytes to the external buffer
		void UnPackBuffer(void* pData, const S32 unpackSize);

		/// switch back to packing mode, if refactorBuffer is true then any unpacked data will be removed from the internal buffer
		void UnPackEnd(const bool refactorBuffer);

	private:
		const U32	m_BufferSize;
		U8			m_Buffer[bufferSize];
		U8*			m_pCurrent;
		U8*			m_pUnpack;
	};


	// constructor
	template <U32 bufferSize>
	C_ByteStream<bufferSize>::C_ByteStream() : m_BufferSize(bufferSize), m_pCurrent(m_Buffer), m_pUnpack(NULL)
	{};


	// destructor
	template <U32 bufferSize>
	C_ByteStream<bufferSize>::~C_ByteStream()
	{};

	// flush All the internal data
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::Flush(void)
	{
		m_pUnpack = NULL;
		m_pCurrent = &m_Buffer[0];
	}

	/// Direct access to the address of the overlapped task for passing into API functions
	template <U32 bufferSize>
	U8* C_ByteStream<bufferSize>::InternalBuffer()
	{
		return &m_Buffer[0];
	}

	// get max size
	template <U32 bufferSize>
	U32 C_ByteStream<bufferSize>::Max(void) const
	{
		return m_BufferSize;
	}

		
	// get current size
	template <U32 bufferSize>
	U32 C_ByteStream<bufferSize>::Current(void) const
	{
		return m_pCurrent - m_Buffer;
	}


	// get remaining bytes in buffer.
	template <U32 bufferSize>
	U32 C_ByteStream<bufferSize>::Remaining(void) const
	{
		if (NULL == m_pUnpack)
			return Current();

		return m_pCurrent - m_pUnpack;
	}
	
	// pack U8 into stream
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackU8(U8 val) 
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
#endif
		if (Current() < m_BufferSize)
			*m_pCurrent++ = val;
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}


	// pack S8 into stream
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackS8(S8 val)
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
#endif
		if (Current() < m_BufferSize)
			*m_pCurrent++ = val;
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}


	// pack U16 into stream
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackU16(U16 val) 
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
#endif
		const S32 remaining = m_BufferSize - Current();
		const S32 copySize = sizeof(val);
		if (remaining >= copySize)
		{
			U8* p = reinterpret_cast<U8*> (&val);
			memcpy(m_pCurrent, p, copySize);
			m_pCurrent += copySize;
		}
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}


	// pack S16 into stream
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackS16(S16 val)
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
#endif
		const S32 remaining = m_BufferSize - Current();
		const S32 copySize = sizeof(val);
		if (remaining >= copySize)
		{
			U8* p = reinterpret_cast<U8*> (&val);
			memcpy(m_pCurrent, p, copySize);
			m_pCurrent += copySize;		
		}
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}

	
	// pack U32 into stream
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackU32(U32 val) 
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
#endif
		const S32 remaining = m_BufferSize - Current();
		const S32 copySize = sizeof(val);
		if (remaining >= copySize)
		{
			U8* p = reinterpret_cast<U8*> (&val);
			memcpy(m_pCurrent, p, copySize);
			m_pCurrent += copySize;		
		}
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}


	// pack S32 into stream
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackS32(S32 val)
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
#endif
		const S32 remaining = m_BufferSize - Current();
		const S32 copySize = sizeof(val);
		if (remaining >= copySize)
		{
			U8* p = reinterpret_cast<U8*> (&val);
			memcpy(m_pCurrent, p, copySize);
			m_pCurrent += copySize;		
		}
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}


	// pack U64 into stream
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackU64(U64 val) 
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
#endif
		const S32 remaining = m_BufferSize - Current();
		const S32 copySize = sizeof(val);
		if (remaining >= copySize)
		{
			U8* p = reinterpret_cast<U8*> (&val);
			memcpy(m_pCurrent, p, copySize);
			m_pCurrent += copySize;		
		}
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}


	// pack S64 into stream
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackS64(S64 val)
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
#endif
		const S32 remaining = m_BufferSize - Current();
		const S32 copySize = sizeof(val);
		if (remaining >= copySize)
		{
			U8* p = reinterpret_cast<U8*> (&val);
			memcpy(m_pCurrent, p, copySize);
			m_pCurrent += copySize;		
		}
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}


	// pack F32 into stream
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackF32(F32 val)
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
#endif
		const S32 remaining = m_BufferSize - Current();
		const S32 copySize = sizeof(val);
		if (remaining >= copySize)
		{
			U8* p = reinterpret_cast<U8*> (&val);
			memcpy(m_pCurrent, p, copySize);
			m_pCurrent += copySize;		
		}
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}


	// pack F64 into stream
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackF64(F64 val)
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
#endif
		const S32 remaining = m_BufferSize - Current();
		const S32 copySize = sizeof(val);
		if (remaining >= copySize)
		{
			U8* p = reinterpret_cast<U8*> (&val);
			memcpy(m_pCurrent, p, copySize);
			m_pCurrent += copySize;		
		}
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}


	// add a buffer to the stream.
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::PackBuffer(void* pData, const S32 dataSize)
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
		if (NULL == pData)
			LOGFATAL(g_Log, "Invalid source buffer\n");
		if (dataSize <= 0)
			LOGFATAL(g_Log, "Zero bytes to copy !!\n");
#endif
		const S32 remaining = m_BufferSize - Current();
		if (remaining >= dataSize)
		{
			memcpy(m_pCurrent, pData, dataSize);
			m_pCurrent += dataSize;
		}
		else
			LOGFATAL(g_Log, "Buffer too small\n");
	}


	/// Modify the first 2 bytes in the buffer to contain number of encrypted data bytes. required for Xbox VDP support
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::ModifyEncryptedSize(const U16 size)
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
		if (Current() < sizeof(U16))
			LOGFATAL(g_Log, "Can't pack Encrypted size since there isnt enough data in the buffer !!\n");
		if (m_BufferSize < sizeof(U16))
			LOGFATAL(g_Log, "Can't use C_ByteStream::ModifyEncryptedSize() as internal buffer is too small !!\n");
#endif

		U16* pFirstShort = reinterpret_cast<U16*> (&m_Buffer[0]);
		*pFirstShort = size;
	}


	/// Its possible to remove a set number of bytes from the head of the list
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::RemovePacked(const U16 sizeToRemove)
	{
#ifndef CFG_GOLD
		if (m_pUnpack)
			LOGFATAL(g_Log, "Can't Pack and Unpack at the same time !!\n");
		if (Current() < sizeToRemove)
			LOGFATAL(g_Log, "Trying to remove more bytes than are actually packed. !!\n");
#endif
		if (sizeToRemove == Current())
			m_pCurrent = m_Buffer; // removed all buffer so quick reset
		else
		{
			const S32 bytesToMove = Current() - sizeToRemove;
			memmove(m_Buffer, m_Buffer+sizeToRemove, bytesToMove);
			m_pCurrent = m_Buffer+bytesToMove;
		}
	}

	

	// start the unpack procudure
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::UnPackBegin(void)
	{
		m_pUnpack = m_Buffer; // latest data always starts from beginning of buffer..
	}


	// unpack a u8 val from the stream
	template <U32 bufferSize>
	U8 C_ByteStream<bufferSize>::UnPackU8(void)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
#endif
		return *m_pUnpack++;
	}


	// unpack a s8 val from the stream
	template <U32 bufferSize>
	S8 C_ByteStream<bufferSize>::UnPackS8(void)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
#endif
		return *m_pUnpack++;
	}


	// unpack a u16 val from the stream
	template <U32 bufferSize>
	U16 C_ByteStream<bufferSize>::UnPackU16(void)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
#endif
		U16 val = 0;
		const unpackSize = sizeof(U16);
		memcpy(&val, m_pUnpack, unpackSize);
		m_pUnpack += unpackSize;
		if (m_pUnpack > m_pCurrent)
			LOGFATAL(g_Log, "Unpacked past end of packed data !!\n");
		return val;
	}


	// unpack a s16 val from the stream
	template <U32 bufferSize>
	S16 C_ByteStream<bufferSize>::UnPackS16(void)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
#endif
		S16 val = 0;
		const unpackSize = sizeof(S16);
		memcpy(&val, m_pUnpack, unpackSize);
		m_pUnpack += unpackSize;
		if (m_pUnpack > m_pCurrent)
			LOGFATAL(g_Log, "Unpacked past end of packed data !!\n");
		return val;
	}


	// unpack a u32 val from the stream
	template <U32 bufferSize>
	U32 C_ByteStream<bufferSize>::UnPackU32(void)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
#endif
		U32 val = 0;
		const unpackSize = sizeof(U32);
		memcpy(&val, m_pUnpack, unpackSize);
		m_pUnpack += unpackSize;
		if (m_pUnpack > m_pCurrent)
			LOGFATAL(g_Log, "Unpacked past end of packed data !!\n");
		return val;
	}


	// unpack a s32 val from the stream
	template <U32 bufferSize>
	S32 C_ByteStream<bufferSize>::UnPackS32(void)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
#endif
		S32 val = 0;
		const unpackSize = sizeof(S32);
		memcpy(&val, m_pUnpack, unpackSize);
		m_pUnpack += unpackSize;
		if (m_pUnpack > m_pCurrent)
			LOGFATAL(g_Log, "Unpacked past end of packed data !!\n");
		return val;
	}


	// unpack a u64 val from the stream
	template <U32 bufferSize>
	U64 C_ByteStream<bufferSize>::UnPackU64(void)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
#endif
		U64 val = 0;
		const unpackSize = sizeof(U64);
		memcpy(&val, m_pUnpack, unpackSize);
		m_pUnpack += unpackSize;
		if (m_pUnpack > m_pCurrent)
			LOGFATAL(g_Log, "Unpacked past end of packed data !!\n");
		return val;
	}


	// unpack a s64 val from the stream
	template <U32 bufferSize>
	S64 C_ByteStream<bufferSize>::UnPackS64(void)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
#endif
		S64 val = 0;
		const unpackSize = sizeof(S64);
		memcpy(&val, m_pUnpack, unpackSize);
		m_pUnpack += unpackSize;
		if (m_pUnpack > m_pCurrent)
			LOGFATAL(g_Log, "Unpacked past end of packed data !!\n");
		return val;
	}


	// unpack a F32 val from the stream
	template <U32 bufferSize>
	F32 C_ByteStream<bufferSize>::UnPackF32(void)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
#endif
		F32 val = 0;
		const unpackSize = sizeof(F32);
		memcpy(&val, m_pUnpack, unpackSize);
		m_pUnpack += unpackSize;
		if (m_pUnpack > m_pCurrent)
			LOGFATAL(g_Log, "Unpacked past end of packed data !!\n");
		return val;
	}


	// unpack a F64 val from the stream
	template <U32 bufferSize>
	F64 C_ByteStream<bufferSize>::UnPackF64(void)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
#endif
		F64 val = 0;
		const unpackSize = sizeof(F64);
		memcpy(&val, m_pUnpack, unpackSize);
		m_pUnpack += unpackSize;
		if (m_pUnpack > m_pCurrent)
			LOGFATAL(g_Log, "Unpacked past end of packed data !!\n");
		return val;
	}


	// unpack a set nimber of bytes to the external buffer
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::UnPackBuffer(void* pData, const S32 unpackSize)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
		if (m_pUnpack >= m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
		if (unpackSize <= 0)
			LOGFATAL(g_Log, "Invalid function parameters !!\n");
		if (m_pUnpack+unpackSize > m_pCurrent)
			LOGFATAL(g_Log, "Trying to unpack past end of packed data !!\n");
		if (NULL == pData)
			LOGFATAL(g_Log, "Invalid function parameters !!\n");
#endif

		memcpy(pData, m_pUnpack, unpackSize);
		m_pUnpack += unpackSize;
	}

	// end the unpack procedure
	template <U32 bufferSize>
	void C_ByteStream<bufferSize>::UnPackEnd(const bool refactorBuffer)
	{
#ifndef CFG_GOLD
		if (NULL == m_pUnpack)
			LOGFATAL(g_Log, "Not in Unpack mode !!\n");
#endif
		if (m_pUnpack)
		{
			if (refactorBuffer)
			{
				// need to delete any bytes we unpacked..
				const S32 bytesToRemove = m_pUnpack - m_Buffer;
				if (bytesToRemove > 0)
				{
					if (m_pUnpack == m_pCurrent)
						m_pCurrent = m_Buffer; // unpacked all buffer so quick reset
					else
					{
						const S32 bytesToMove = m_pCurrent - m_pUnpack;
						memmove(m_Buffer, m_pUnpack, bytesToMove);
						m_pCurrent = m_Buffer + bytesToMove;
					}
				}
			}
		}
		m_pUnpack = NULL;
	}


}//end of namespace NetPlay

#endif
