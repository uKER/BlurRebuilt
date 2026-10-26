#ifndef _DECODERADPCMF16_H_
#define _DECODERADPCMF16_H_


//#ifdef __STDC__
//#define ARGS(x) x
//#else
//#define ARGS(x) ()
//#endif

#define USE_NEW_ADPCM

#ifdef USE_NEW_ADPCM
#define BATCH_SIZE  64
#endif

class cDecodeVagWorkArea
{
public:
	cDecodeVagWorkArea()
	{
//		ClearPrevious();
	}
	void ClearPrevious()
	{
		mPreviousL[0] = mPreviousL[1] = mPreviousL[2] = mPreviousL[3] = 0;
		mPreviousR[0] = mPreviousR[1] = mPreviousR[2] = mPreviousR[3] = 0;
		mDeltaFader = 1.0f;
		mPrevBlockNr = -1;
	}
	F32 mPreviousL[4];
	F32 mPreviousR[4];
	F32 mDeltaFader;
	S32 mPrevBlockNr;
	S32 mAlign[2];
};

class cDecodeVagInfo
{
public:
	U8* 	mSourceVag;
	U32 	mVagSizeInBytes;
	S32 	mPcmReadOffset;
	F32 	*mpDestPCML;
	F32 	*mpDestPCMR;
	S32 	mDestPCMSamples;
	bool	mLoop;
	U32 	mPCMLoopPosEnd;
	U32 	mPCMLoopPosStart;
	bool	mbStereo;
	bool	mReachedEndOfStream;
	U32		mSamplesConsumed;
	bool	mAbortStream;

};

namespace Audio
{
    namespace Core
    {

////////////////////////////////////////////////////////////////////////////////
//	ADPCM4bitDecoder
////////////////////////////////////////////////////////////////////////////////
        class ADPCM4bitDecoder : public Decoder
        {
		public:
            //  this struct defines the wave data
            struct WaveData
            {
                U8 uChannels;
                bool bLoop;
                U32 uLength;
                U32 uSampleRate;
                S8* psData;
				U32  uPCMLoopPosEnd;
				WaveServerLocalIndex hWaveServerIndex;
            };

			struct BaseState
            {
				//	set if looping
                bool bLoop;
				//	set if using SRC and 1st order LP
                bool bResample;
				bool bPlaying;
				bool bPaused;
				//	current 
                U32 uSampleIndex;
				U32 uSampleCounter;
                F32 fPrevPitch;
                F32 fPitch;
				F32 fNewPitch;
				bool bTriggerCallback;

				U32 uAdpcmWaveInfoIndex;

                F32 m_fA;
                F32 m_fB;

				cDecodeVagWorkArea	workarea;
            };

        public:
            ADPCM4bitDecoder(bool bLoop, bool bResample);
            ~ADPCM4bitDecoder();
			static void PreInit();
			static void PostShutdown();

			void BecomeActive(const DecoderDataInfo& info);
			void BecomeInactive(const DecoderDataInfo& info);

			void ReleaseResources(const DecoderDataInfo& info);

            void Init(const DecoderDataInfo& info);
            void Shutdown(const DecoderDataInfo& info);
			void SetHasCallback() { m_baseState.bTriggerCallback = true; }
			bool IsActive(const DecoderDataInfo& info) { return m_baseState.bPlaying; }
            F32 Position(const DecoderDataInfo& info);
            void Reset(const DecoderDataInfo& info, U32 uOffset = 0);
            void SetPitch(F32 fPitch);
            bool Finished(const DecoderDataInfo& info);
			void SetPosition(U32 uPosition);
			F32 GetRescaledPitch(F32 f32Pitch);
			inline WaveStreamData* GetWaveData(const DecoderDataInfo& info) { return Core::GetWaveData(m_waveData.hWaveServerIndex, info.pWaveServerData); }
			inline AdpcmWaveInfo* GetPrevFrameBufferInfo(const DecoderDataInfo& info) { return GetAdpcmBufferData(m_baseState.uAdpcmWaveInfoIndex, info.pAdpcmBufferData); }
			void SetPlaying(const DecoderDataInfo& info, const bool bPlaying) { if (GetWaveData(info)) GetWaveData(info)->SetPlaying(bPlaying, (U32*)(info.pWaveServerData)); /*else*/ m_baseState.bPlaying = bPlaying; }
			void SetAdpcmWaveIndex(U32 u32Index) { m_baseState.uAdpcmWaveInfoIndex = u32Index; }
			U8* GetData( const DecoderDataInfo& info );
			U32 Length()
			{
				return m_waveData.uLength / (m_waveData.uChannels * sizeof(S16));
			}
            static void ReleaseDecoder() {}

			void SetWaveData(WaveData& waveData)
			{
				m_waveData = waveData;
			}
			bool DecodeBlock( U8* source, F32* dest, F32* prev1, F32* prev2, F32* deltaFader );
			bool DecodeBlock3( U8* source, F32* dest, F32* prev1, F32* prev2, F32* deltaFader );
			S32 ReadVagStreamMono( cDecodeVagInfo* decodeInfo, cDecodeVagWorkArea* workarea );
			S32 ReadVagStreamStereo( cDecodeVagInfo* decodeInfo, cDecodeVagWorkArea* workarea );

			static const U32 Total() { return m_uTotalADPCM; }
			static const U32 Active() { return m_uActiveADPCM; }

#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU)
			static void SetTotal(U32 u32Total) { m_uTotalADPCM = u32Total; }
			static void SetActive(U32 u32Active) { m_uActiveADPCM = u32Active; }
#endif

        protected:
            static U32 m_uTotalADPCM;
            static U32 m_uActiveADPCM;

		protected:
			inline bool UsingResampler() { return m_baseState.bResample; }
			inline S32 GetNumSamples();
			inline F32 GetSubFramePitch(const ProcessInfo& info);
			inline void UpdateFilterCoeffs(F32 fPitch, const ProcessInfo& info);
			inline void PostUpdatePitch(const ProcessInfo& info);
			inline void CrossingBoundary(const ProcessInfo& info, S32 sNumSamples);
			bool IsDataInAvailable(const DecoderDataInfo& info);
#ifdef PLATFORM_SPU
			void DiagnosticInfo(U32 sourceIndex, U32 decodeBufferFillSize, bool bHaveSourceIndex = true);
#endif
		protected:
				WaveData	m_waveData;
				BaseState	m_baseState;

				bool		m_bInitedWave;
				bool		m_bPrevActive;

        };


//////////////////////////////////////////////////////////////////////////////////
////	MonoADPCM4bitDecoder
//////////////////////////////////////////////////////////////////////////////////
        class MonoADPCM4bitDecoder : public ADPCM4bitDecoder
        {
        public:
            //  this struct holds the state (and data) for a decoding sound
			struct ADPCMMonoState : public MonoState
            {
				U32 writeIndex;
            };

        public:
			MonoADPCM4bitDecoder(bool bLoop, bool bResample) : ADPCM4bitDecoder(bLoop, bResample)
			{
                m_state.fPrevSample = 0.f;
                m_state.fPrevIn = 0.f;
                m_state.fPrevOut = 0.f;
				m_state.writeIndex = 0;
				m_u32NumChannels = 1;
			}
			static void PreInit();
			static void PostShutdown();

            void Init(const DecoderDataInfo& info);
            void Shutdown(const DecoderDataInfo& info);
          
            bool FillBuffer(F32* afData, const ProcessInfo& info);
         
            void SkipBuffer(const ProcessInfo& info);
          
			U32 Length() { return m_waveData.uLength / sizeof(S16); }

            static void ReleaseDecoder() {}

        private:
			ADPCMMonoState m_state;
		};

//////////////////////////////////////////////////////////////////////////////////
////	StereoADPCM4bitDecoder
//////////////////////////////////////////////////////////////////////////////////
        class StereoADPCM4bitDecoder : public ADPCM4bitDecoder
        {
        public:
			StereoADPCM4bitDecoder(bool bLoop, bool bResample) : ADPCM4bitDecoder(bLoop, bResample)
			{
                m_state.fPrevSampleL = 0.f;
                m_state.fPrevInL = 0.f;
                m_state.fPrevOutL = 0.f;

				m_state.fPrevSampleR = 0.f;
                m_state.fPrevInR = 0.f;
                m_state.fPrevOutR = 0.f;

				m_u32NumChannels = 2;

			}
			static void PreInit();
			static void PostShutdown();

            void Init(const DecoderDataInfo& info);
            void Shutdown(const DecoderDataInfo& info);
          
            bool FillBuffer(F32* afData, const ProcessInfo& info);
           
            void SkipBuffer(const ProcessInfo& info);
           
			U32 Length() { return m_waveData.uLength / sizeof(S16); }

            static void ReleaseDecoder() {}

			
        private:
			StereoState m_state;
		};
    }//end core
}//end audio
#endif // _DECODERADPCMF16_H_

