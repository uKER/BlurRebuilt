#ifndef _DECODERPCMS16_H_
#define _DECODERPCMS16_H_

namespace Audio
{
    namespace Core
    {
////////////////////////////////////////////////////////////////////////////////
//	PCMS16Decoder
////////////////////////////////////////////////////////////////////////////////
        class PCMS16Decoder : public Decoder
        {
        public:
            struct WaveData
            {
                U8 uChannels;
                U32 uLength;
                U32 uSampleRate;
				// for compatibility with old system- should ultimately be removed & all sounds use WaveServerLocalIndex pointer instead
                S16* psData;
				WaveServerLocalIndex hWaveServerIndex;
				bool bLoop;
            };

            struct BaseState
            {
				U16 uDataSize;
				//	set if looping
                bool bLoop;
				//	set if using SRC and 1st order LP
                bool bResample;
                bool bPlaying;
                S32 sWaveIndex;
                F32 fPrevPitch;
                F32 fPitch;
				F32 fNewPitch;

				bool bTriggerCallback;


//                U32 u32CallbackId;
//                U32 u32StateOffset;

                F32 m_fA;
                F32 m_fB;

            };

        public:
			PCMS16Decoder(bool bLoop, bool bResample)
			{
                m_baseState.bLoop = bLoop;
				m_baseState.bResample = bResample;
                SetWaveIndex(0);
                m_baseState.fPrevPitch = 1.f;
                m_baseState.fPitch = 1.f;
                m_baseState.fNewPitch = 1.f;
//                m_baseState.u32CallbackId = 0;
//				m_baseState.u32StateOffset = 0;
                m_baseState.m_fA = 0.f;
                m_baseState.m_fB = 0.f;
				m_baseState.bPlaying = false;
				m_baseState.bTriggerCallback = false;

				m_uTotalPCM++;

				m_eType = uPCM16BitSignedInt;

				m_bCallbackFired = false;
#if defined (PLATFORM_SPU) || defined (PLATFORM_PS3)
				u32DataOffset = 0;
#endif
			}
            ~PCMS16Decoder();
			static void PreInit();
			static void PostShutdown();

            void Init(const DecoderDataInfo& info);
            void Shutdown(const DecoderDataInfo& info);
			/*audio_virtual */bool IsActive(const DecoderDataInfo& info) { if (m_waveData.hWaveServerIndex.IsValid()) return GetWaveData(info)->IsPlaying(); return m_baseState.bPlaying; }
//            void SetCallbackId(U32 u32StateOffset, U32 u32CallbackId);
			void SetHasCallback() { m_baseState.bTriggerCallback = true; }
            F32 Position(const DecoderDataInfo& info);
            void Reset(const DecoderDataInfo& info, U32 uOffset = 0);
            void SetPitch(F32 fPitch);
            bool Finished(const DecoderDataInfo& info);
            void SetPosition(U32 uPosition);
			inline WaveStreamData* GetWaveData(const DecoderDataInfo& info) { return Core::GetWaveData(m_waveData.hWaveServerIndex, info.pWaveServerData); }
			U32 GetBytesAvailable(const DecoderDataInfo& info) { if (m_waveData.hWaveServerIndex.IsValid()) return GetWaveData(info)->GetBytesAvailable(); return (Length() - m_baseState.sWaveIndex); }
			U32 GetLoopCount(const DecoderDataInfo& info, bool bReset = false);
            inline U32 Length()
			{
				return m_waveData.uLength / (m_waveData.uChannels * sizeof(S16));
			}

            static void ReleaseDecoder() {}


			void SetWaveData(WaveData& waveData)
			{
				m_waveData = waveData;
			}

			bool IsPlaying(const DecoderDataInfo& info) { if (GetWaveData(info)) return GetWaveData(info)->IsPlaying(); return m_baseState.bPlaying; }
			void SetPlaying(const DecoderDataInfo& info, const bool bPlaying) { if (GetWaveData(info)) GetWaveData(info)->SetPlaying(bPlaying, (U32*)(info.pWaveServerData)); else m_baseState.bPlaying = bPlaying; }

			void SetWaveIndex(S32 s32WaveIndex) { m_baseState.sWaveIndex = s32WaveIndex; }
			S32 GetWaveIndex(const DecoderDataInfo& info) { if (! m_waveData.hWaveServerIndex.IsValid()) return m_baseState.sWaveIndex; else return 0;/*GetWaveData(pWaveServerData)->GetSourceDataPosition();*/ }	// if we return GetSourceDataPosition from wave server, messes up u32localindex in FillBuffer, which assumes index is 0 for streams

			static const U32 Total() { return m_uTotalPCM; }
			static const U32 Active() { return m_uActivePCM; }

#ifdef PLATFORM_SPU
			void PrepareDmaTransfer(const DecoderDataInfo& info);
			void FetchData(U8* pu8PpuAddress, U8* pu8SpuAddress, U32 u32SizeInBytes);
#endif

        protected:
            static U32 m_uTotalPCM;
            static U32 m_uActivePCM;

        protected:
			inline bool UsingResampler() { return m_baseState.bResample; }
			inline S32 GetNumSamples();
			inline F32 GetSubFramePitch(const ProcessInfo& info);
			inline void UpdateFilterCoeffs(F32 fPitch, const ProcessInfo& info);
			inline void PostUpdatePitch(const ProcessInfo& info);
			inline void CrossingBoundary(const ProcessInfo& info, S32 sNumSamples);

			S16* GetData(const DecoderDataInfo& info, U32 u32NumSamples, U32* pu32SamplesProcessed);
//			bool IsDataInAvailable();

        protected:
			WaveData m_waveData;
			BaseState m_baseState;
#if defined (PLATFORM_SPU) || defined (PLATFORM_PS3)
			U32 u32DataOffset;
#endif
		};

////////////////////////////////////////////////////////////////////////////////
//	MonoPCMS16Decoder
////////////////////////////////////////////////////////////////////////////////
        class MonoPCMS16Decoder : public PCMS16Decoder
        {
        public:
			MonoPCMS16Decoder(bool bLoop, bool bResample) : PCMS16Decoder(bLoop, bResample)
			{
                m_state.fPrevSample = 0.f;
                m_state.fPrevIn = 0.f;
                m_state.fPrevOut = 0.f;

				m_u32NumChannels = 1;
			}
			static void PreInit();
			static void PostShutdown();

            void Init(const DecoderDataInfo& info);
            bool FillBuffer(F32* afData, const ProcessInfo& info);
            void SkipBuffer(const ProcessInfo& info);
			U32 Length() { return m_waveData.uLength / sizeof(S16); }

            static void ReleaseDecoder() {}

        private:
			MonoState m_state;
		};

////////////////////////////////////////////////////////////////////////////////
//	StereoPCMS16Decoder
////////////////////////////////////////////////////////////////////////////////
        class StereoPCMS16Decoder : public PCMS16Decoder
        {
        public:
			StereoPCMS16Decoder(bool bLoop, bool bResample) : PCMS16Decoder(bLoop, bResample)
			{
                m_state.fPrevSampleL = 0.f;
                m_state.fPrevSampleR = 0.f;
                m_state.fPrevInL = 0.f;
                m_state.fPrevInR = 0.f;
                m_state.fPrevOutL = 0.f;
                m_state.fPrevOutR = 0.f;

				m_u32NumChannels = 2;
			}
			static void PreInit();
			static void PostShutdown();

            void Init(const DecoderDataInfo& info);
            bool FillBuffer(F32* afData, const ProcessInfo& info);
            void SkipBuffer(const ProcessInfo& info);
            U32 Length()
			{
				return m_waveData.uLength / (m_waveData.uChannels * sizeof(S16));
			}

            static void ReleaseDecoder() {}

        private:
			StereoState m_state;
		};
    }
}

#endif // _DECODERPCMS16_H_
