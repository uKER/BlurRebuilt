#ifndef _WAVE_SERVER_H_
#define _WAVE_SERVER_H_


#define AUDIO_FILE_STREAMER_QUEUE_SIZE	1024
#define	MAX_MP3_REQUESTS_TO_PROCESS		8


#include "../Common/CircularBuffer.h"


namespace Audio
{

	namespace Core
	{

		class WaveServer;

////////////////////////////////////////////////////////////////////////////////
//	WaveStreamReadResult
////////////////////////////////////////////////////////////////////////////////
		struct WaveStreamReadResult
		{
			U32						m_u32NumBytesRead;
			WaveServerHandle		m_hStream;
			U32						m_u32RequestId;
			bool					m_bLastReadSuccessful;
			U8						m_u8ResultType;
		};

////////////////////////////////////////////////////////////////////////////////
//	WaveStreamLoadingThread
////////////////////////////////////////////////////////////////////////////////
		class WaveStreamLoadingThread : public Thread
		{

		public:
			WaveStreamLoadingThread(U32 u32StackSize, WaveServer* pWaveServer);
			void					Execute(void* pVoid);
			bool					SendRequest(WaveStreamReadRequest& request);
			bool					GetResult(WaveStreamReadResult& result);

		private:
			WaveStreamReadRequest	m_aStreamReadRequest[AUDIO_FILE_STREAMER_QUEUE_SIZE];
			WaveStreamReadResult	m_aStreamReadResult[AUDIO_FILE_STREAMER_QUEUE_SIZE];
			WaveServer*				m_pWaveServer;
			volatile S32			m_s32RequestQueueReadPointer;
			volatile S32			m_s32RequestQueueWritePointer;
			volatile S32			m_s32ResultQueueReadPointer;
			volatile S32			m_s32ResultQueueWritePointer;
			Lock					m_RequestLock;
			Lock					m_ResultLock;
		};

		//template<class T, U32 uSize, U32 uAlign, bool bPhysAlloc>
		//class CircularBuffer
		//{
		//public:
		//	T* GetReadPtr() { return NULL; }
		//	U32 GetReadSpace() { return 0; }
		//	U32 GetReadAvailable() { return 0; }
		//	U32 IncRead() { return 0; }

		//	T* GetWritePtr() { return NULL; }
		//	U32 GetWriteSpace() { return 0; }
		//	U32 GetWriteAvailable() { return 0; }
		//	U32 IncRead() { return 0; }

		//	void Relocate(T* pAddr) {}
		//	void MoveToSPU(T* pAddr) {}
		//	void Restore() {}

		//private:
		//	T* m_pBuffer;

		//	U32 m_uRead;
		//	U32 m_uWrite;
		//}

		//struct CircularBufferData
		//{
		//	U32			m_u32ReadOffset;
		//	U32			m_u32BytesAvailable;
		//	U32			m_u32BufferSize;
		//	U8*			m_pBuffer;
		//}

		enum WaveStreamState
		{
			eActive = 0,
			eStreamFinished,
			eUnderrun,
			eDormant
		};

		struct StreamConsumptionInfo
		{
			void					Init() { m_u32BytesConsumed = 0; m_u32SourceBufferPositionRequired = 0; m_bInUse = false; }

			U32						m_u32BytesConsumed;
			U32						m_u32SourceBufferPositionRequired;
			bool					m_bInUse;
		};

		//class ICircularBuffer
		//{
		//	virtual 
		//};

		class WaveStreamNode
		{
			friend class WaveServer;

		public:
			void					Init();
			inline bool				IsInUse() { return (m_pOutputCircularBuffer != 0); }
			inline CircularBuffer*	GetBuffer() { if (! m_pOutputCircularBuffer) CreateBuffer(); return m_pOutputCircularBuffer; }
			void					Update();	// check if circular buffer has space, if so, attempt to fill
			bool					BecomeActive();
			bool					BecomeInactive(); 
			bool					HasFinished() { return (m_eState == eStreamFinished); }
			bool					Destroy();
			inline void				CreateBuffer() { m_pOutputCircularBuffer = new CircularBuffer(1024 * 8); }

		private:
			WaveServerHandle		m_hProcess;
//			WaveStream*				m_pInputStream;
			CircularBuffer*			m_pOutputCircularBuffer;
//			StreamConsumptionInfo	m_StreamConsumptionInfo;
			WaveStreamState			m_eState;
		};


		class StreamSource
		{

		public:

			AudioStreamFileHandle	m_fsFileHandle;
		};


		class AudioGraph
		{

			friend class WaveServer;

		private:
			AudioGraph();
			~AudioGraph() { Destroy(); }

			bool					Create(WaveServerStreamCreateParams& params);
			bool					Destroy();
			U8*						GetBuffer();
			bool					CreateOutputBuffer();
			bool					DestroyOutputBuffer();
			inline U32				GetBufferSize() { return m_u32BufferSize; }
			inline U32				GetBytesAvailable() { return m_u32BytesAvailable; }
			inline U32				GetTotalBytesConsumed() { return m_u32TotalBytesConsumed; }
			void					ConsumeData(U32 u32BytesConsumed);
			bool					BecomeActive();
			bool					BecomeInactive();
			void					Update();
			bool					IsPrimed() { return m_bActive; }
			bool					HasFinished() { if (m_StreamNodes.Size()) return m_StreamNodes.At(0).HasFinished(); return true; }
			inline void				SetNextActive(AudioGraph* pNext) { m_pNextActive = pNext; }
			inline void				SetPrevActive(AudioGraph* pPrev) { m_pPrevActive = pPrev; }
			inline AudioGraph*		GetNextActive() { return m_pNextActive; }
			inline AudioGraph*		GetPrevActive() { return m_pPrevActive; }
			void					RemoveFromActiveList() { if (m_pPrevActive) m_pPrevActive->SetNextActive(m_pNextActive); if (m_pNextActive) m_pNextActive->SetPrevActive(m_pPrevActive); m_pPrevActive = 0; m_pNextActive = 0; }

			Btl::Vector<WaveStreamNode>	m_StreamNodes;
			// TODO: buffer needs to be 16 byte aligned
//			WaveStreamData			m_WaveStreamData;
			U32						m_u32BufferSize;
			U32						m_u32BufferAlignment;
			U32						m_u32BytesAvailable;
			U32						m_u32TotalBytesConsumed;
			U8*						m_pData;
			AudioGraph*				m_pNextActive;
			AudioGraph*				m_pPrevActive;
			bool					m_bPhysicalMemoryBuffer;
			bool					m_bActive;	// could ditch this, just check whether buffer in first audio node exists instead?
		};


		class WaveServer : public IWaveServer
		{

			friend class WaveStreamLoadingThread;
			friend class WaveStreamNode;

		public:
#ifdef PLATFORM_PS3
			WaveServer();
#else
			WaveServer();
#endif
			bool						CreateStream(StreamHandle& hStream, WaveServerStreamCreateParams& params);
			bool						DestroyStream(StreamHandle hStream);
			bool						AllocateResources(StreamHandle hStream);
			bool						FlushResources(StreamHandle hStream);
			bool						RegisterStreamSource(StreamSourceHandle& hSource, WaveServerRegisterSourceParams& params);
			bool						DeregisterStreamSource(StreamSourceHandle hSource);
//			S16*						GetBuffer(StreamHandle hStream);
			void						Update();
			bool						UpdateStream(WaveStreamData& waveStreamData, U32* pu32WavesNeededUpdate);
			bool						AllocateStreamProcessResources(WaveServerHandle hWave);
			bool						DeallocateStreamProcessResources(WaveServerHandle hWave);
			bool						DestroyWaveServerProcess(WaveServerHandle hWave);
			
			FileStreamManager&			GetFileStreamManager() { return m_FileStreamManager; }

		private:
			bool						SendRequest(WaveStreamReadRequest& request) { return m_LoadThread.SendRequest(request); }
			bool						GetData(WaveServerHandle hWave, CircularBuffer* pDestCircularBuffer);
			StreamSource*				GetSource(StreamSourceHandle& hSource) { return m_apSource[hSource]; }
			WaveServerHandle			GetWaveServerHandle(U32 u32SystemHandle, U32 u32SystemId);
			inline AudioGraph*			GetGraph(StreamHandle hStream) { return m_apGraph.GetByHandle(hStream); }
			bool						BecomeActive(AudioGraph* pGraph);
			bool						BecomeInactive(AudioGraph* pGraph);
			void						AddToActiveList(AudioGraph* pGraph);
			void						RemoveFromActiveList(AudioGraph* pGraph);

			PagedArray<AudioGraph*>		m_apGraph;
			Btl::Vector<StreamSource*>	m_apSource;
			U32							m_u32StreamsInUse;
			U32							m_u32SourcesInUse;
			AudioGraph*					m_pFirstActive;
			AudioGraph*					m_pLastActive;
			WaveStreamLoadingThread		m_LoadThread;
//#ifdef PLATFORM_PS3
//			MP3DecodeManager			m_MP3Manager;
//#endif
			FileStreamManager			m_FileStreamManager;
		};
	}
}

#endif	//	_WAVE_SERVER_H_
