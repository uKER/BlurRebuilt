#ifndef _FILESTREAMMANAGER_H_
#define _FILESTREAMMANAGER_H_


#include "..\common\thread.h"
#include "btl/containers/vector.h"
#include "../AppInterfaceLayer/PagedArray.h"

#include <System/Serialise/I_File.h>


#define MAX_NUM_STREAMING_FILES			128

namespace Audio
{
    namespace Core
    {

		// forward decl
		class FileStreamManager;

		class FileStreamPlaybackBufferArray
		{
		public:
		    
			FileStreamPlaybackBufferArray()
				: m_Entries( 0 )
			{
				for ( U32 i=0; i<m_MaxEntries; i++ )
				{
					m_PtrArray[i] = NULL;
				}
			}
		    
			~FileStreamPlaybackBufferArray()
			{
			}
		    
			void* Alloc()
			{
				if ( m_Entries )
				{
					void* ptr = m_PtrArray[ --m_Entries ];
					if ( !ptr ) DebugBreak();
					return ptr;
				}
				return NULL;
			}

			void Free( void* ptr )
			{
				if ( !ptr ) DebugBreak();
				if ( m_Entries < m_MaxEntries )
				{
					m_PtrArray[ m_Entries++ ] = ptr;
				}
				else
				{
					AUDIO_CORE_FATAL("FileStreamPlaybackBufferArray::Free() : m_MaxEntries needs to be increased! [m_MaxEntries = %d]\n", m_MaxEntries );
				}
			}

		private:
		    
			static const U32    m_MaxEntries = 128;
			U32                 m_Entries;
			void*               m_PtrArray[ m_MaxEntries ];
		};

		class FileStreamPlaybackBuffer
		{
		public:
			FileStreamPlaybackBuffer(U32 u32MemorySize);
			~FileStreamPlaybackBuffer();
		    
			static void Init(U32 u32MemorySize = 0);

			void* Alloc( const U32 input_size, const U32 uAlignment=(U32)-1, const Audio::ISystem::AllocType eType=(Audio::ISystem::AllocType)-1 );
			void Free( void* ptr, const U32 input_size, const Audio::ISystem::AllocType eType=(Audio::ISystem::AllocType)-1 );
		    
			static FileStreamPlaybackBuffer* TheBuffer() { return m_pFileStreamPlaybackBuffer; }

#ifdef AUDIO_STREAMER_DEBUG_INFO
			void DebugInfo();
#endif

		private:
			static FileStreamPlaybackBuffer*	m_pFileStreamPlaybackBuffer;
		    
			static const U32			m_MaxEntries = 32;
			U32							m_Entries;
			S32							m_Size[ m_MaxEntries ];
			FileStreamPlaybackBufferArray		m_Array[ m_MaxEntries ];
		    
			U32							m_MemorySize;
			U32							m_MemoryFree;
			U32							m_u32NumBuffers;
			void*						m_pMemoryBase;
			char*						m_pMemoryFree;
		};

		enum StreamManagerDebugLevel
		{
			eExtendedInfo = 0,
			eStandardInfo,
			eWarning
		};

////////////////////////////////////////////////////////////////////////////////
//	FileStreamerDebugRenderer
////////////////////////////////////////////////////////////////////////////////
class FileStreamerDebugRenderer
{

public:
	FileStreamerDebugRenderer();

	void BeginRender();
	void DrawOutlineBox(F32 f32xPos, F32 f32YPos, F32 f32XSize, F32 f32YSize, F32 f32OutlineWidth, U32 f32FillColour, U32 f32OutlineColour);
	void PrintStreamInfoString(F32 f32XOffset, bool bHighlighted, char* strText);
	void DrawStreamInfoBox(U32 u32Size, U32 u32BytesRead, U32 u32BytesWritten, U64 u64TotalBytesWritten, U64 u64TotalBytesRead, U32 u32BufferSize, U32 u32ItemIndex, U32 u32StreamFlags, const Btl::String& name, U32 format);

private:
	F32 f32XStartPos;
	F32 f32YStartPos;

	F32 xPos;
	F32 yPos;
	F32 xSize;
	F32 ySize;
	F32 f32MeterWidth;
	F32 f32MeterHeight;
	F32 f32SecondMeterYOffset;
	F32 f32TextYOffset;
	F32 f32EOF_READ;
	F32 f32FINISHED;
	F32 f32UNDERRUN;
	F32 f32RESET_REQUIRED;
	F32 f32RELEASING;
	F32 f32RELEASED;
	F32 f32DESTROYING;
	F32 f32BUFFERS_ALLOCATED;
	F32 f32RELEASE_REQUIRED;
	F32 f32DATA_NOT_READY;
	F32 f32YET_TO_UPDATE;
	F32 f32TextSize;
};

////////////////////////////////////////////////////////////////////////////////
//	FileStreamManager
////////////////////////////////////////////////////////////////////////////////
class FileStreamManager : public WaveServerComponent
        {
			friend class FileStream;

        public:
			FileStreamManager(IWaveServer* iWaveServer);
			~FileStreamManager();

			AudioStreamFileHandle		PrepareFile(const char* strFilename);
			FS_STREAM_HANDLE			CreateStream(FileStreamDef& streamDef);
			bool						DestroyStream(FS_STREAM_HANDLE hStream);			// flags the stream for destruction
			bool						GetHeader(AudioStreamFileHandle hFile, U32 u32HeaderSize, void* pDest);
			bool						AllocStreamBuffers(FS_STREAM_HANDLE hStream);
			void						FreeStreamBuffers(FS_STREAM_HANDLE hStream);
			bool						PrepareStream(FS_STREAM_HANDLE hStream);			// prepare stream for use
			bool						ReleaseStream(FS_STREAM_HANDLE hStream);			// frees any buffers and unprepares stream. does not destroy it
			bool						ResetStream(FS_STREAM_HANDLE hStream);
			bool						IsStreamFinishedReading(FS_STREAM_HANDLE hStream);
			bool						IsStreamFinishedWriting(FS_STREAM_HANDLE hStream);
			bool						IsStreamPrepared(FS_STREAM_HANDLE hStream);
			bool						IsStreamPrecached(FS_STREAM_HANDLE hStream);
			bool						IsStreamReleasing(FS_STREAM_HANDLE hStream);
			bool						IsStreamResetting(FS_STREAM_HANDLE hStream);
			bool						IsStreamAwaitingDestruction(FS_STREAM_HANDLE hStream);
			bool						AreStreamBuffersAllocated(FS_STREAM_HANDLE hStream);
			bool						IsStreamDataReady(FS_STREAM_HANDLE hStream);
			bool						GetData(WaveServerHandle hStream, CircularBuffer* pCircularBuffer);
			bool						SetFilePosition(FS_STREAM_HANDLE hStream, U32 u32PositionInBytes);
			U32							GetFilePosition(FS_STREAM_HANDLE hStream);
			U32							GetPrevSeekPos(FS_STREAM_HANDLE hStream);
			S32							GetNumBytesDataReady(FS_STREAM_HANDLE hStream);
			bool						ReleaseFile(AudioStreamFileHandle hFile);
			bool						DoFileRelease(AudioStreamFileHandle hFile);
			void						Update();
			void						Render();
			U32							GetStreamPosition(FS_STREAM_HANDLE hStream);
			void						ReadRequestProcessed(WaveServerHandle hStream, U32 u32NumBytesRead, U32 u32RequestId);
			void						SetDebugLevel(StreamManagerDebugLevel eLevel) { m_eDebugLevel = eLevel; }
			void						InitStreamBufferAllocator() { FileStreamPlaybackBuffer::Init(); }
			#ifdef ENABLE_DEBUG_EXTENSIONS
			void						DumpStreamInfo();
			#endif
		private:
			inline FileStream*			GetStream(FS_STREAM_HANDLE hStream) { return m_apStream.GetByHandle(hStream); }
			inline FileStream*			GetStream(WaveServerHandle hStream) { return GetStream(FS_STREAM_HANDLE(hStream & 0x00ffffff)); }
			bool						DoStreamDestroy(FS_STREAM_HANDLE hStream);			// deletes the stream object
			FileStreamFileWrapper*		GetFileWrapper(AudioStreamFileHandle hFile) { return m_apFile[hFile]; }
			void*						operator new(size_t sz);
			void						operator delete(void* ptr);
			void						ExtendedInfo(const char* strInfo, ...);
			void						Info(const char* strInfo, ...);
			void						Warning(const char* strInfo, ...);

			PagedArray<FileStream*>		m_apStream;
			Btl::Vector<FS_STREAM_HANDLE>	m_vStreamsToUpdate;
			FileStreamFileWrapper*		m_apFile[MAX_NUM_STREAMING_FILES];
			StreamManagerDebugLevel		m_eDebugLevel;
			Lock						m_lockObject;
#ifndef CFG_SPEED
			FileStreamerDebugRenderer	m_DebugRenderer;
#endif
		};
	}
}


#endif // _FILESTREAMMANAGER_H_
