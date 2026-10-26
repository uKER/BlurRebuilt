#ifndef _DECODER_H_
#define _DECODER_H_

//#define AUDIO_NEW_DECODERS

namespace Audio
{
    namespace Core
    {
////////////////////////////////////////////////////////////////////////////////
//	Decoder
////////////////////////////////////////////////////////////////////////////////
        class Decoder
        {
		public:
            //  this struct holds the state (and data) for a decoding sound
            struct StereoState
            {
				F32 fPrevSampleL;
				F32 fPrevSampleR;

				F32 fPrevInL;
				F32 fPrevInR;
				F32 fPrevOutL;
				F32 fPrevOutR;
            };

            //  this struct holds the state (and data) for a decoding sound
            struct MonoState
            {
				F32 fPrevSample;

				F32 fPrevIn;
				F32 fPrevOut;
            };

        public:
			static void* operator new(size_t sz, void* pHere)
			{
				return pHere;
			};
			static void operator delete(void* ptr, void* pHere)
			{
			};
			static void* operator new(size_t sz)
			{
				Assert(0 && "This should not be called");
				//return new U8[sz];
				return malloc(sz);
			};
			static void operator delete(void* ptr)
			{
				Assert(0 && "This should not be called");
				//delete ptr;
				free(ptr);
			};

        public:
#ifdef VF_DECODERS
            audio_virtual ~Decoder() {};
/*
			//	one time system calls
            audio_virtual void PreInit() {}
            audio_virtual void PostShutdown() {}
			
			//	work methods
			audio_virtual void PreDoWork() {}
			audio_virtual void DoWork(bool bIsAudible) {}
			audio_virtual void PostDoWork() {}

			audio_virtual void Reset() {}
			audio_virtual void SetPitch() {}
*/
            audio_virtual void Init(const DecoderDataInfo& info) {}
            audio_virtual void Shutdown(const DecoderDataInfo& info) {}
			audio_virtual bool IsActive(const DecoderDataInfo& info) = 0;
//            audio_virtual void SetCallbackId(U32 u32StateOffset, U32 u32CallbackId) {}
			audio_virtual void SetHasCallback() { }
            audio_virtual bool FillBuffer(F32* afData, const ProcessInfo& info) = 0;
            audio_virtual void SkipBuffer(const ProcessInfo& info) = 0;
			audio_virtual F32 Position(const DecoderDataInfo& info) = 0;
            audio_virtual void Reset(const DecoderDataInfo& info, U32 uOffset) = 0;
            audio_virtual void SetPitch(F32 fPitch) {}
            //audio_virtual bool Finished() = 0;
            audio_virtual void SetPosition(const ProcessInfo& info, U32 uPosition) {}
            audio_virtual U32 Length() { return 0; }
//#ifdef PLATFORM_SPU
//			audio_virtual void PrepareDmaTransfer() = 0;
//			audio_virtual void ReleaseDmaTransfer() = 0;
//#endif
#else
            ~Decoder() {};

            void Init(const DecoderDataInfo& info);
            void Shutdown(const DecoderDataInfo& info);
			bool IsActive(const DecoderDataInfo& info);
//            void SetCallbackId(U32 u32StateOffset, U32 u32CallbackId);
			void SetHasCallback();
            bool FillBuffer(F32* afData, const ProcessInfo& info);
            void SkipBuffer(const ProcessInfo& info);
			F32 Position(const DecoderDataInfo& info);
            void Reset(const DecoderDataInfo& info, U32 uOffset);
            void SetPitch(F32 fPitch);
            void SetPosition(U32 uPosition);
            U32 Length();
//#ifdef PLATFORM_SPU
//			void PrepareDmaTransfer();
//			void ReleaseDmaTransfer();
//#endif
#endif

			U32 m_eType;
			U32 m_u32NumChannels;
			bool m_bCallbackFired;

			void Resample(const float fPitch, const unsigned int uSamplesRequired, const F32* __restrict in, F32* __restrict out);
			void Resample(const float fPitch, const unsigned int uSamplesRequired, const F32* __restrict inLeft, const F32* __restrict inRight, F32* __restrict outLeft, F32* __restrict outRight);

			void FillF32BufferFromS16(unsigned int samplesRequired, const S16* AUDIO_RESTRICT src, F32* AUDIO_RESTRICT dst);
			void FillF32BufferFromS16(unsigned int samplesRequired, const S16* AUDIO_RESTRICT src, F32* AUDIO_RESTRICT dstL, F32* AUDIO_RESTRICT dstR);

			void LowPassFilter(unsigned int uSamplesRequired, MonoState& state, F32* AUDIO_RESTRICT buffer, float fA, float fB);
			void LowPassFilter(unsigned int uSamplesRequired, StereoState& state, F32* AUDIO_RESTRICT bufferL, F32* AUDIO_RESTRICT bufferR, float fA, float fB);

			void Prime(const DecoderDataInfo& info);
			void Flush(const DecoderDataInfo& info);

			bool HasCallbackFired()
			{
				if (m_bCallbackFired)
				{
					m_bCallbackFired = false;
					return true;
				}
				return false;
			}
		};
    }
}

#endif // _DECODER_H_
