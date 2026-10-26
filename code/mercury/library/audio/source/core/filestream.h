#ifndef _FILESTREAM_H_
#define _FILESTREAM_H_


//#define AudioStreamFileHandle					S32
typedef Audio::GenericStandaloneAudioS32Handle	AudioStreamFileHandle;

#include <System/Serialise/I_File.h>

#define MAX_NUM_DATA_CHUNKS		4

namespace Audio
{
    namespace Core
    {

		// forward decls
		class FileStreamManager;


////////////////////////////////////////////////////////////////////////////////
//	FileStreamDef
////////////////////////////////////////////////////////////////////////////////
		class FileStreamDef
		{
			friend class WaveServer;
			friend class FileStream;
			friend class FileStreamManager;

			U32					m_u32DataProcessMaxChunkSize;	// size of return buffers. maximum number of bytes GetData() can return
			U32					m_u32FileReadChunkSize;			// number of bytes we read from disk in one file read
			U32					m_u32NumInitialWriteChunks;
			U32					m_u32BufferSizeInReadChunks;	// number of chunks of size m_u32FileReadChunkSize we've got for holding data
			U32					m_u32FileStartPosition;
			U32					m_u32FileEndPosition;
			AudioStreamFileHandle		m_hFile;
			U32					m_u32HeaderSkipSize;
			U32					m_u32Format;
			bool				m_bReturnBufferPhysicalMemory;
			bool				m_bLoop;
			bool				m_bPrecache;
			bool				m_bTempAmaxNullFileReadHack;

			#ifndef CFG_SPEED
			Btl::String			m_strBankFilename;
			Btl::String			m_strWaveName;
			#endif
		};

////////////////////////////////////////////////////////////////////////////////
//	FileStreamDataChunk
////////////////////////////////////////////////////////////////////////////////
		class FileStreamDataChunk
		{

		public:
			FileStreamDataChunk() { m_pDataChunk = 0; Init(); }
			~FileStreamDataChunk() { }

			void				Init() { m_u32BytesAvailable = 0; m_bEndOfStream = false; m_bAllDataConsumed = true; m_u32PrevOffset = 0; }
			void*				GetData(U32 u32Offset, U32& u32BytesAvailable);
			inline bool			IsDataAvailable() { return (m_u32BytesAvailable > 0); }
			bool				SetBytesConsumed(U32 u32BytesConsumed);
			bool				SetBytesAvailable(U32 u32BytesAvailable);
			void				SetEndOfStreamReached() { m_bEndOfStream = true; }
			bool				IsEndOfStreamReached() { return m_bEndOfStream; }
			inline bool			IsWaitingForNewData() { return m_bAllDataConsumed; }
			bool				SetDataChunk(void* pData) { if (m_pDataChunk) {AUDIO_CORE_ERROR("Trying to set data in an audio stream data chunk that already contains a buffer"); return false; } m_pDataChunk = pData; return true; }
			void*				GetBufferForWriting() { if (m_bAllDataConsumed) return m_pDataChunk; return 0; }
			inline void*		GetBufferAndRelease() { void* pRet = m_pDataChunk; m_pDataChunk = 0; return pRet; }
			inline bool			IsBufferAllocated() { return (m_pDataChunk != 0); }

		private:
			void*				m_pDataChunk;
			U32					m_u32BytesAvailable;
			U32					m_u32PrevOffset;
			bool				m_bEndOfStream;
			bool				m_bAllDataConsumed;
		};

////////////////////////////////////////////////////////////////////////////////
//	FileStream
////////////////////////////////////////////////////////////////////////////////
        class FileStream
        {

			friend class FileStreamManager;

        private:
			FileStream();
			~FileStream();

			struct FileStreamState
			{
				U32				m_u32CurrentChunkReadPointer;
				U32				m_u32CurrReadChunk;
				U32				m_u32NextWriteChunk;
				U32				m_u32HasBeenWrittenChunk;
			};

			// need to be able to get the amount of bytes we've got available

			void				InitState();
			bool				Setup(FileStreamDef& streamDef);
			bool				AllocBuffers();
			void				FreeBuffers();
			bool				Prepare();
			bool				FlagForRelease();
			bool				FlagForDestruction();
			bool				FlagForReset();
			bool				Release();
			bool				Reset();
			bool				GetData(CircularBuffer* pCircularBuffer);
			bool				FillNextChunkIfPossible();
			bool				SubmitLoadRequest(U32 u32NumChunksToRead, U32 u32HeaderSkip = 0);
			void				ReleaseBuffer(void* pBuffer);
			bool				LoadRequestSubmitted() { return m_bLoadRequestSubmitted; }
			bool				QueryStateFlag(U32 u32Flag) { return ((u32Flag & m_u32StateFlags) > 0); }
			U32					GetPosition();
			U32					GetPrevSeekPos() { return m_u32PrevSeekPos; }
			bool				SetFilePosition(U32 u32Position);
			AudioStreamFileHandle		GetFileHandle() { return m_FileStreamDef.m_hFile; }

		private:
			void				SetManager(FileStreamManager* pManager) { m_pManager = pManager; }
			void				SetStreamHandle(FS_STREAM_HANDLE hStream) { m_hStream = hStream; }
			void				ReadRequestProcessed(U32 u32BytesWritten, U32 u32RequestId);
			inline U32			GetBufferSize() { return m_FileStreamDef.m_u32BufferSizeInReadChunks * m_FileStreamDef.m_u32FileReadChunkSize; }
			inline U32			GetStreamTotalDataSize() { return m_FileStreamDef.m_u32FileEndPosition - m_FileStreamDef.m_u32FileStartPosition; }
			inline S32			GetNumBytesDataReady() { assert (m_u64TotalBytesWritten >= m_u64TotalBytesRead); return (S32)(m_u64TotalBytesWritten - m_u64TotalBytesRead);  }
			inline U64			GetTotalBytesWritten() { return m_u64TotalBytesWritten; }
			inline U32			GetBytesWrittenPosInStream() { return (U32)(m_u64TotalBytesWritten % (U64)(GetStreamTotalDataSize())); }
			inline U32			GetBytesReadPosInStream() { return (U32)(m_u64TotalBytesRead % (U64)(GetStreamTotalDataSize())); }// just for debug info atm
			inline U64			GetTotalBytesRead() { return m_u64TotalBytesRead ; }// just for debug info atm
			void				SetStateFlagToTrue(U32 u32Flag) { m_u32StateFlags |= u32Flag; }
			void				SetStateFlagToFalse(U32 u32Flag) { m_u32StateFlags &= ~u32Flag; }
			void				SetStateFlags(U32 u32Flags) { m_u32StateFlags = u32Flags; }
			bool				ShouldStreamPrecache() { return m_FileStreamDef.m_bPrecache; }
			void*				GetCurrentReadChunkData(U32& u32BytesAvailable);
			bool				SetCurrentReadChunkBytesConsumed(U32 u32BytesConsumed);
			bool				IsDataAvailableInCurrentReadChunk();
			void				SetCurrentWriteChunkContainsStreamEnd() { GetHasBeenWrittenChunk().SetEndOfStreamReached(); }
			bool				DoesCurrentReadChunkContainStreamEnd();
			FileStreamDataChunk*	GetCurrentReadChunk();
			
			inline U32			GetChunksToEndOfBuffer() const
			{
				return m_FileStreamDef.m_u32BufferSizeInReadChunks - GetNextWriteChunkIndex();
			}

			FileStreamDataChunk&	GetNextWriteChunk();
			FileStreamDataChunk&	GetHasBeenWrittenChunk() { return m_aDataChunk[GetHasBeenWrittenChunkIndex()]; }
			inline U32			GetCurrentReadChunkIndex() const { return m_FileStreamState.m_u32CurrReadChunk % m_FileStreamDef.m_u32BufferSizeInReadChunks; }
			inline U32			GetNextWriteChunkIndex() const { return m_FileStreamState.m_u32NextWriteChunk % m_FileStreamDef.m_u32BufferSizeInReadChunks; }
			inline U32			GetHasBeenWrittenChunkIndex() const { return m_FileStreamState.m_u32HasBeenWrittenChunk % m_FileStreamDef.m_u32BufferSizeInReadChunks; }


			Btl::Vector<FileStreamDataChunk, Btl::VectorParameters_Array<MAX_NUM_DATA_CHUNKS > > m_aDataChunk;

			FileStreamDef		m_FileStreamDef;
			FileStreamState		m_FileStreamState;
			U32					m_u32StateFlags;
			FileStreamManager*	m_pManager;
			FS_STREAM_HANDLE	m_hStream;
			U64					m_u64TotalBytesWritten;
			U64					m_u64TotalBytesRead;
			U32					m_u32PrevRequestedNumBytes;
			U32					m_u32PrevSeekPos;
			U32					m_u32LastIssuedRequestId;
			bool				m_bLoadRequestSubmitted;
			bool				m_bEndianSwap;
		};
	}
}


#endif // _FILESTREAM_H_
