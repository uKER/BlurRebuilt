#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_


#include "Lock.h"


namespace Audio
{

	class CircularBuffer
	{

	public:
		CircularBuffer() { Init(0); }
		CircularBuffer(U32 u32BufferSize);
		~CircularBuffer();

		void					Init(U32 u32BufferSize);
		U8*						GetBuffer() { return m_pu8Buffer; }
		inline U32				GetBufferSize() { return m_u32BufferSize; }
		bool					GetData(U8** ppData, U32 u32BytesRequired, U32* pu32BytesAvailable);
		bool					WriteData(U8* pSourceData, U32 u32BytesAvailable, U32* pu32BytesWritten);
		U32						GetBytesAvailable();
		inline U32				GetReadOffsetDistanceTilBufferEnd() { return m_u32BufferSize - m_u32ReadOffset; }
		inline U32				GetWriteOffsetDistanceTilBufferEnd() { return m_u32BufferSize - m_u32WriteOffset; }
		U32						GetReadOffset() { return m_u32ReadOffset; }
		U32						GetWriteOffset() { return  m_u32WriteOffset; }
		inline bool				HasSpace() { return (GetBytesAvailable() != m_u32BufferSize); }
		inline U32				GetSpaceInBytes() { return m_u32BufferSize - GetBytesAvailable(); }
		void					SetStarving(bool bStarving) { m_bStarving = true; }
		void					SetEndOfStreamReached() { m_bEndOfStreamReached = true; }
		bool					HasReachedEndOfStream() { return m_bEndOfStreamReached; }
		bool					BecomeActive();
		bool					BecomeInactive(); 
		inline bool				IsActive() { return m_pu8Buffer != 0; }


	private:
		void					IncrementReadOffset(U32 u32NumBytes);
		void					IncrementWriteOffset(U32 u32NumBytes);
#ifdef PLATFORM_PS3
		inline U32				min(U32 a, U32 b) { return (a < b) ? a : b; }
#endif

		U32						m_u32BufferSize;
		U8*						m_pu8Buffer;
		Core::ThreadSafeIndex	m_u32ReadOffset;
		Core::ThreadSafeIndex	m_u32WriteOffset;
		Core::ThreadSafeIndex	m_u32BytesAvailable;
		bool					m_bEndOfStreamReached;
		bool					m_bStarving;
	};

}

#endif	//	_CIRCULAR_BUFFER_H_
