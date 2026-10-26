#ifndef _MP3_DECODE_H_
#define _MP3_DECODE_H_


#include "mp3declib.h"
#include "../Common/Handle.h"
#include "../Common/CircularBuffer.h"

#define	SIZEOF_MP3_UNCONSUMED_DATA_BUFFERS	1024 * 4
#define	ENCODE_BUFFER_SIZE_BYTES			1024 * 2	// min
#define	DECODE_BUFFER_SIZE_SAMPLES			1024 * 2	// min


namespace Audio
{

	typedef Handle	MP3Handle;
	//class MP3Handle
	//{

	//public:
	//	MP3Handle() { }
	//	MP3Handle(U32 u32Handle) { h = u32Handle; }

	//	inline void operator =(const U16 value) { h = value; }
	//	inline bool IsValid() { return h.IsValid(); }
	//	inline void SetAsInvalid() { h.SetAsInvalid(); }

	//	inline U32 Value() { return h.Value(); }

	//	Handle h;
	//};


	namespace Core
	{

		class MP3DecodeManager;

		class MP3Stream
		{

			friend class MP3DecodeManager;

		public:
			MP3Stream();
			~MP3Stream();

			bool							Init(U8* pMP3Data, U32 u32DataSizeBytes, U8 u8NumChannels, bool bLoop, MP3DecodeManager* pManager);
			bool							AllocateResources();
			bool							DeallocateResources();
			inline bool						AreResourcesAllocated() { return m_bResourcesAllocated; }
			bool							SendDataRequest(U32 u32SamplesRequired);
//			bool							RetrieveData(S16** ppDestBufferL, S16** ppDestBufferR);
			void							Update();
			inline U32						GetNumSamplesStillNeeded() { return m_u32SamplesStillNeeded; }
			inline U32						GetNumSamplesRequested() { return m_u32OriginalNumSamplesRequested; }
			bool							HasFinished() { return (m_u32EncodedDataOffset >= m_u32EncodedDataTotalSizeBytes); }
			inline bool						IsDataReady() { return m_bRequestedDataReady; }
			inline bool						WasLastReadSuccessful() { return (m_bDecodingBegun && m_bPrevReadSuccessful); }
			inline void						SetNext(MP3Stream* pStream) { m_pNextUpdater = pStream; }
			inline void						SetPrev(MP3Stream* pStream) { m_pPrevUpdater = pStream; }
			inline void						SetNextDataMoveItem(MP3Stream* pStream) { m_pNextDataMoveItem = pStream; }
			inline void						SetPrevDataMoveItem(MP3Stream* pStream) { m_pPrevDataMoveItem = pStream; }
			inline MP3Stream*				GetNext() { return m_pNextUpdater; }
			inline MP3Stream*				GetPrev() { return m_pPrevUpdater; }
			inline MP3Stream*				GetNextDataMoveItem() { return m_pNextDataMoveItem; }
			inline MP3Stream*				GetPrevDataMoveItem() { return m_pPrevDataMoveItem; }
			bool							GetData(S16* ps16Data, U32 u32BytesRequired, U32* pu32BytesWritten);
			bool							FillOutputBuffer(CircularBuffer* pDestCircularBuffer);
			inline bool						NeedsData() { return ((! GetNumSamplesAvailable()) && (! m_u32NumSamplesInDecodedBuffer)); }
			inline bool						DoesDecodedDataNeedMoving() { return (m_u32NumSamplesInDecodedBuffer); }
			inline bool						IsInDataMoveList() { return m_bIsInDataMoveList; }
			bool							IsStreamUseable();
			void							SetNotUseable() { m_bUseable = false; }
			bool							IsDeallocatePending() { return m_bDeallocatePending; }
			void							SetDeallocatePending() { m_bDeallocatePending = true; }
			void							UnsetWaveServerRequestState() { m_bWaveServerUpdateRequestSubmitted = false; }

		private:
			U32								GetNumSamplesAvailable() { return m_UnconsumedDecodedData.GetBytesAvailable() / sizeof(S16); }
//			bool							CopyDecodedDataIntoBuffers();
			bool							SendRequestToDecoder();
			inline U32						GetEncodedBytesAvailable() { return m_u32EncodedDataTotalSizeBytes - m_u32EncodedDataOffset; }
			void							WriteDecodedDataIntoCircularBuffer();

			U8*								m_pMP3Data;
			U8*								m_pu8SrcBuffer;
			S16*							m_s16TempDestBuffer;
//			S16*							m_s16TempDestBufferR;
			U32								m_u32EncodedDataTotalSizeBytes;
			U32								m_u32EncodedDataOffset;
			CellMP3DecoderStreamInstance*	m_pDecoderStreamInstance;
			CellMP3Context*					m_pContext1;
			CellMP3Context*					m_pContext2;
//			S16*							m_ps16UnconsumedDecodedDataL;
//			S16*							m_ps16UnconsumedDecodedDataR;
			CircularBuffer					m_UnconsumedDecodedData;
			MP3DecodeManager*				m_pManager;
			U32								m_u32NumSamplesInDecodedBuffer;
			U32								m_u32SamplesStillNeeded;
			U32								m_u32OriginalNumSamplesRequested;
			//S16*							m_pFinalDestBufferL;
			//S16*							m_pFinalDestBufferR;
			MP3Stream*						m_pNextUpdater;
			MP3Stream*						m_pPrevUpdater;
			MP3Stream*						m_pNextDataMoveItem;
			MP3Stream*						m_pPrevDataMoveItem;
			U8								m_u8NumChannels;
			volatile bool					m_bUseable;
			volatile bool					m_bDeallocatePending;
			volatile bool					m_bDecodeRequestSubmitted;
			volatile bool					m_bWaveServerUpdateRequestSubmitted;
			bool							m_bRequestedDataReady;
			bool							m_bDecodingBegun;
			bool							m_bPrevReadSuccessful;
			bool							m_bIsInDataMoveList;
			bool							m_bResourcesAllocated;
			bool							m_bLoop;
			bool							m_bInUpdateList;
		};


		class MP3DecodeManager : public WaveServerComponent
		{

		public:
//			MP3DecodeManager();
			MP3DecodeManager(IWaveServer* iWaveServer) : WaveServerComponent(iWaveServer)
			{
				m_pSpurs = 0;
				m_pMP3DecoderMem = 0;
				m_u32NumStreamsRequiringUpdate = 0;
				m_pFirst = 0;
				m_pLast = 0;

				Init();
			}
			~MP3DecodeManager();

			bool							Init();
			MP3Handle						CreateMP3Stream(U8* pMP3Data, U32 u32DataSize, U8 u8NumChannels, bool bLoop = false);
			bool							DestroyMP3Stream(MP3Handle hStream);
			bool							AllocateStreamResources(MP3Handle hStream);
			bool							DeallocateStreamResources(WaveServerHandle hStream);
			bool							AreStreamResourcesAllocated(MP3Handle hStream);
			inline U8*						GetDecoderMemory() { return m_pMP3DecoderMem; }
			bool							SendRequestToMP3Decoder(MP3Handle hStream, U32 u32SamplesRequired);
			bool							SendDecodeRequestToWaveServerThread(WaveServerHandle hStream, U32 u32SamplesRequired);
//			bool							RetrieveData(MP3Handle hStream, S16** ppDestBufferL = 0, S16** ppDestBufferR = 0);
			bool							GetData(WaveServerHandle hWave, CircularBuffer* pDestCircularBuffer);
			bool							RequestFillBuffers(WaveServerHandle hWave);
			bool							HasFinished(MP3Handle hStream);
			void							Update();
			U32								GetNumSamplesRequested(MP3Handle hStream);
			bool							WasLastDecodeSuccessful(MP3Handle hStream);
			void							ReadRequestProcessed(WaveServerHandle hStream, U32 u32NumBytesRead);
			bool							IsStreamUseable(MP3Handle hStream);

		private:
			bool							DeallocateStreamResourcesImmediate(MP3Handle hStream);
			void							AddToUpdateList(MP3Stream* pStream);
			void							RemoveFromUpdateList(MP3Stream* pStream);
			void							AddToDataMoveList(MP3Stream* pStream);
			void							RemoveFromDataMoveList(MP3Stream* pStream);

			HandleCollection<MP3Handle, 0>	m_StreamHandles;
			CellSpurs*						m_pSpurs;
			U32								m_u32NumStreamsRequiringUpdate;
			MP3Stream*						m_pFirst;
			MP3Stream*						m_pLast;
			MP3Stream*						m_pFirstDataMoveItem;
			MP3Stream*						m_pLastDataMoveItem;
			U8*								m_pMP3DecoderMem;

			friend class WaveStreamLoadingThread;

		};

	}
}

#endif	//	_MP3_DECODE_H_
